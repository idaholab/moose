//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TangentialMortarMechanicalContact.h"

registerMooseObject("MooseApp", TangentialMortarMechanicalContact);

InputParameters
TangentialMortarMechanicalContact::validParams()
{
  InputParameters params = ADMortarConstraint::validParams();

  MooseEnum component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "component", component, "The force component constraint that this object is supplying");
  params.addClassDescription(
      "Used to apply tangential stresses from frictional contact using lagrange multipliers");
  params.set<bool>("compute_lm_residual") = false;
  return params;
}

TangentialMortarMechanicalContact::TangentialMortarMechanicalContact(
    const InputParameters & parameters)
  : ADMortarConstraint(parameters), _component(getParam<MooseEnum>("component"))
{
}

ADReal
TangentialMortarMechanicalContact::computeQpResidual(Moose::MortarType type)
{
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
      return _test_secondary[_i][_qp] * _lambda[_qp] * _tangents[_qp][0](_component) /
             _tangents[_qp][0].norm();

    case Moose::MortarType::Primary:
      // Equal and opposite reactions so we put a negative sign here
      return -_test_primary[_i][_qp] * _lambda[_qp] * _tangents[_qp][0](_component) /
             _tangents[_qp][0].norm();

    default:
      return 0;
  }
}
