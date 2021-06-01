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
}

void
ComputeFrictionalForceLMMechanicalContact::computeQpProperties()
{
  // Compute the value of _qp_gap
  ComputeWeightedGapLMMechanicalContact::computeQpProperties();

  // Compute the value of weighted tangential velocity
  if (_has_primary)
  {
    ADRealVectorValue relative_velocity(_secondary_x_dot[_qp] - _primary_x_dot[_qp],
                                        _secondary_y_dot[_qp] - _primary_y_dot[_qp],
                                        0);
    // Add derivative information
    relative_velocity(0).derivatives() =
        _secondary_x_dot[_qp].derivatives() - _primary_x_dot[_qp].derivatives();
    relative_velocity(1).derivatives() =
        _secondary_y_dot[_qp].derivatives() - _primary_y_dot[_qp].derivatives();

    // Get the component in the tangential direction
    // TODO: Create a consistent mortar tangential vector field
    _qp_tangential_velocity = relative_velocity * (_tangents[_qp][0] * _JxW_msm[_qp] * _coord[_qp]);
  }
  else
    _qp_tangential_velocity = std::numeric_limits<Real>::quiet_NaN();
}

void
ComputeFrictionalForceLMMechanicalContact::computeQpIProperties()
{
  // Get the _node_to_weighted_gap map
  ComputeWeightedGapLMMechanicalContact::computeQpIProperties();

  // Get the _node_to_weighted_tangential_velocity map
  const auto * const node = _lower_secondary_elem->node_ptr(_i);
  _node_to_weighted_tangential_velocity[node] += _test[_i][_qp] * _qp_tangential_velocity;
}

void
ComputeFrictionalForceLMMechanicalContact::residualSetup()
{
  // Clear both maps
  ComputeWeightedGapLMMechanicalContact::residualSetup();

  _node_to_weighted_tangential_velocity.clear();
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
  for (const auto & pr : _node_to_weighted_tangential_velocity)
  {
    _weighted_gap_ptr = &_node_to_weighted_gap[pr.first];
    _tangential_vel_ptr = &pr.second;
    enforceConstraintOnNode(pr.first);
  }
}

void
ComputeFrictionalForceLMMechanicalContact::enforceConstraintOnNode(const Node * const node)
{
  ComputeWeightedGapLMMechanicalContact::enforceConstraintOnNode(node);

  // Get friction LM index
  const auto dof_index = node->dof_number(_sys.number(), _friction_var->number(), 0);
  ADReal friction_lm_value = _friction_var->getNodalValue(*node);
  Moose::derivInsert(friction_lm_value.derivatives(), dof_index, 1.);
  const ADReal & tangential_vel = *_tangential_vel_ptr;

  // Get normal LM index
  ADReal contact_pressure = _var->getNodalValue(*node);
  const auto dof_index_cp = node->dof_number(_sys.number(), _var->number(), 0);
  Moose::derivInsert(contact_pressure.derivatives(), dof_index_cp, 1.);

  const auto & weighted_gap = *_weighted_gap_ptr;
  ADReal nodal_residual;

  // Primal-dual active set strategy (PDASS)
  if (contact_pressure < _epsilon || std::isnan(weighted_gap) || std::isnan(tangential_vel))
    nodal_residual = friction_lm_value;
  else
  {
    const auto term_1 = std::max(_mu * (contact_pressure + _c * weighted_gap),
                                 std::abs(friction_lm_value + _c_t * tangential_vel * _dt)) *
                        friction_lm_value;
    const auto term_2 = _mu * std::max(0.0, contact_pressure + _c * weighted_gap) *
                        (friction_lm_value + _c_t * tangential_vel * _dt);

    nodal_residual = term_1 - term_2;
  }

  if (_subproblem.currentlyComputingJacobian())
    _assembly.processDerivatives(nodal_residual, dof_index, _matrix_tags);
  else
    _assembly.cacheResidual(dof_index, nodal_residual.value(), _vector_tags);
}
