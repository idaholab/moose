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
    _epsilon(1.0e-7),
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
  // Get the value of _qp_gap
  ComputeWeightedGapLMMechanicalContact::computeQpProperties();

  // Compute the value of weighted tangential velocity
  // Build the velocity vector

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
    _qp_tangential_velocity = relative_velocity * _tangents[_qp][0];
  }
  else
    _qp_tangential_velocity = std::numeric_limits<Real>::quiet_NaN();
}

void
ComputeFrictionalForceLMMechanicalContact::computeQpIProperties()
{
  // Get the _node_to_weighted_gap map
  ComputeWeightedGapLMMechanicalContact::computeQpIProperties();

  const auto * const node = _lower_secondary_elem->node_ptr(_i);
  _node_to_weighted_tangential_velocity[node] += _test[_i][_qp] * _qp_tangential_velocity;
  //  Moose::out << "Node locations: x: " << _lower_secondary_elem->node_ptr(_i)->get_info() <<
  //  "\n"; Moose::out << "_node_to_weighted_tangential_velocity[node].size(): "
  //             << _node_to_weighted_tangential_velocity.size() << "\n";
  //  Moose::out << "qp_tangential_vel: " << MetaPhysicL::raw_value(_qp_tangential_velocity) <<
  //  "\n"; Moose::out << "_secondary_x_dot[_qp]: " << MetaPhysicL::raw_value(_secondary_x_dot[_qp])
  //  << "\n";
}

void
ComputeFrictionalForceLMMechanicalContact::residualSetup()
{
  // Clear _node_to_weighted_gap map
  ComputeWeightedGapLMMechanicalContact::residualSetup();

  _node_to_weighted_tangential_velocity.clear();
}

void
ComputeFrictionalForceLMMechanicalContact::jacobianSetup()
{
  residualSetup();
}

void
ComputeFrictionalForceLMMechanicalContact::computeResidual(const Moose::MortarType mortar_type)
{
  if (mortar_type != Moose::MortarType::Lower)
    return;

  // ComputeWeightedGapLMMechanicalContact::computeResidual(mortar_type);

  // Fill node to weighted tangential velocity map.
  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
  {
    computeQpProperties();
    for (_i = 0; _i < _test.size(); ++_i)
      computeQpIProperties();
  }
}

void
ComputeFrictionalForceLMMechanicalContact::computeJacobian(const Moose::MortarType mortar_type)
{
  // During "computeResidual" and "computeJacobian" we are actually just computing properties on the
  // mortar segment element mesh. We are *not* actually assembling into the residual/Jacobian. For
  // the zero-penetration constraint, the property of interest is the map from node to weighted gap.
  // Computation of the properties proceeds identically for residual and Jacobian evaluation hence
  // why we simply call computeResidual here. We will assemble into the residual/Jacobian later from
  // the post() method
  computeResidual(mortar_type);
}

void
ComputeFrictionalForceLMMechanicalContact::post()
{
  // Enforce weighted gap constraint (calls enforceConstraintOnNode)
  ComputeWeightedGapLMMechanicalContact::post();

  // Enforce weighted tangential velocity constraint
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
  // Enforce normal contact constraint
  // ComputeWeightedGapLMMechanicalContact::enforceConstraintOnNode(node);

  // Get friction LM index
  const auto dof_index = node->dof_number(_sys.number(), _friction_var->number(), 0);
  ADReal friction_lm_value = _friction_var->getNodalValue(*node);
  Moose::derivInsert(friction_lm_value.derivatives(), dof_index, 1.);
  const ADReal & tangential_vel = *_tangential_vel_ptr;
  // const Real tangential_vel = -0.002;

  // Get normal LM index
  ADReal contact_pressure = _var->getNodalValue(*node);
  const auto dof_index_cp = node->dof_number(_sys.number(), _var->number(), 0);
  Moose::derivInsert(contact_pressure.derivatives(), dof_index_cp, 1.);

  const auto & weighted_gap = *_weighted_gap_ptr;
  ADReal nodal_residual;

  // This below causes a seg fault. Wonder why
  // Moose::out << "tangential_vel: " << MetaPhysicL::raw_value(tangential_vel) << "\n";

  // Implementation of primal-dual active set strategy (residual)

  if (contact_pressure < _epsilon || std::isnan(weighted_gap) || std::isnan(tangential_vel))
    nodal_residual = friction_lm_value;
  else
  {

    //    Moose::out << "tangential_vel: " << MetaPhysicL::raw_value(tangential_vel) << "\n";
    //    Moose::out << "_node_to_weighted_tangential_velocity.size() from inside: "
    //               << _node_to_weighted_tangential_velocity.size() << "\n";
    // Form 1
    auto term_1 = std::max(_mu * (contact_pressure + _c * weighted_gap),
                           std::abs(friction_lm_value + _c_t * tangential_vel * _dt)) *
                  friction_lm_value;
    auto term_2 = _mu * std::max(0.0, contact_pressure + _c * weighted_gap) *
                  (friction_lm_value + _c_t * tangential_vel * _dt);

    // friction_lm_value = (term_1 - term_2);

    // Form 2
    //    ADReal a;
    //    if (tangential_vel * friction_lm_value < 0)
    //      a = -std::abs(tangential_vel);
    //    else
    //      a = std::abs(tangential_vel);
    //
    //    // NCP part 2: require that the frictional force can never exceed the frictional
    //    // coefficient times the normal force.
    //    auto b = _mu * contact_pressure;
    nodal_residual = term_1 - term_2;
  }

  if (_subproblem.currentlyComputingJacobian())
    _assembly.processDerivatives(nodal_residual, dof_index, _matrix_tags);
  else
    _assembly.cacheResidual(dof_index, nodal_residual.value(), _vector_tags);
}
