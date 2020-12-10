//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VariableFunctionElementIntegral.h"
#include "Function.h"

registerMooseObject("isopodApp", VariableFunctionElementIntegral);

InputParameters
VariableFunctionElementIntegral::validParams()
{
  InputParameters params = ElementIntegralPostprocessor::validParams();

  params.addRequiredParam<FunctionName>("function", "Name of function to integrate");

  params.addRequiredCoupledVar("variable",
                               "The name of the variable that this boundary condition applies to");
  params.addClassDescription("Integrates a function times variable over elements");
  params.addParam<Real>(
      "scale_factor", 1, "A scale factor to be applied to the postprocessor value");
  return params;
}

VariableFunctionElementIntegral::VariableFunctionElementIntegral(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _function(getFunction("function")),
    _u(coupledValue("variable")),
    _scale_factor(getParam<Real>("scale_factor"))
{
}

Real
VariableFunctionElementIntegral::computeQpIntegral()
{
  return _function.value(_t, _q_point[_qp]) * _u[_qp] * _scale_factor;
}
