//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NormalMortarMechanicalContact.h"
#include "WeightedGapUserObject.h"

registerMooseObject("ContactApp", NormalMortarMechanicalContact);

InputParameters
NormalMortarMechanicalContact::validParams()
{
  InputParameters params = ADMortarLagrangeConstraint::validParams();

  MooseEnum component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "component", component, "The force component constraint that this object is supplying");
  params.addClassDescription(
      "This class is used to apply normal contact forces using lagrange multipliers");
  params.set<bool>("compute_lm_residual") = false;
  params.set<bool>("interpolate_normals") = false;
  params.addRequiredParam<UserObjectName>("weighted_gap_uo", "The weighted gap user object.");
  return params;
}

NormalMortarMechanicalContact::NormalMortarMechanicalContact(const InputParameters & parameters)
  : ADMortarLagrangeConstraint(parameters),
    _component(getParam<MooseEnum>("component")),
    _weighted_gap_uo(getUserObject<WeightedGapUserObject>("weighted_gap_uo"))
{
  if (getParam<bool>("interpolate_normals"))
    paramError("interpolate_normals",
               "Mechanical mortar contact uses normalized secondary nodal normals and cannot be "
               "combined with quadrature-point normal interpolation.");
}

ADReal
NormalMortarMechanicalContact::computeQpResidual(Moose::MortarType type)
{
  // Interpolate the nodal traction vectors, sum_j Phi_j z_j n_j, rather than scaling an
  // interpolated scalar pressure by the nodal normal belonging to this row's node. Only the former
  // is the transpose of the weighted gap, so only the former keeps the two sides of the interface
  // in equilibrium. Scaling by a row's own normal also made the primary-side force depend on
  // secondary node numbering, because the primary row index was used to look up a normal in an
  // array indexed by secondary node.
  const auto & phi = _weighted_gap_uo.tractionBasis();
  const bool ad_normals = _weighted_gap_uo.usesNodalNormalDerivatives();
  ADReal traction_component = 0;
  for (const auto j : index_range(phi))
  {
    const auto nodal_pressure =
        _weighted_gap_uo.nodalContactPressure(_lower_secondary_elem->node_ref(j));

    // Take the geometric normals from this constraint, whose mortar state is reinitialized in
    // this loop. A user object can be configured on a different interface than the constraint that
    // consumes it, in which case its own copy is never populated.
    if (ad_normals)
      traction_component += phi[j][_qp] * nodal_pressure *
                            _weighted_gap_uo.contactNormal(*_lower_secondary_elem, j)(_component);
    else
      traction_component += phi[j][_qp] * nodal_pressure * _normals[j](_component);
  }

  switch (type)
  {
    case Moose::MortarType::Secondary:
      // If the traction is positive, then this residual is positive, indicating that we have an
      // outflow of momentum, which in turn indicates that the momentum will tend to decrease at
      // this location with time, which is what we want because the force vector is in the negative
      // direction (always opposite of the normals). Conversely, if the traction is negative, then
      // this residual is negative, indicating that we have an inflow of momentum, which in turn
      // indicates the momentum will tend to increase at this location with time, which is what we
      // want because the force vector is in the positive direction (always opposite of the
      // normals).
      return _test_secondary[_i][_qp] * traction_component;

    case Moose::MortarType::Primary:
      // The traction is signed according to the secondary face, so we need to introduce a negative
      // sign here
      return -_test_primary[_i][_qp] * traction_component;

    default:
      return 0;
  }
}
