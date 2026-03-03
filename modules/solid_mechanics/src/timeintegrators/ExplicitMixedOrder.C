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
#include "ExplicitTimeIntegrator.h"
#include "Moose.h"
#include "MooseError.h"
#include "MooseTypes.h"
#include "MooseVariableFieldBase.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"
#include "TimeStepper.h"
#include "TransientBase.h"
#include "MooseMesh.h"

// libMesh includes
#include "TransientBase.h"
#include "libmesh/id_types.h"
#include "libmesh/nonlinear_solver.h"
#include "libmesh/sparse_matrix.h"
#include "libmesh/mesh_base.h"
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
      "recompute_mass_matrix_after_mesh_change",
      false,
      "If set to true, the mass matrix will be recomputed when the mesh changes (e.g. through "
      "adaptivity). If use_constant_mass is set to true, adadaptivity is used, and this parameter "
      "is not set to true, the simulation will error out when the mesh changes.");

  params.addParam<TagName>("mass_matrix_tag", "mass", "The tag for the mass matrix");

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

  params.addParam<bool>(
      "restrict_to_active_blocks",
      true,
      "This is only relevant in cases where the first-order and second-order "
      "variables are not defined over the whole mesh. "
      "If set to true, the FE problem will be restricted to blocks of the mesh "
      "upon which the first-order and second-order variables are defined. Usually "
      "you want this, but with mesh adaptivity in cases where the variables are "
      "only defined on a subset of the mesh, a computational cost is associated "
      "with frequently re-calculating the active elements.");

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
    _recompute_mass_matrix_on_mesh_change(
        getParam<bool>("recompute_mass_matrix_after_mesh_change")),
    _mesh_changed(true),
    _mass_matrix_name(getParam<TagName>("mass_matrix_tag")),
    _mass_matrix_lumped(addVector("mass_matrix_lumped", true, GHOSTED)),
    _solution_older(_sys.solutionState(2)),
    _vars_first(declareRestartableData<std::unordered_set<unsigned int>>("first_order_vars")),
    _local_first_order_indices(
        declareRestartableData<std::vector<dof_id_type>>("first_local_indices")),
    _vars_second(declareRestartableData<std::unordered_set<unsigned int>>("second_order_vars")),
    _local_second_order_indices(
        declareRestartableData<std::vector<dof_id_type>>("second_local_indices")),
    _relevant_blocks{},
    _all_mesh_is_relevant(false),
    _restrict_to_active_blocks(getParam<bool>("restrict_to_active_blocks")),
    _elem_range(nullptr),
    _node_range(nullptr),
    _elem_buffer(),
    _node_buffer()
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

void
ExplicitMixedOrder::meshChanged()
{
  if (_constant_mass && !_recompute_mass_matrix_on_mesh_change)
    paramError("recompute_mass_matrix_after_mesh_change",
               "Must be set to true explicitly by the user to support adaptivity with "
               "`use_constant_mass`.");

  // after mesh changes we need to recompute the mass matrix, it is not interpolable as it contains
  // a volume integrated quantity!
  _mesh_changed = true;

  ExplicitTimeIntegrator::meshChanged();
}

TagID
ExplicitMixedOrder::massMatrixTagID() const
{
  return _sys.subproblem().getMatrixTagID(_mass_matrix_name);
}

void
ExplicitMixedOrder::solve()
{
  // Getting the tagID for the mass matrix
  auto mass_tag = massMatrixTagID();

  // Reset iteration counts
  _n_nonlinear_iterations = 0;
  _n_linear_iterations = 0;

  _current_time = _fe_problem.time();

  auto & mass_matrix = _nonlinear_implicit_system->get_system_matrix();

  if (_mesh_changed)
  {
    updateDOFIndices();
    // DOFs have now been updated, and the ghost stuff in following line involved DOFs
    computeAndApplyRanges();
    // Now _mass_matrix_diag_inverted and _mass_matrix_lumped have been appropriately ghosted.
    // Below, they are recalculated
  }

  // Compute the mass matrix
  if (_mesh_changed || !_constant_mass)
  {
    // We only want to compute "inverted" lumped mass matrix once.
    _fe_problem.computeJacobianTag(
        *_nonlinear_implicit_system->current_local_solution, mass_matrix, mass_tag);

    // Calculate and record the lumped mass matrix for use in residual calculation
    mass_matrix.vector_mult(*_mass_matrix_lumped, *_ones);
    _mass_matrix_lumped->close();

    // "Invert" the diagonal mass matrix
    *_mass_matrix_diag_inverted = *_mass_matrix_lumped;
    _mass_matrix_diag_inverted->reciprocal();
    _mass_matrix_diag_inverted->close();
  }

  _mesh_changed = false;

  // Set time to the time at which to evaluate the residual
  _fe_problem.time() = _fe_problem.timeOld();
  _nonlinear_implicit_system->update();

  // Evaluate residual and move it to the RHS
  evaluateRHSResidual();

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
ExplicitMixedOrder::evaluateRHSResidual()
{
  // Compute the residual
  _fe_problem.computeResidual(
      *_nonlinear_implicit_system->current_local_solution, *_explicit_residual, _nl->number());

  // Move the residual to the RHS
  *_explicit_residual *= -1.0;
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
  const std::unique_ptr<NumericVector<Number>> mass_inv_first(
      NumericVector<Number>::build(_communicator));
  const std::unique_ptr<NumericVector<Real>> exp_res_first(
      NumericVector<Number>::build(_communicator));
  _mass_matrix_diag_inverted->create_subvector(*mass_inv_first, _local_first_order_indices, false);
  _explicit_residual->create_subvector(*exp_res_first, _local_first_order_indices, false);

  // Need velocity vector split into subvectors
  auto vel_first = vel->get_subvector(_local_first_order_indices);

  // Velocity update for foward euler
  vel_first->pointwise_mult(*mass_inv_first, *exp_res_first);

  // Restore the velocities
  vel->restore_subvector(std::move(vel_first), _local_first_order_indices);

  // Compute Central Difference
  // Split diag mass and residual vectors into correct subvectors
  const std::unique_ptr<NumericVector<Real>> mass_inv_second(
      NumericVector<Number>::build(_communicator));
  const std::unique_ptr<NumericVector<Real>> exp_res_second(
      NumericVector<Number>::build(_communicator));
  _mass_matrix_diag_inverted->create_subvector(
      *mass_inv_second, _local_second_order_indices, false);
  _explicit_residual->create_subvector(*exp_res_second, _local_second_order_indices, false);

  // Only need acceleration and old velocity vector for central difference
  auto accel_second = accel->get_subvector(_local_second_order_indices);

  auto vel_second = vel->get_subvector(_local_second_order_indices);

  // Compute acceleration for central difference
  accel_second->pointwise_mult(*mass_inv_second, *exp_res_second);

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
ExplicitMixedOrder::initialSetup()
{
  ExplicitTimeIntegrator::initialSetup();

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

  buildRelevantBlocks(var_names_first, var_names_second);
  computeAndApplyRanges();
}

void
ExplicitMixedOrder::computeAndApplyRanges()
{
  if (!_all_mesh_is_relevant && _restrict_to_active_blocks)
  {
    constructRanges();
    setCurrentAlgebraicRanges();
    if (_nonlinear_implicit_system)
      _nonlinear_implicit_system->update();
    reinitGhostedVectorsForCurrentAlgebraicRange();
  }
}

void
ExplicitMixedOrder::buildRelevantBlocks(const std::vector<VariableName> & var_names_first,
                                        const std::vector<VariableName> & var_names_second)
{
  if (!_restrict_to_active_blocks)
    return;

  auto & mesh = _sys.system().get_mesh();
  auto & moose_mesh = _fe_problem.mesh();
  std::set<SubdomainID> all_ids;
  mesh.subdomain_ids(all_ids, /*global=*/true);

  bool all_blocks_selected = false;
  // little lambda function to add blocks to _relevant_blocks
  const auto add_blocks_for = [&](const std::vector<VariableName> & var_names)
  {
    for (const auto & var_name : var_names)
    {
      const auto & var = _fe_problem.getVariable(/*tid=*/0, var_name);
      const auto & block_names = var.blocks();

      if (std::find(block_names.begin(), block_names.end(), "ANY_BLOCK_ID") != block_names.end())
      {
        _relevant_blocks.insert(all_ids.begin(), all_ids.end());
        all_blocks_selected = true;
        return;
      }
      for (const auto & name : block_names)
        _relevant_blocks.insert(moose_mesh.getSubdomainID(name));
    }
  };

  _relevant_blocks.clear();
  add_blocks_for(var_names_first);
  if (!all_blocks_selected)
    add_blocks_for(var_names_second);

  // if the variables are defined over all blocks, don't bother with
  // the computeAndApplyRanges stuff, as it's all done by other parts of MOOSE
  _all_mesh_is_relevant = (_relevant_blocks == all_ids);
}

void
ExplicitMixedOrder::init()
{
  ExplicitTimeIntegrator::init();

  // Compute ICs for velocity
  computeICs();
}

void
ExplicitMixedOrder::updateDOFIndices()
{
  auto & lm_sys = _sys.system();

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
ExplicitMixedOrder::constructRanges()
{
  auto & mesh = _sys.system().get_mesh();
  const processor_id_type my_rank = processor_id();

  // ---- Build union element range across all relevant blocks ----
  _elem_buffer.clear();
  _elem_buffer.reserve(mesh.n_active_elem()); // heuristic

  for (const auto id : _relevant_blocks)
  {
    // Per-subdomain active elements
    for (const auto * e : mesh.active_subdomain_element_ptr_range(id))
    {
      if (e->processor_id() == my_rank)
        _elem_buffer.push_back(e);
    }
  }

  // Build a StoredRange (ConstElemRange) from the packed vector
  _elem_range = std::make_unique<libMesh::ConstElemRange>(&_elem_buffer);

  // ---- Derive node range from element range ----
  std::unordered_set<const libMesh::Node *> uniq;
  uniq.reserve(mesh.n_nodes());

  for (const libMesh::Elem * e : *_elem_range)
    for (unsigned i = 0; i < e->n_nodes(); ++i)
      uniq.insert(e->node_ptr(i));

  _node_buffer.assign(uniq.begin(), uniq.end());
  _node_range = std::make_unique<libMesh::ConstNodeRange>(&_node_buffer);
}

void
ExplicitMixedOrder::setCurrentAlgebraicRanges()
{
  _fe_problem.setCurrentAlgebraicElementRange(_elem_range.get());
  _fe_problem.setCurrentAlgebraicNodeRange(_node_range.get());
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

void
ExplicitMixedOrder::buildGhostIDs(const libMesh::ConstElemRange & elems,
                                  std::vector<dof_id_type> & ghost_ids) const
{
  ghost_ids.clear();
  const auto & dm = _sys.system().get_dof_map();

  const auto first = dm.first_dof();
  const auto end = dm.end_dof();

  std::vector<dof_id_type> dofs;
  for (const libMesh::Elem * e : elems)
  {
    dofs.clear();
    dm.dof_indices(e, dofs);
    for (const auto di : dofs)
      if (di < first || di >= end) // off-rank -> needs ghost
        ghost_ids.push_back(di);
  }
  std::sort(ghost_ids.begin(), ghost_ids.end());
  ghost_ids.erase(std::unique(ghost_ids.begin(), ghost_ids.end()), ghost_ids.end());
}

void
ExplicitMixedOrder::reinitGhostedVectorsForCurrentAlgebraicRange()
{
  // Defensive: ensure we have a valid element range before proceeding
  if (!_elem_range)
    return;

  // Build ghost IDs for the newly restricted algebraic coverage
  std::vector<dof_id_type> ghost_ids;
  buildGhostIDs(*_elem_range, ghost_ids);

  // Size info
  const auto & dm = _sys.system().get_dof_map();
  const auto n_glob = dm.n_dofs();
  const auto n_local = dm.n_local_dofs();

  // If sizes are not distributed yet, do not attempt re-init
  if (n_glob == 0)
    return;

  // Re-init ONLY those vectors you created as GHOSTED (do not convert PARALLEL vectors to ghosted)
  auto safe_reinit = [&](NumericVector<Number> * v)
  {
    if (v && v->type() == GHOSTED)
      v->init(n_glob, n_local, ghost_ids, /*fast=*/false, /*ptype=*/GHOSTED);
  };
  safe_reinit(_mass_matrix_diag_inverted);
  safe_reinit(_mass_matrix_lumped);
}
