//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementL1Error.h"
#include "Function.h"

registerMooseObject("MooseApp", ElementL1Error);

InputParameters
ElementL1Error::validParams()
{
  InputParameters params = ElementIntegralVariablePostprocessor::validParams();
  params.addClassDescription(
      "Computes L1 error between an elemental field variable and an analytical function.");
  params.addRequiredParam<FunctionName>("function", "The analytic solution to compare against");
  return params;
}

ElementL1Error::ElementL1Error(const InputParameters & parameters)
  : ElementIntegralVariablePostprocessor(parameters), _func(getFunction("function"))
{
}

Real
ElementL1Error::computeQpIntegral()
{
  return std::abs(_u[_qp] - _func.value(_t, _q_point[_qp]));
}
