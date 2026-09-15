//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TangentialMortarMechanicalContact.h"
#include "WeightedVelocitiesUserObject.h"
#include "AutomaticMortarGeneration.h"

registerMooseObject("ContactApp", TangentialMortarMechanicalContact);

InputParameters
TangentialMortarMechanicalContact::validParams()
{
  InputParameters params = ADMortarLagrangeConstraint::validParams();

  MooseEnum component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "component", component, "The force component constraint that this object is supplying");

  // This enum is used to pick the proper 'tangent' vector (i.e. tangent or binormal)
  MooseEnum direction("direction_1 direction_2", "direction_1");
  params.addParam<MooseEnum>("direction",
                             direction,
                             "Tangent direction to compute the residual due to frictional contact");
  params.addClassDescription(
      "Used to apply tangential stresses from frictional contact using lagrange multipliers");
  params.addRequiredParam<UserObjectName>("weighted_velocities_uo",
                                          "The weighted velocities user object.");
  params.set<bool>("interpolate_normals") = false;
  params.set<bool>("compute_lm_residual") = false;
  return params;
}

TangentialMortarMechanicalContact::TangentialMortarMechanicalContact(
    const InputParameters & parameters)
  : ADMortarLagrangeConstraint(parameters),
    _component(getParam<MooseEnum>("component")),
    _direction(getParam<MooseEnum>("direction")),
    _weighted_velocities_uo(getUserObject<WeightedVelocitiesUserObject>("weighted_velocities_uo"))
{
  if (getParam<bool>("interpolate_normals"))
    paramError("interpolate_normals",
               "Mechanical mortar contact uses tangents derived from normalized secondary nodal "
               "normals and cannot be combined with quadrature-point normal interpolation.");

  // Same reasoning as NormalMortarMechanicalContact: this constraint only reads the resulting
  // flag, so the check below only applies once the user object confirms derivatives are on. A
  // mismatched interface here can leave the user object's normal cache empty for this
  // constraint's nodes (hard-erroring inside libmesh_map_find rather than here), and a mismatched
  // displacement variable would read the wrong derivative indices from the cached normal.
  if (_weighted_velocities_uo.nodalNormalDerivativesEnabled())
  {
    if (secondarySubdomain() != _weighted_velocities_uo.secondarySubdomain() ||
        primarySubdomain() != _weighted_velocities_uo.primarySubdomain())
      paramError("weighted_velocities_uo",
                 "'weighted_velocities_uo' must be defined on the same secondary/primary "
                 "subdomain pair as this constraint when nodal-normal derivatives are enabled.");

    if (&_secondary_var != _weighted_velocities_uo.dispVar(_component))
      paramError("weighted_velocities_uo",
                 "'weighted_velocities_uo' must use the same displacement variable as this "
                 "constraint's 'variable' when nodal-normal derivatives are enabled.");

    if (getParam<bool>("use_displaced_mesh") !=
        _weighted_velocities_uo.parameters().get<bool>("use_displaced_mesh"))
      paramError("weighted_velocities_uo",
                 "'weighted_velocities_uo' must use the same 'use_displaced_mesh' setting as "
                 "this constraint when nodal-normal derivatives are enabled.");
  }
}

ADReal
TangentialMortarMechanicalContact::computeQpResidual(Moose::MortarType type)
{
  // Interpolate the nodal frictional traction vectors, sum_j Phi_j z_j t_j, rather than scaling an
  // interpolated scalar pressure by the tangent belonging to this row's node. Each node carries its
  // own Householder tangent frame, and those frames are unrelated between nodes, so pairing one
  // node's tangential coefficient with another node's frame is not a small error. See
  // WeightedGapUserObject::nodalContactPressure for why interpolating the vector is the consistent
  // choice.
  const auto direction = static_cast<unsigned int>(_direction);
  // tangentialTractionBasis(direction), not tractionBasis(): the latter is the normal Lagrange
  // multiplier's basis, so both this loop's bound and its dof lookup below must come from this
  // direction's own tangential Lagrange multiplier -- the same variable nodalTangentialPressure
  // reads its coefficients from -- or a node could be paired with another node's coefficient.
  const auto & phi = _weighted_velocities_uo.tangentialTractionBasis(direction);
  const bool ad_tangents = _weighted_velocities_uo.usesNodalNormalDerivatives();

  // Take the geometric tangents from this constraint's mortar generation, whose state is
  // reinitialized in this loop. A user object can be configured on a different interface than the
  // constraint that consumes it, in which case its own copy is never populated.
  const auto & nodal_tangents = amg().getNodalTangents(*_lower_secondary_elem);

  ADReal traction_component = 0;
  for (const auto j : index_range(phi))
  {
    const auto nodal_pressure = _weighted_velocities_uo.nodalTangentialPressure(
        _lower_secondary_elem->node_ref(j), direction);

    // householderTangents() applies a Householder reflection to two Cartesian basis vectors, so the
    // frame it returns is already orthonormal and needs no normalization here.
    if (ad_tangents)
    {
      const auto & tangents = _weighted_velocities_uo.contactTangents(*_lower_secondary_elem, j);
      traction_component += phi[j][_qp] * nodal_pressure * tangents[direction](_component);
    }
    else
      traction_component += phi[j][_qp] * nodal_pressure * nodal_tangents[direction][j](_component);
  }

  switch (type)
  {
    case Moose::MortarType::Secondary:
      // We have taken the convention the lagrange multiplier must have the same sign as the
      // relative slip velocity of the secondary face. So positive lambda indicates that force is
      // being applied in the negative direction, so we want to decrease the momentum in the system,
      // which means we want an outflow of momentum, which means we want the residual to be positive
      // in that case. Negative lambda means force is being applied in the positive direction, so we
      // want to increase momentum in the system, which means we want an inflow of momentum, which
      // means we want the residual to be negative in that case. So the sign of this residual should
      // be the same as the sign of lambda
      return _test_secondary[_i][_qp] * traction_component;

    case Moose::MortarType::Primary:
      return -_test_primary[_i][_qp] * traction_component;

    default:
      return 0;
  }
}
