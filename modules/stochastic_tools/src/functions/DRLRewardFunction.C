//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DRLRewardFunction.h"
#include "FunctionInterface.h"

registerMooseObject("StochasticToolsApp", DRLRewardFunction);

InputParameters
DRLRewardFunction::validParams()
{
  InputParameters params = Function::validParams();

  params.addRequiredParam<FunctionName>("design_function", "The desired value to reach.");
  params.addRequiredParam<PostprocessorName>(
      "observed_value", "The name of the Postprocessor that contains the observed value.");
  params.addParam<Real>("c1", 10, "1st coefficient in the reward function.");
  params.addParam<Real>("c2", 1, "2nd coefficient in the reward function.");

  return params;
}

DRLRewardFunction::DRLRewardFunction(const InputParameters & parameters)
  : Function(parameters),
    FunctionInterface(this),
    _design_function(getFunction("design_function")),
    _observed_pp_name(getParam<PostprocessorName>("observed_value")),
    _observed_value(getPostprocessorValueByName(_observed_pp_name)),
    _c1(getParam<Real>("c1")),
    _c2(getParam<Real>("c2"))
{
}

Real
DRLRewardFunction::value(Real t, const Point & p) const
{
  Real design_value = _design_function.value(t, p);
  return -_c1 * std::abs(design_value - _observed_value) + _c2;
}

ADReal
DRLRewardFunction::value(const ADReal & t, const ADPoint & p) const
{
  ADReal design_value = _design_function.value(t, p);
  return -_c1 * std::abs(design_value - _observed_value) + _c2;
}
