//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementL2Error.h"
#include "Function.h"

registerMooseObject("MooseApp", ElementL2Error);

InputParameters
ElementL2Error::validParams()
{
  InputParameters params = ElementIntegralVariablePostprocessor::validParams();
  params.addRequiredParam<FunctionName>("function", "The analytic solution to compare against");
  params.addClassDescription(
      "Computes L2 error between a field variable and an analytical function");
  return params;
}

ElementL2Error::ElementL2Error(const InputParameters & parameters)
  : ElementIntegralVariablePostprocessor(parameters), _func(getFunction("function"))
{
}

Real
ElementL2Error::getValue()
{
  return std::sqrt(ElementIntegralPostprocessor::getValue());
}

Real
ElementL2Error::computeQpIntegral()
{
  Real diff = _u[_qp] - _func.value(_t, _q_point[_qp]);
  return diff * diff;
}
