//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScaledAbsDifferenceDRLRewardFunction.h"

registerMooseObject("StochasticToolsApp", ScaledAbsDifferenceDRLRewardFunction);

InputParameters
ScaledAbsDifferenceDRLRewardFunction::validParams()
{
  InputParameters params = Function::validParams();

  params.addClassDescription(
      "Evaluates a scaled absolute difference reward function for a process "
      "which is controlled by a Deep Reinforcement Learning based surrogate.");

  params.addRequiredParam<FunctionName>("design_function", "The desired value to reach.");
  params.addRequiredParam<PostprocessorName>(
      "observed_value", "The name of the Postprocessor that contains the observed value.");

  params.addParam<Real>("c1", 10, "1st coefficient in the reward function.");
  params.addParam<Real>("c2", 1, "2nd coefficient in the reward function.");

  return params;
}

ScaledAbsDifferenceDRLRewardFunction::ScaledAbsDifferenceDRLRewardFunction(
    const InputParameters & parameters)
  : Function(parameters),
    FunctionInterface(this),
    _design_function(getFunction("design_function")),
    _observed_value(getPostprocessorValueByName(getParam<PostprocessorName>("observed_value"))),
    _c1(getParam<Real>("c1")),
    _c2(getParam<Real>("c2"))
{
}

Real
ScaledAbsDifferenceDRLRewardFunction::value(Real t, const Point & p) const
{
  Real design_value = _design_function.value(t, p);
  return -_c1 * std::abs(design_value - _observed_value) + _c2;
}

ADReal
ScaledAbsDifferenceDRLRewardFunction::value(const ADReal & t, const ADPoint & p) const
{
  ADReal design_value = _design_function.value(t, p);
  return -_c1 * std::abs(design_value - _observed_value) + _c2;
}
