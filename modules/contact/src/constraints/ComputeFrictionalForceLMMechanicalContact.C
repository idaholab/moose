//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeFrictionalForceLMMechanicalContact.h"
#include "DisplacedProblem.h"
#include "Assembly.h"

#include "DualRealOps.h"

#include "metaphysicl/dualsemidynamicsparsenumberarray.h"
#include "metaphysicl/parallel_dualnumber.h"
#include "metaphysicl/parallel_dynamic_std_array_wrapper.h"
#include "metaphysicl/parallel_semidynamicsparsenumberarray.h"
#include "timpi/parallel_sync.h"

registerMooseObject("ContactApp", ComputeFrictionalForceLMMechanicalContact);

InputParameters
ComputeFrictionalForceLMMechanicalContact::validParams()
{
  InputParameters params = ComputeWeightedGapLMMechanicalContact::validParams();
  params.addClassDescription("Computes the tangential frictional forces");
  params.addRequiredCoupledVar("friction_lm", "The frictional Lagrange's multiplier");
  params.addCoupledVar("friction_lm_dir",
                       "The frictional Lagrange's multiplier for an addtional direction.");

  params.addParam<Real>("c_t", 1e0, "Numerical parameter for tangential constraints");
  params.addParam<Real>(
      "epsilon",
      1.0e-7,
      "Minimum value of contact pressure that will trigger frictional enforcement");
  params.addRequiredParam<Real>("mu", "The friction coefficient for the Coulomb friction law");
  return params;
}

ComputeFrictionalForceLMMechanicalContact::ComputeFrictionalForceLMMechanicalContact(
    const InputParameters & parameters)
  : ComputeWeightedGapLMMechanicalContact(parameters),
    _c_t(getParam<Real>("c_t")),
    _friction_var(getVar("friction_lm", 0)),
    _secondary_x_dot(_secondary_var.adUDot()),
    _primary_x_dot(_primary_var.adUDotNeighbor()),
    _secondary_y_dot(adCoupledDot("disp_y")),
    _primary_y_dot(adCoupledNeighborValueDot("disp_y")),
    _secondary_z_dot(_has_disp_z ? &adCoupledDot("disp_z") : nullptr),
    _primary_z_dot(_has_disp_z ? &adCoupledNeighborValueDot("disp_z") : nullptr),
    _epsilon(getParam<Real>("epsilon")),
    _mu(getParam<Real>("mu")),
    _friction_var_dir(getVar("friction_lm_dir", 0)),
    _3d(_has_disp_z)

{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError(
      "ComputeFrictionalForceLMMechanicalContact relies on use of the global indexing container "
      "in order to make its implementation feasible");
#endif

  if (!getParam<bool>("use_displaced_mesh"))
    paramError("use_displaced_mesh",
               "'use_displaced_mesh' must be true for the "
               "ComputeFrictionalForceLMMechanicalContact object");

  if (!_friction_var->isNodal())
    if (_friction_var->feType().order != static_cast<Order>(0))
      mooseError(
          "Frictional contact constraints only support elemental variables of CONSTANT order");

  if (_3d && !_friction_var_dir)
    paramError("friction_lm_dir",
               "Three-dimensional mortar frictional contact simulations require an additional "
               "frictional Lagrange's multiplier to enforce a second tangential pressure");
}

void
ComputeFrictionalForceLMMechanicalContact::computeQpProperties()
{
  // Compute the value of _qp_gap
  ComputeWeightedGapLMMechanicalContact::computeQpProperties();

  ADRealVectorValue relative_velocity(
      _secondary_x_dot[_qp] - _primary_x_dot[_qp], _secondary_y_dot[_qp] - _primary_y_dot[_qp], 0);
  // Add derivative information
  relative_velocity(0).derivatives() =
      _secondary_x_dot[_qp].derivatives() - _primary_x_dot[_qp].derivatives();
  relative_velocity(1).derivatives() =
      _secondary_y_dot[_qp].derivatives() - _primary_y_dot[_qp].derivatives();
  if (_has_disp_z)
    relative_velocity(2).derivatives() =
        (*_secondary_z_dot)[_qp].derivatives() - (*_primary_z_dot)[_qp].derivatives();

  // Get the component in the tangential direction
  // TODO: Create a consistent mortar tangential vector field (right now tangents from libmesh)
  _qp_tangential_velocity = relative_velocity * (_tangents[_qp][0] * _JxW_msm[_qp] * _coord[_qp]);

  if (_3d)
    _qp_tangential_velocity_dir =
        relative_velocity * (_tangents[_qp][1] * _JxW_msm[_qp] * _coord[_qp]);
}

void
ComputeFrictionalForceLMMechanicalContact::computeQpIProperties()
{
  // Get the _dof_to_weighted_gap map
  ComputeWeightedGapLMMechanicalContact::computeQpIProperties();

  // Get the _dof_to_weighted_tangential_velocity map
  const DofObject * dof = _friction_var->isNodal()
                              ? static_cast<const DofObject *>(_lower_secondary_elem->node_ptr(_i))
                              : static_cast<const DofObject *>(_lower_secondary_elem);

  _dof_to_weighted_tangential_velocity[dof] += _test[_i][_qp] * _qp_tangential_velocity;

  // Get the _dof_to_weighted_tangential_velocity map for a second direction
  if (_3d)
  {
    const DofObject * dof_dir =
        _friction_var_dir->isNodal()
            ? static_cast<const DofObject *>(_lower_secondary_elem->node_ptr(_i))
            : static_cast<const DofObject *>(_lower_secondary_elem);

    _dof_to_weighted_tangential_velocity_dir[dof_dir] +=
        _test[_i][_qp] * _qp_tangential_velocity_dir;
  }
}

void
ComputeFrictionalForceLMMechanicalContact::residualSetup()
{
  // Clear both maps
  ComputeWeightedGapLMMechanicalContact::residualSetup();

  _dof_to_weighted_tangential_velocity.clear();

  if (_3d)
    _dof_to_weighted_tangential_velocity_dir.clear();
}

void
ComputeFrictionalForceLMMechanicalContact::communicateVelocities()
{
#ifdef MOOSE_SPARSE_AD
  // We may have weighted gap information that should go to other processes that own the dofs
  using Datum = std::pair<dof_id_type, ADReal>;
  std::unordered_map<processor_id_type, std::vector<Datum>> push_data;

  // Same map for additional direction (three dimensions) -- must use same dofs
  std::unordered_map<processor_id_type, std::vector<Datum>> push_data_dir;

  for (auto & pr : _dof_to_weighted_tangential_velocity)
  {
    const auto * const dof_object = pr.first;
    const auto proc_id = dof_object->processor_id();
    if (proc_id == this->processor_id())
      continue;

    push_data[proc_id].push_back(std::make_pair(dof_object->id(), std::move(pr.second)));
    push_data_dir[proc_id].push_back(std::make_pair(
        dof_object->id(), std::move(_dof_to_weighted_tangential_velocity_dir[dof_object])));
  }

  const auto & lm_mesh = _mesh.getMesh();

  auto action_functor = [this, &lm_mesh](const processor_id_type libmesh_dbg_var(pid),
                                         const std::vector<Datum> & sent_data) {
    mooseAssert(pid != this->processor_id(), "We do not send messages to ourself here");
    for (auto & pr : sent_data)
    {
      const auto dof_id = pr.first;
      const auto * const dof_object =
          _nodal ? static_cast<const DofObject *>(lm_mesh.node_ptr(dof_id))
                 : static_cast<const DofObject *>(lm_mesh.elem_ptr(dof_id));
      mooseAssert(dof_object, "This should be non-null");
      _dof_to_weighted_tangential_velocity[dof_object] += std::move(pr.second);
    }
  };

  TIMPI::push_parallel_vector_data(_communicator, push_data, action_functor);

  if (_3d)
  {
    auto action_functor_dir = [this, &lm_mesh](const processor_id_type libmesh_dbg_var(pid),
                                               const std::vector<Datum> & sent_data) {
      mooseAssert(pid != this->processor_id(), "We do not send messages to ourself here");
      for (auto & pr : sent_data)
      {
        const auto dof_id = pr.first;
        const auto * const dof_object =
            _nodal ? static_cast<const DofObject *>(lm_mesh.node_ptr(dof_id))
                   : static_cast<const DofObject *>(lm_mesh.elem_ptr(dof_id));
        mooseAssert(dof_object, "This should be non-null");
        _dof_to_weighted_tangential_velocity_dir[dof_object] += std::move(pr.second);
      }
    };
    TIMPI::push_parallel_vector_data(_communicator, push_data_dir, action_functor_dir);
  }
#endif
}

void
ComputeFrictionalForceLMMechanicalContact::post()
{
  communicateGaps();
  communicateVelocities();

  // Enforce frictional complementarity constraints
  for (const auto & pr : _dof_to_weighted_tangential_velocity)
  {
    const DofObject * const dof = pr.first;

    if (dof->processor_id() != this->processor_id())
      continue;

    auto & weighted_gap_pr = _dof_to_weighted_gap[dof];
    _weighted_gap_ptr = &weighted_gap_pr.first;
    _normalization_ptr = &weighted_gap_pr.second;
    _tangential_vel_ptr = &pr.second;

    if (_3d)
    {
      _tangential_vel_dir_ptr = &_dof_to_weighted_tangential_velocity[pr.first];
      enforceConstraintOnDof3d(dof);
    }
    else
      enforceConstraintOnDof(dof);
  }
}

void
ComputeFrictionalForceLMMechanicalContact::incorrectEdgeDroppingPost(
    const std::unordered_set<const Node *> & inactive_lm_nodes)
{
  communicateGaps();
  communicateVelocities();

  // Enforce frictional complementarity constraints
  for (const auto & pr : _dof_to_weighted_tangential_velocity)
  {
    const DofObject * const dof = pr.first;

    // If node inactive, skip
    if ((inactive_lm_nodes.find(static_cast<const Node *>(dof)) != inactive_lm_nodes.end()) ||
        (dof->processor_id() != this->processor_id()))
      continue;

    _weighted_gap_ptr = &_dof_to_weighted_gap[dof].first;
    _tangential_vel_ptr = &pr.second;
    if (_3d)
    {
      _tangential_vel_dir_ptr = &_dof_to_weighted_tangential_velocity[pr.first];
      enforceConstraintOnDof3d(dof);
    }
    else
      enforceConstraintOnDof(dof);
  }
}

void
ComputeFrictionalForceLMMechanicalContact::enforceConstraintOnDof3d(const DofObject * const dof)
{
  ComputeWeightedGapLMMechanicalContact::enforceConstraintOnDof(dof);

  // Get normal LM
  const auto normal_dof_index = dof->dof_number(_sys.number(), _var->number(), 0);
  const ADReal & weighted_gap = *_weighted_gap_ptr;
  ADReal contact_pressure = (*_sys.currentSolution())(normal_dof_index);
  Moose::derivInsert(contact_pressure.derivatives(), normal_dof_index, 1.);

  // Get friction LM
  const auto friction_dof_index = dof->dof_number(_sys.number(), _friction_var->number(), 0);
  const ADReal & tangential_vel = *_tangential_vel_ptr;
  ADReal friction_lm_value = (*_sys.currentSolution())(friction_dof_index);
  Moose::derivInsert(friction_lm_value.derivatives(), friction_dof_index, 1.);

  // Get friction LM for second direction (three dimensions)
  const auto friction_dir_dof_index =
      dof->dof_number(_sys.number(), _friction_var_dir->number(), 0);
  const ADReal & tangential_dir_vel = *_tangential_vel_dir_ptr;
  ADReal friction_dir_lm_value = (*_sys.currentSolution())(friction_dir_dof_index);
  Moose::derivInsert(friction_dir_lm_value.derivatives(), friction_dir_dof_index, 1.);

  // Get normalized c and c_t values (if normalization specified
  const Real c = _normalize_c ? _c / *_normalization_ptr : _c;
  const Real c_t = _normalize_c ? _c_t / *_normalization_ptr : _c_t;

  ADReal dof_residual;
  ADReal dof_residual_dir;

  // Primal-dual active set strategy (PDASS)
  if (contact_pressure < _epsilon)
  {
    dof_residual = friction_lm_value;
    dof_residual_dir = friction_dir_lm_value;
  }
  else
  {
    Real epsilon_ad = 0.0;
    const auto lamdba_plus_cg = contact_pressure + c * weighted_gap;
    std::array<ADReal, 2> lambda_t_plus_ctu;
    lambda_t_plus_ctu[0] = friction_lm_value + c_t * tangential_vel * _dt;
    lambda_t_plus_ctu[1] = friction_dir_lm_value + c_t * tangential_dir_vel * _dt;

    const auto term_1_x =
        std::max(_mu * lamdba_plus_cg,
                 std::sqrt(lambda_t_plus_ctu[0] * lambda_t_plus_ctu[0] +
                           lambda_t_plus_ctu[1] * lambda_t_plus_ctu[1] + epsilon_ad)) *
        friction_lm_value;

    const auto term_1_y =
        std::max(_mu * lamdba_plus_cg,
                 std::sqrt(lambda_t_plus_ctu[0] * lambda_t_plus_ctu[0] +
                           lambda_t_plus_ctu[1] * lambda_t_plus_ctu[1] + epsilon_ad)) *
        friction_dir_lm_value;

    const auto term_2_x = _mu * std::max(0.0, lamdba_plus_cg) * lambda_t_plus_ctu[0];

    const auto term_2_y = _mu * std::max(0.0, lamdba_plus_cg) * lambda_t_plus_ctu[1];

    dof_residual = term_1_x - term_2_x;
    dof_residual_dir = term_1_y - term_2_y;
  }

  if (_subproblem.currentlyComputingJacobian())
  {
    // This should be okay since we adding different Jacobian rows (dofs)
    _assembly.processDerivatives(dof_residual, friction_dof_index, _matrix_tags);
    _assembly.processDerivatives(dof_residual_dir, friction_dir_dof_index, _matrix_tags);
  }
  else
  {
    _assembly.cacheResidual(friction_dof_index, dof_residual.value(), _vector_tags);
    _assembly.cacheResidual(friction_dir_dof_index, dof_residual_dir.value(), _vector_tags);
  }
}

void
ComputeFrictionalForceLMMechanicalContact::enforceConstraintOnDof(const DofObject * const dof)
{
  ComputeWeightedGapLMMechanicalContact::enforceConstraintOnDof(dof);

  // Get friction LM
  const auto friction_dof_index = dof->dof_number(_sys.number(), _friction_var->number(), 0);
  const ADReal & tangential_vel = *_tangential_vel_ptr;
  ADReal friction_lm_value = (*_sys.currentSolution())(friction_dof_index);
  Moose::derivInsert(friction_lm_value.derivatives(), friction_dof_index, 1.);

  // Get normal LM
  const auto normal_dof_index = dof->dof_number(_sys.number(), _var->number(), 0);
  const ADReal & weighted_gap = *_weighted_gap_ptr;
  ADReal contact_pressure = (*_sys.currentSolution())(normal_dof_index);
  Moose::derivInsert(contact_pressure.derivatives(), normal_dof_index, 1.);

  // Get normalized c and c_t values (if normalization specified
  const Real c = _normalize_c ? _c / *_normalization_ptr : _c;
  const Real c_t = _normalize_c ? _c_t / *_normalization_ptr : _c_t;

  ADReal dof_residual;

  // Primal-dual active set strategy (PDASS)
  if (contact_pressure < _epsilon)
    dof_residual = friction_lm_value;
  else
  {
    const auto term_1 = std::max(_mu * (contact_pressure + c * weighted_gap),
                                 std::abs(friction_lm_value + c_t * tangential_vel * _dt)) *
                        friction_lm_value;
    const auto term_2 = _mu * std::max(0.0, contact_pressure + c * weighted_gap) *
                        (friction_lm_value + c_t * tangential_vel * _dt);

    dof_residual = term_1 - term_2;
  }

  if (_subproblem.currentlyComputingJacobian())
    _assembly.processDerivatives(dof_residual, friction_dof_index, _matrix_tags);
  else
    _assembly.cacheResidual(friction_dof_index, dof_residual.value(), _vector_tags);
}
