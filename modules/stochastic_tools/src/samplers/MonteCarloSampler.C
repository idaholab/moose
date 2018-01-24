//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
