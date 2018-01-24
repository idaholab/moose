//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementIntegralVariableUserObject.h"

template <>
InputParameters
validParams<ElementIntegralVariableUserObject>()
{
  InputParameters params = validParams<ElementIntegralUserObject>();
  params.addRequiredCoupledVar("variable", "The name of the variable that this object operates on");
  return params;
}

ElementIntegralVariableUserObject::ElementIntegralVariableUserObject(
    const InputParameters & parameters)
  : ElementIntegralUserObject(parameters),
    MooseVariableInterface(this, false),
    _u(coupledValue("variable")),
    _grad_u(coupledGradient("variable"))
{
  addMooseVariableDependency(mooseVariable());
}

Real
ElementIntegralVariableUserObject::computeQpIntegral()
{
  return _u[_qp];
}
