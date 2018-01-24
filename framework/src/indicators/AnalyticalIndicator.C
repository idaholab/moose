//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AnalyticalIndicator.h"
#include "Function.h"

template <>
InputParameters
validParams<AnalyticalIndicator>()
{
  InputParameters params = validParams<ElementIntegralIndicator>();
  params.addRequiredParam<FunctionName>("function", "The analytic solution to compare against");
  return params;
}

AnalyticalIndicator::AnalyticalIndicator(const InputParameters & parameters)
  : ElementIntegralIndicator(parameters), _func(getFunction("function"))
{
}

Real
AnalyticalIndicator::computeQpIntegral()
{
  Real diff = _u[_qp] - _func.value(_t, _q_point[_qp]);
  return diff * diff;
}
