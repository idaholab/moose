/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TestDistributionPostprocessor.h"
#include "Distribution.h"

template <>
InputParameters
validParams<TestDistributionPostprocessor>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<DistributionName>(
      "distribution", "The distribution which supplies the postprocessor value.");
  params.addRequiredParam<Real>(
      "value", "A value to pass to the cdf, pdf, or quantile function of the given distribution.");

  MooseEnum method("cdf pdf quantile");
  params.addParam<MooseEnum>("method", method, "The distribution method to call.");
  return params;
}

TestDistributionPostprocessor::TestDistributionPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _distribution(getDistribution("distribution")),
    _value(getParam<Real>("value")),
    _distribution_method(getParam<MooseEnum>("method"))
{
}

PostprocessorValue
TestDistributionPostprocessor::getValue()
{
  if (_distribution_method == "pdf")
    return _distribution.pdf(_value);
  else if (_distribution_method == "cdf")
    return _distribution.cdf(_value);
  else if (_distribution_method == "quantile")
    return _distribution.quantile(_value);
  mooseError("This should be possible to reach, what did you do!");
}
