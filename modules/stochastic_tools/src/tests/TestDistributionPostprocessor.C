/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
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
  params.addParam<Real>(
      "cdf_value", 0.5, "A cdf value that used to determine the value of sampled parameter");
  params.addParam<Real>("sample_value",
                        0.5,
                        "A given sampled value that can be used to determine "
                        "the probability and cumulative probability.");

  return params;
}

TestDistributionPostprocessor::TestDistributionPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _distribution(getDistribution("distribution")),
    _cdf_value(getParam<Real>("cdf_value")),
    _sample_value(getParam<Real>("sample_value"))
{
}

void
TestDistributionPostprocessor::initialize()
{
}

void
TestDistributionPostprocessor::execute()
{
}

PostprocessorValue
TestDistributionPostprocessor::getValue()
{
  return _distribution.cdf(_sample_value) * _distribution.pdf(_sample_value) *
         _distribution.inverseCdf(_cdf_value) * _distribution.getRandomNumber();
}
