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
}

std::vector<DenseMatrix<Real>>
MonteCarloSampler::sample()
{
  std::vector<DenseMatrix<Real>> output(1);
  output[0].resize(_num_samples, _distributions.size());
  for (std::size_t i = 0; i < _num_samples; ++i)
    for (auto j = beginIndex(_distributions); j < _distributions.size(); ++j)
      output[0](i, j) = _distributions[j]->quantile(rand());
  return output;
}
