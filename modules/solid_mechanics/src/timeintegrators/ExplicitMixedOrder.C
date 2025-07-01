//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "Assembly.h"
#include "ExplicitMixedOrder.h"
#include "MeshChangedInterface.h"
#include "Moose.h"
#include "MooseError.h"
#include "MooseTypes.h"
#include "MooseVariableFieldBase.h"
#include "NonlinearSystem.h"

// libMesh includes
#include "TransientBase.h"
#include "libmesh/nonlinear_solver.h"
#include "libmesh/sparse_matrix.h"
#include <algorithm>
#include <iterator>
#include <utility>

registerMooseObject("SolidMechanicsApp", ExplicitMixedOrder);
registerMooseObjectRenamed("SolidMechanicsApp",
                           DirectCentralDifference,
                           "10/14/2025 00:00",
                           ExplicitMixedOrder);

InputParameters
ExplicitMixedOrder::validParams()
{
  InputParameters params = TimeIntegrator::validParams();

  params.addClassDescription(
      "Implementation of explicit time integration without invoking any of the nonlinear solver.");

  params.addParam<bool>("use_constant_mass",
                        false,
                        "If set to true, will only compute the mass matrix in the first time step, "
                        "and keep using it throughout the simulation.");
  params.addParam<bool>(
      "use_constant_damping",
      false,
      "If set to true, will only compute the damping matrix in the first time step, "
      "and keep using it throughout the simulation.");
  params.addParam<TagName>("mass_matrix_tag", "mass", "The tag for the mass matrix");
  params.addParam<TagName>("damping_matrix_tag", "damping", "The tag for the damping matrix");

  params.addParam<std::vector<VariableName>>(
      "second_order_vars",
      {},
      "A subset of variables that require second-order integration (velocity and acceleration) to "
      "be applied by this time integrator.");

  params.addParam<std::vector<VariableName>>(
      "first_order_vars",
      {},
      "A subset of variables that require first-order integration (velocity only) to be applied by "
      "this time integrator.");

  // Prevent users from using variables option by accident.
  params.suppressParameter<std::vector<VariableName>>("variables");

  return params;
}

ExplicitMixedOrder::ExplicitMixedOrder(const InputParameters & parameters)
  : TimeIntegrator(parameters),
    MeshChangedInterface(parameters),
    _has_damping(false),
    _constant_mass(getParam<bool>("use_constant_mass")),
    _constant_damping(getParam<bool>("use_constant_damping")),
    _mass_matrix(getParam<TagName>("mass_matrix_tag")),
    _damping_matrix(getParam<TagName>("damping_matrix_tag")),
    _solution_older(_sys.solutionState(2)),
    _explicit_residual(addVector("explicit_residual", false, PARALLEL)),
    _solution_update(addVector("solution_update", true, PARALLEL)),
    _mass_matrix_lumped(addVector("mass_matrix_lumped", true, GHOSTED)),
    _damping_matrix_lumped(addVector("damping_matrix_lumped", true, GHOSTED)),
    _vars_first(declareRestartableData<std::unordered_set<unsigned int>>("first_order_vars")),
    _local_first_order_indices(
        declareRestartableData<std::vector<dof_id_type>>("first_local_indices")),
    _vars_second(declareRestartableData<std::unordered_set<unsigned int>>("second_order_vars")),
    _local_second_order_indices(
        declareRestartableData<std::vector<dof_id_type>>("second_local_indices"))
{
  _fe_problem.setUDotRequested(true);
  _fe_problem.setUDotOldRequested(true);
  _fe_problem.setUDotDotRequested(true);

  // This effectively changes the default solve_type to LINEAR instead of PJFNK,
  // so that it is valid to not supply solve_type in the Executioner block:
  if (_nl)
    _fe_problem.solverParams(_nl->number())._type = Moose::ST_LINEAR;

  _ones = addVector("ones", true, PARALLEL);

  // don't set any of the common SNES-related petsc options to prevent unused option warnings
  Moose::PetscSupport::dontAddCommonSNESOptions(_fe_problem);
}

void
ExplicitMixedOrder::meshChanged()
{
  *_ones = 1.;
}

void
ExplicitMixedOrder::init()
{
  _has_damping = _sys.subproblem().matrixTagExists(_damping_matrix);

  meshChanged();

  if (_nl && _fe_problem.solverParams(_nl->number())._type != Moose::ST_LINEAR)
    mooseError(
        "The chosen time integrator requires 'solve_type = LINEAR' in the Executioner block.");

  // Compute ICs for velocity
  computeICs();

  // Seperate variables into first and second time integration order and find
  // the local indices for each
  const auto & var_names_first = getParam<std::vector<VariableName>>("first_order_vars");
  const auto & var_names_second = getParam<std::vector<VariableName>>("second_order_vars");
  std::vector<unsigned int> var_num_vec;

  auto & lm_sys = _sys.system();
  lm_sys.get_all_variable_numbers(var_num_vec);
  std::unordered_set<unsigned int> var_nums(var_num_vec.begin(), var_num_vec.end());

  for (const auto & var_name : var_names_first)
    if (lm_sys.has_variable(var_name))
    {
      const auto var_num = lm_sys.variable_number(var_name);
      _vars_first.insert(var_num);
      var_nums.erase(var_num);
    }

  for (const auto & var_name : var_names_second)
    if (lm_sys.has_variable(var_name))
    {
      const auto var_num = lm_sys.variable_number(var_name);
      _vars_second.insert(var_num);
      var_nums.erase(var_num);
    }

  // If var_nums is empty then that means the user has specified all the variables in this system
  if (!var_nums.empty())
    mooseError("Not all nonlinear variables have their order specified.");

  std::vector<dof_id_type> var_dof_indices, work_vec;
  for (const auto var_num : _vars_first)
  {
    work_vec = _local_first_order_indices;
    _local_first_order_indices.clear();
    lm_sys.get_dof_map().local_variable_indices(var_dof_indices, lm_sys.get_mesh(), var_num);
    std::merge(work_vec.begin(),
               work_vec.end(),
               var_dof_indices.begin(),
               var_dof_indices.end(),
               std::back_inserter(_local_first_order_indices));
  }

  work_vec.clear();
  var_dof_indices.clear();

  for (const auto var_num : _vars_second)
  {
    work_vec = _local_second_order_indices;
    _local_second_order_indices.clear();
    lm_sys.get_dof_map().local_variable_indices(var_dof_indices, lm_sys.get_mesh(), var_num);
    std::merge(work_vec.begin(),
               work_vec.end(),
               var_dof_indices.begin(),
               var_dof_indices.end(),
               std::back_inserter(_local_second_order_indices));
  }
}

void
ExplicitMixedOrder::computeTimeDerivatives()
{
  /*
  Because this is called in NonLinearSystemBase
  this should not actually compute the time derivatives.
  Calculating time derivatives here will cause issues for the
  solution update.
  */
  return;
}

void
ExplicitMixedOrder::postResidual(NumericVector<Number> & residual)
{
  residual += *_Re_time;
  residual += *_Re_non_time;
  residual.close();

  // Reset time to the time at which to evaluate nodal BCs, which comes next
  _fe_problem.time() = _current_time;
}

TagID
ExplicitMixedOrder::massMatrixTagID() const
{
  return _sys.subproblem().getMatrixTagID(_mass_matrix);
}

TagID
ExplicitMixedOrder::dampingMatrixTagID() const
{
  return _sys.subproblem().getMatrixTagID(_damping_matrix);
}

void
ExplicitMixedOrder::solve()
{
  // Reset iteration counts
  _n_nonlinear_iterations = 0;
  _n_linear_iterations = 0;

  // since we will modify time when evaluating the residual, we need to record the current time so
  // that we can reset it back after the residual is computed
  _current_time = _fe_problem.time();

  // system matrix
  auto & sytem_matrix = _nonlinear_implicit_system->get_system_matrix();

  // current solution
  const auto & sol = *_nonlinear_implicit_system->current_local_solution;

  // Get the lumped mass matrix
  // We may only want to compute lumped mass matrix once.
  auto mass_tag = massMatrixTagID();
  if (!_constant_mass || _t_step == 1)
  {
    _fe_problem.computeJacobianTag(sol, sytem_matrix, mass_tag);
    // lump
    sytem_matrix.vector_mult(*_mass_matrix_lumped, *_ones);
    _mass_matrix_lumped->close();
  }

  // Get the lumped damping matrix
  // We may only want to compute lumped damping matrix once.
  if (_has_damping)
  {
    auto damping_tag = dampingMatrixTagID();
    if (!_constant_damping || _t_step == 1)
    {
      _fe_problem.computeJacobianTag(sol, sytem_matrix, damping_tag);
      // lump
      sytem_matrix.vector_mult(*_damping_matrix_lumped, *_ones);
      _damping_matrix_lumped->close();
    }
  }

  // Set time to the time at which to evaluate the residual
  _fe_problem.time() = _fe_problem.timeOld();
  _nonlinear_implicit_system->update();

  // Compute the residual
  _explicit_residual->zero();
  _fe_problem.computeResidual(sol, *_explicit_residual, _nl->number());

  // Move the residual to the RHS
  *_explicit_residual *= -1.0;

  // Perform the linear solve
  forwardEuler();
  centralDifference();
  const bool converged = solutionUpdate();

  // Apply constraints
  _nl->overwriteNodeFace(*_nonlinear_implicit_system->solution);

  // Update the solution
  *_nonlinear_implicit_system->solution = _nl->solutionOld();
  *_nonlinear_implicit_system->solution += *_solution_update;
  _nonlinear_implicit_system->update();
  _nl->setSolution(*_nonlinear_implicit_system->current_local_solution);
  _nonlinear_implicit_system->nonlinear_solver->converged = converged;
}

void
ExplicitMixedOrder::forwardEuler()
{
  // Split lumped mass matrix for first-order variables
  auto Minv = NumericVector<Number>::build(_communicator);
  _mass_matrix_lumped->create_subvector(*Minv, _local_first_order_indices, false);
  Minv->reciprocal();

  // Split residual vector for first-order variables
  const auto r = NumericVector<Number>::build(_communicator);
  _explicit_residual->create_subvector(*r, _local_first_order_indices, false);

  // Calculate velocity
  auto vel = _sys.solutionUDot();
  auto v = vel->get_subvector(_local_first_order_indices);
  v->pointwise_mult(*Minv, *r);

  // close the vectors
  vel->restore_subvector(std::move(v), _local_first_order_indices);
  vel->close();
}

void
ExplicitMixedOrder::centralDifference()
{
  // Split lumped mass matrix for first-order variables
  auto M = NumericVector<Number>::build(_communicator);
  _mass_matrix_lumped->create_subvector(*M, _local_second_order_indices, false);

  const auto r = NumericVector<Number>::build(_communicator);
  _explicit_residual->create_subvector(*r, _local_second_order_indices, false);

  // Split lumped mass matrix for first-order variables
  auto accel = _sys.solutionUDotDot();
  auto a = accel->get_subvector(_local_second_order_indices);
  auto vel = _sys.solutionUDot();
  auto vn = vel->get_subvector(_local_second_order_indices);
  if (_has_damping)
  {
    auto C = NumericVector<Number>::build(_communicator);
    _damping_matrix_lumped->create_subvector(*C, _local_second_order_indices, false);

    auto rc = C->clone();
    rc->pointwise_mult(*rc, *vn);
    r->add(-1.0, *rc);

    auto coef = C->clone();
    coef->scale(_dt / 2);
    coef->add(*M);
    coef->reciprocal();
    a->pointwise_mult(*coef, *r);
  }
  else
  {
    auto coef = M->clone();
    coef->reciprocal();
    a->pointwise_mult(*coef, *r);
  }

  // Calculate velocity
  auto delta_v = a->clone();
  delta_v->scale((_dt + _dt_old) / 2);
  vn->add(*delta_v);

  // close the vectors
  accel->restore_subvector(std::move(a), _local_second_order_indices);
  // _console << "acceleration: " << *accel << std::endl;
  accel->close();
  vel->restore_subvector(std::move(vn), _local_second_order_indices);
  // _console << "velocity: " << *vel << std::endl;
  vel->close();
}

bool
ExplicitMixedOrder::solutionUpdate()
{
  auto vel = _sys.solutionUDot();
  *_solution_update = *vel;
  _solution_update->scale(_dt);

  // Check for convergence by seeing if there is a nan or inf
  auto sum = _solution_update->sum();
  const bool converged = std::isfinite(sum);

  // The linear iteration count remains zero
  _n_linear_iterations = 0;

  return converged;
}

void
ExplicitMixedOrder::postSolve()
{
  // Once we have the new solution, we want to adanceState to make sure the
  // coupling between the solution and the computed material properties is kept correctly.
  _fe_problem.advanceState();
}

void
ExplicitMixedOrder::computeICs()
{
  // Compute the first-order approximation of the velocity at the current time step
  // using the Euler scheme, where the velocity is estimated as the difference
  // between the current solution and the previous time step, divided by the time
  auto vel = _sys.solutionUDot();
  *vel = *_solution;
  *vel -= _solution_old;
  *vel /= _dt;
  vel->close();
}

ExplicitMixedOrder::TimeOrder
ExplicitMixedOrder::findVariableTimeOrder(unsigned int var_num) const
{
  if (_vars_first.empty() && _vars_second.empty())
    mooseError("Time order sets are both empty.");
  if (_vars_first.count(var_num))
    return FIRST;
  else if (_vars_second.count(var_num))
    return SECOND;
  else
    mooseError("Variable " + _sys.system().variable_name(var_num) +
               " does not exist in time order sets.");
}
