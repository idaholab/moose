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

  params.addParam<std::vector<PostprocessorName>>(
      "point",
      "A set of three PostprocessorNames or constant values (or any mixture thereof) that will be "
      "passed to the function in the space argument");

  params.addParam<Real>("scale_factor", 1, "A scale factor to be applied to the function");
  params.addParam<std::vector<PostprocessorName>>(
      "indirect_dependencies",
      "If the evaluated function depends on other postprocessors they must be listed here to "
      "ensure proper dependency resolution");

  params.addParam<PostprocessorName>("time",
                                     "The PostprocessorName or constant value that will be passed "
                                     "to the function in the time argument.");
  params.declareControllable("scale_factor");
  params.addClassDescription(
      "Computes the value of a supplied function at a single point (scalable)");
  return params;
}

FunctionValuePostprocessor::FunctionValuePostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _function(getFunction("function")),
    _scale_factor(getParam<Real>("scale_factor")),
    _has_space_pp(isParamValid("point")),
    _time_pp(nullptr)
{
  if (isParamValid("time"))
    _time_pp = &getPostprocessorValue("time");

  if (_has_space_pp)
  {
    if (coupledPostprocessors("point") != 3)
      paramError("point", "Size must be 3");
    _point.resize(3);
    for (unsigned int j = 0; j < 3; ++j)
      _point[j] = &getPostprocessorValue("point", j);
  }

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
  Point p;
  if (_has_space_pp)
    for (unsigned int j = 0; j < 3; ++j)
      p(j) = *_point[j];
  if (_time_pp)
    return _scale_factor * _function.value(*_time_pp, p);
  return _scale_factor * _function.value(_t, p);
}
