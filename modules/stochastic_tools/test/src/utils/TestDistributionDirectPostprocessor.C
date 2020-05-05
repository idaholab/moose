//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestDistributionDirectPostprocessor.h"
#include "Normal.h"

registerMooseObject("StochasticToolsTestApp", TestDistributionDirectPostprocessor);

InputParameters
TestDistributionDirectPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<DistributionName>(
      "distribution", "The normal distribution which supplies the postprocessor value.");
  params.addRequiredParam<Real>(
      "value", "A value to pass to the cdf, pdf, or quantile function of the given distribution.");
  params.addParam<Real>("mean", 0, "Mean of normal distribution.");
  params.addParam<Real>("standard_deviation", 1, "Standard deviation of normal distribution.");
  MooseEnum method("cdf pdf quantile");
  params.addParam<MooseEnum>("method", method, "The distribution method to call.");
  return params;
}

TestDistributionDirectPostprocessor::TestDistributionDirectPostprocessor(
    const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _distribution(getDistribution<Normal>("distribution")),
    _value(getParam<Real>("value")),
    _mean(getParam<Real>("mean")),
    _std_dev(getParam<Real>("standard_deviation")),
    _distribution_method(getParam<MooseEnum>("method"))
{
}

PostprocessorValue
TestDistributionDirectPostprocessor::getValue()
{
  if (_distribution_method == "pdf")
    return _distribution.pdf(_value, _mean, _std_dev);
  else if (_distribution_method == "cdf")
    return _distribution.cdf(_value, _mean, _std_dev);
  else if (_distribution_method == "quantile")
    return _distribution.quantile(_value, _mean, _std_dev);
  mooseError("This should be possible to reach, what did you do!");
}
