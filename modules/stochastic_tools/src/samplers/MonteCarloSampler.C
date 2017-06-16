/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "MonteCarloSampler.h"

template <>
InputParameters
validParams<MonteCarloSampler>()
{
  InputParameters params = validParams<Sampler>();
  params.addClassDescription("Monte Carlo Sampler.");
  params.addRequiredParam<unsigned int>(
      "n_samples", "Number of Monte Carlo samples to perform for each distribution.");
  return params;
}

MonteCarloSampler::MonteCarloSampler(const InputParameters & parameters)
  : Sampler(parameters), _num_samples(getParam<unsigned int>("n_samples"))
{
  checkSeedNumber(1);
}

DenseMatrix<Real>
MonteCarloSampler::sampleDistribution(Distribution & distribution)
{
  DenseMatrix<Real> output(_num_samples, 1);
  for (std::size_t i = 0; i < _num_samples; ++i)
    output(i, 0) = distribution.quantile(_generator.rand(0));
  return output;
}
