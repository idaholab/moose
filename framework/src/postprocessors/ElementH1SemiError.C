//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementH1SemiError.h"
#include "Function.h"

template <>
InputParameters
validParams<ElementH1SemiError>()
{
  InputParameters params = validParams<ElementIntegralVariablePostprocessor>();
  params.addRequiredParam<FunctionName>("function", "The analytic solution to compare against");
  return params;
}

ElementH1SemiError::ElementH1SemiError(const InputParameters & parameters)
  : ElementIntegralVariablePostprocessor(parameters), _func(getFunction("function"))
{
}

Real
ElementH1SemiError::getValue()
{
  return std::sqrt(ElementIntegralVariablePostprocessor::getValue());
}

Real
ElementH1SemiError::computeQpIntegral()
{
  RealGradient diff = _grad_u[_qp] - _func.gradient(_t, _q_point[_qp]);
  return diff * diff;
}
