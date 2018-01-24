//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementIntegralVariablePostprocessor.h"

template <>
InputParameters
validParams<ElementIntegralVariablePostprocessor>()
{
  InputParameters params = validParams<ElementIntegralPostprocessor>();
  params.addRequiredCoupledVar("variable", "The name of the variable that this object operates on");
  return params;
}

ElementIntegralVariablePostprocessor::ElementIntegralVariablePostprocessor(
    const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    MooseVariableInterface(this, false),
    _u(coupledValue("variable")),
    _grad_u(coupledGradient("variable")),
    _u_dot(coupledDot("variable"))
{
  addMooseVariableDependency(mooseVariable());
}

Real
ElementIntegralVariablePostprocessor::computeQpIntegral()
{
  return _u[_qp];
}
