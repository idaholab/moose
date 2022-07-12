//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionValuePostprocessor.h"
#include "Function.h"

registerMooseObject("MooseApp", FunctionValuePostprocessor);

InputParameters
FunctionValuePostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<FunctionName>("function",
                                        "The function which supplies the postprocessor value.");
  params.addParam<Point>(
      "point", Point(), "A point in space to be given to the function Default: (0, 0, 0)");
  params.addParam<Real>("scale_factor", 1, "A scale factor to be applied to the function");
  params.addParam<std::vector<PostprocessorName>>(
      "indirect_dependencies",
      "If the evaluated function depends on other postprocessors they must be listed here to "
      "ensure proper dependency resolution");

  params.declareControllable("point scale_factor");
  params.addClassDescription(
      "Computes the value of a supplied function at a single point (scalable)");
  return params;
}

FunctionValuePostprocessor::FunctionValuePostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _function(getFunction("function")),
    _point(getParam<Point>("point")),
    _scale_factor(getParam<Real>("scale_factor"))
{
  const auto & indirect_dependencies =
      getParam<std::vector<PostprocessorName>>("indirect_dependencies");
  _depend_uo.insert(indirect_dependencies.begin(), indirect_dependencies.end());
}

void
FunctionValuePostprocessor::initialize()
{
}

void
FunctionValuePostprocessor::execute()
{
}

PostprocessorValue
FunctionValuePostprocessor::getValue()
{
  return _scale_factor * _function.value(_t, _point);
}
