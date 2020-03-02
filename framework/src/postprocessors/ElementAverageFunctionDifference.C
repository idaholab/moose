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
  params.addParam<Point>(
      "point", Point(), "A point in space to be given to the function Default: (0, 0, 0)");
  params.addParam<bool>("absolute_value",
                        true,
                        "Flag to take the absolute value of the diffference between the variable "
                        "and the function.");
  return params;
}

ElementAverageFunctionDifference::ElementAverageFunctionDifference(
    const InputParameters & parameters)
  : ElementAverageValue(parameters),
    _func(getFunction("function")),
    _point(getParam<Point>("point")),
    _abs(getParam<bool>("absolute_value"))
{
}

Real
ElementAverageFunctionDifference::getValue()
{
  if (_abs)
    return std::abs(std::abs(ElementAverageValue::getValue()) - std::abs(_func.value(_t, _point)));
  return ElementAverageValue::getValue() - _func.value(_t, _point);
}
