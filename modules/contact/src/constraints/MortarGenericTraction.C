//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarGenericTraction.h"
#include "CohesiveZoneModelBase.h"

registerMooseObject("ContactApp", MortarGenericTraction);

InputParameters
MortarGenericTraction::validParams()
{
  InputParameters params = ADMortarLagrangeConstraint::validParams();
  // Mechanical contact residuals use only primal values and normals
  params.set<bool>("_compute_primal_gradients") = false;

  MooseEnum component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "component", component, "The force component constraint that this object is supplying");
  params.addClassDescription(
      "Applies the traction of a mortar cohesive zone user object to the secondary and primary "
      "sides of the interface.");
  params.addRequiredParam<UserObjectName>("cohesive_zone_uo", "The cohesive zone user object.");
  params.set<bool>("interpolate_normals") = false;
  params.set<bool>("compute_lm_residual") = false;
  // The cohesive zone user object runs on the undisplaced mesh and is only reinitialized for
  // constraints that also run there.
  params.set<bool>("use_displaced_mesh") = false;
  return params;
}

MortarGenericTraction::MortarGenericTraction(const InputParameters & parameters)
  : ADMortarLagrangeConstraint(parameters),
    _component(getParam<MooseEnum>("component")),
    _cohesize_zone_uo(const_cast<CohesiveZoneModelBase &>(
        getUserObject<CohesiveZoneModelBase>("cohesive_zone_uo")))
{
  if (getParam<bool>("use_displaced_mesh") !=
      _cohesize_zone_uo.getParam<bool>("use_displaced_mesh"))
    paramError("use_displaced_mesh",
               "'use_displaced_mesh' must match that of the cohesive zone user object, which is "
               "only reinitialized on its own mortar interface.");

  // The traction is interpolated on the mortar segments of the user object's interface
  if (secondarySubdomain() != _cohesize_zone_uo.secondarySubdomain() ||
      primarySubdomain() != _cohesize_zone_uo.primarySubdomain())
    paramError("cohesive_zone_uo",
               "'cohesive_zone_uo' must be defined on the same secondary/primary subdomain pair as "
               "this constraint.");
}

ADReal
MortarGenericTraction::computeQpResidual(Moose::MortarType type)
{
  switch (type)
  {
    case Moose::MortarType::Secondary:
      return _test_secondary[_i][_qp] * _cohesize_zone_uo.czmGlobalTraction(_component)[_qp];
    case Moose::MortarType::Primary:
      return -_test_primary[_i][_qp] * _cohesize_zone_uo.czmGlobalTraction(_component)[_qp];

    default:
      return 0;
  }
}
