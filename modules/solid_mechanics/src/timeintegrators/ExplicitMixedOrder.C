// MOOSE includes
#include "Assembly.h"
#include "ExplicitMixedOrder.h"
#include "ExplicitTimeIntegrator.h"
#include "Moose.h"
#include "MooseError.h"
#include "MooseTypes.h"
#include "MooseVariableFieldBase.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"
#include "TimeStepper.h"
#include "TransientBase.h"

// libMesh includes
#include "TransientBase.h"
#include "libmesh/id_types.h"
#include "libmesh/nonlinear_solver.h"
#include "libmesh/sparse_matrix.h"
#include "DirichletBCBase.h"
#include "libmesh/vector_value.h"
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
  InputParameters params = ExplicitTimeIntegrator::validParams();

  params.addClassDescription(
      "Implementation of explicit time integration without invoking any of the nonlinear solver.");

  params.addParam<bool>("use_constant_mass",
                        false,
                        "If set to true, will only compute the mass matrix in the first time step, "
                        "and keep using it throughout the simulation.");
  params.addParam<bool>(
      "use_constant_damp",
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

  MooseEnum solve_type("consistent lumped lump_preconditioned", "lumped");
  params.setParameters("solve_type", solve_type);
  params.ignoreParameter<MooseEnum>("solve_type");
  return params;
}

ExplicitMixedOrder::ExplicitMixedOrder(const InputParameters & parameters)
  : ExplicitTimeIntegrator(parameters),
    _constant_mass(getParam<bool>("use_constant_mass")),
    _constant_damping(getParam<bool>("use_constant_damp")),
    _mass_matrix(getParam<TagName>("mass_matrix_tag")),
    _damping_matrix(getParam<TagName>("damping_matrix_tag")),
    _solution_older(_sys.solutionState(2)),
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
  // Getting the tagID for the mass and damping matrix
  auto mass_tag = massMatrixTagID();
  auto damping_tag = dampingMatrixTagID();

  // Reset iteration counts
  _n_nonlinear_iterations = 0;
  _n_linear_iterations = 0;

  _current_time = _fe_problem.time();

  // Get the lumped mass matrix
  // Define the matrix with system size
  auto & mass_matrix = _nonlinear_implicit_system->get_system_matrix();
  // Compute the mass matrix
  if (!_constant_mass || _t_step == 1)
  {
    // We only want to compute "inverted" lumped mass matrix once.
    _fe_problem.computeJacobianTag(
        *_nonlinear_implicit_system->current_local_solution, mass_matrix, mass_tag);
    // Calculating the lumped mass matrix for use in residual calculation
    mass_matrix.vector_mult(*_mass_matrix_diag_inverted, *_ones);
    // // "Invert" the diagonal mass matrix
    // _mass_matrix_diag_inverted->reciprocal();
    _mass_matrix_diag_inverted->close();
  }

  // Get the lumped damping matrix
  // Define the matrix with system size
  auto & damping_matrix = _nonlinear_implicit_system->get_system_matrix();
  // Compute the damping matrix
  if (!_constant_damping || _t_step == 1)
  {
    // We only want to compute "inverted" lumped mass matrix once.
    _fe_problem.computeJacobianTag(
        *_nonlinear_implicit_system->current_local_solution, damping_matrix, damping_tag);
    // Calculating the lumped damping matrix for use in residual calculation
    damping_matrix.vector_mult(*_damping_matrix_diag_inverted, *_ones);
    // // "Invert" the diagonal mass matrix
    // _mass_matrix_diag_inverted->reciprocal();
    _damping_matrix_diag_inverted->close();
  }

  // Set time to the time at which to evaluate the residual
  _fe_problem.time() = _fe_problem.timeOld();
  _nonlinear_implicit_system->update();

  // Compute the residual
  _explicit_residual->zero();
  _fe_problem.computeResidual(
      *_nonlinear_implicit_system->current_local_solution, *_explicit_residual, _nl->number());

  // Move the residual to the RHS
  *_explicit_residual *= -1.0;

  // Perform the linear solve
  bool converged = performExplicitSolve(mass_matrix);
  _nl->overwriteNodeFace(*_nonlinear_implicit_system->solution);

  // Update the solution
  *_nonlinear_implicit_system->solution = _nl->solutionOld();
  *_nonlinear_implicit_system->solution += *_solution_update;

  _nonlinear_implicit_system->update();

  _nl->setSolution(*_nonlinear_implicit_system->current_local_solution);
  _nonlinear_implicit_system->nonlinear_solver->converged = converged;
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

bool
ExplicitMixedOrder::performExplicitSolve(SparseMatrix<Number> &)
{

  bool converged = false;

  // Grab all the vectors that we will need
  auto accel = _sys.solutionUDotDot();
  auto vel = _sys.solutionUDot();

  // Compute Forward Euler
  // Split diag mass and residual vectors into correct subvectors
  const std::unique_ptr<NumericVector<Number>> mass_first(
      NumericVector<Number>::build(_communicator));
  const std::unique_ptr<NumericVector<Real>> exp_res_first(
      NumericVector<Number>::build(_communicator));
  _mass_matrix_diag_inverted->create_subvector(*mass_first, _local_first_order_indices, false);
  _explicit_residual->create_subvector(*exp_res_first, _local_first_order_indices, false);

  // Need velocity vector split into subvectors
  auto vel_first = vel->get_subvector(_local_first_order_indices);

  // Velocity update for foward euler
  vel_first->pointwise_mult(*mass_first, *exp_res_first);

  // Restore the velocities
  vel->restore_subvector(std::move(vel_first), _local_first_order_indices);

  // Compute Central Difference
  // Split diag mass and residual vectors into correct subvectors
  const std::unique_ptr<NumericVector<Real>> mass_second(
      NumericVector<Number>::build(_communicator));
  const std::unique_ptr<NumericVector<Real>> damping_second(
      NumericVector<Number>::build(_communicator));
  const std::unique_ptr<NumericVector<Real>> exp_res_second(
      NumericVector<Number>::build(_communicator));
  _mass_matrix_diag_inverted->create_subvector(*mass_second, _local_second_order_indices, false);
  _damping_matrix_diag_inverted->create_subvector(
      *damping_second, _local_second_order_indices, false);
  _explicit_residual->create_subvector(*exp_res_second, _local_second_order_indices, false);

  // Only need acceleration and old velocity vector for central difference
  auto accel_second = accel->get_subvector(_local_second_order_indices);

  auto vel_second = vel->get_subvector(_local_second_order_indices);

  // Compute acceleration for central difference
  auto damping_res = damping_second->clone();
  damping_second->scale(_dt / 2);
  *mass_second += *damping_second;
  damping_res->pointwise_mult(*damping_res, *vel_second);
  damping_res->scale(2 * _dt / (_dt + _dt_old));
  *exp_res_second -= *damping_res;
  accel_second->pointwise_mult(*mass_second, *exp_res_second);

  // Scaling the acceleration
  auto accel_scaled = accel_second->clone();
  accel_scaled->scale((_dt + _dt_old) / 2);

  // Velocity update for central difference
  *vel_second += *accel_scaled;

  // Restore acceleration
  accel->restore_subvector(std::move(accel_second), _local_second_order_indices);

  vel->restore_subvector(std::move(vel_second), _local_second_order_indices);

  // Same solution update for both methods
  *_solution_update = *vel;
  _solution_update->scale(_dt);

  // Check for convergence by seeing if there is a nan or inf
  auto sum = _solution_update->sum();
  converged = std::isfinite(sum);

  // The linear iteration count remains zero
  _n_linear_iterations = 0;
  vel->close();
  accel->close();

  return converged;
}

void
ExplicitMixedOrder::init()
{
  ExplicitTimeIntegrator::init();

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