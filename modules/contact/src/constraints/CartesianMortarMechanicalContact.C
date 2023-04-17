//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CartesianMortarMechanicalContact.h"

registerMooseObject("ContactApp", CartesianMortarMechanicalContact);

InputParameters
CartesianMortarMechanicalContact::validParams()
{
  InputParameters params = ADMortarLagrangeConstraint::validParams();

  MooseEnum component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "component", component, "The force component constraint that this object is supplying");
  params.addClassDescription(
      "This class is used to apply normal contact forces using lagrange multipliers");
  params.set<bool>("compute_lm_residual") = false;
  return params;
}

CartesianMortarMechanicalContact::CartesianMortarMechanicalContact(
    const InputParameters & parameters)
  : ADMortarLagrangeConstraint(parameters), _component(getParam<MooseEnum>("component"))
{
}

ADReal
CartesianMortarMechanicalContact::computeQpResidual(Moose::MortarType type)
{
  switch (type)
  {
    case Moose::MortarType::Secondary:
      return _test_secondary[_i][_qp] * _lambda[_qp];

    case Moose::MortarType::Primary:
      return -_test_primary[_i][_qp] * _lambda[_qp];

    default:
      return 0;
  }
}
