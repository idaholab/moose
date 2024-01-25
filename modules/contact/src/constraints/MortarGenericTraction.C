//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarGenericTraction.h"
#include "BilinearMixedModeCohesiveZoneModel.h"

registerMooseObject("ContactApp", MortarGenericTraction);

InputParameters
MortarGenericTraction::validParams()
{
  InputParameters params = ADMortarLagrangeConstraint::validParams();

  MooseEnum component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "component", component, "The force component constraint that this object is supplying");
  params.addClassDescription(
      "Used to apply tangential stresses from frictional contact using lagrange multipliers");
  params.addRequiredParam<UserObjectName>("cohesive_zone_uo", "The cohesive zone user object.");
  params.set<bool>("interpolate_normals") = false;
  params.set<bool>("compute_lm_residual") = false;
  return params;
}

MortarGenericTraction::MortarGenericTraction(const InputParameters & parameters)
  : ADMortarLagrangeConstraint(parameters),
    _component(getParam<MooseEnum>("component")),
    _cohesize_zone_uo(const_cast<BilinearMixedModeCohesiveZoneModel &>(
        getUserObject<BilinearMixedModeCohesiveZoneModel>("cohesive_zone_uo")))
{
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
