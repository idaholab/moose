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

registerMooseObject("ContactApp", ComputeFrictionalForceLMMechanicalContact);

InputParameters
ComputeFrictionalForceLMMechanicalContact::validParams()
{
  InputParameters params = ComputeWeightedGapLMMechanicalContact::validParams();
  params.addClassDescription("Computes the tangential frictional forces");
  params.addRequiredCoupledVar("friction_lm", "The frictional Lagrange's multiplier");
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
    _mu(getParam<Real>("mu"))
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
  // TODO: Create a consistent mortar tangential vector field
  _qp_tangential_velocity = relative_velocity * (_tangents[_qp][0] * _JxW_msm[_qp] * _coord[_qp]);
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
}

void
ComputeFrictionalForceLMMechanicalContact::residualSetup()
{
  // Clear both maps
  ComputeWeightedGapLMMechanicalContact::residualSetup();

  _dof_to_weighted_tangential_velocity.clear();
}

void
ComputeFrictionalForceLMMechanicalContact::jacobianSetup()
{
  residualSetup();
}

void
ComputeFrictionalForceLMMechanicalContact::post()
{
  // Enforce frictional complementarity constraints
  for (const auto & pr : _dof_to_weighted_tangential_velocity)
  {
    const DofObject * const dof = pr.first;

    _weighted_gap_ptr = &_dof_to_weighted_gap[dof].first;
    _tangential_vel_ptr = &pr.second;

    enforceConstraintOnDof(dof);
  }
}

void
ComputeFrictionalForceLMMechanicalContact::incorrectEdgeDroppingPost(
    const std::unordered_set<const Node *> & inactive_lm_nodes)
{
  // Enforce frictional complementarity constraints
  for (const auto & pr : _dof_to_weighted_tangential_velocity)
  {
    // If node inactive, skip
    if (inactive_lm_nodes.find(static_cast<const Node *>(pr.first)) != inactive_lm_nodes.end())
      continue;

    const DofObject * const dof = pr.first;

    _weighted_gap_ptr = &_dof_to_weighted_gap[dof].first;
    _tangential_vel_ptr = &pr.second;

    enforceConstraintOnDof(dof);
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
