//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementAverageFunctionDifference.h"

#include "Function.h"

registerMooseObject("MooseApp", ElementAverageFunctionDifference);

defineLegacyParams(ElementAverageFunctionDifference);

InputParameters
ElementAverageFunctionDifference::validParams()
{
  InputParameters params = ElementAverageValue::validParams();
  params.addClassDescription("Calculates the difference between a variable and a function.");
  params.addRequiredParam<FunctionName>("function", "The analytic solution to compare against");
  params.addParam<bool>("absolute_value",
                        false,
                        "Flag to take the absolute value of the diffference between the variable "
                        "and the function.");
  return params;
}

ElementAverageFunctionDifference::ElementAverageFunctionDifference(
    const InputParameters & parameters)
  : ElementAverageValue(parameters),
    _func(getFunction("function")),
    _abs(getParam<bool>("absolute_value"))
{
}

Real
ElementAverageFunctionDifference::getValue()
{
  const Real integral = ElementAverageValue::getValue();

  gatherSum(_volume);
  const Real val = integral / _volume - _func.value(_t, _q_point[_qp]);

  if (_abs)
    return std::abs(val);
  return val;
}
