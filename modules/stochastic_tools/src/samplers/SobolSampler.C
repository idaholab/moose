//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SobolSampler.h"

template <>
InputParameters
validParams<SobolSampler>()
{
  InputParameters params = validParams<Sampler>();
  params.addClassDescription("Sobol variance-based sensitivity analysis Sampler.");
  params.addRequiredParam<unsigned int>(
      "n_samples", "Number of Monte Carlo samples to perform for each distribution.");
  return params;
}

SobolSampler::SobolSampler(const InputParameters & parameters)
  : Sampler(parameters),
    _num_samples(getParam<unsigned int>("n_samples")),
    _a_matrix(0, 0),
    _b_matrix(0, 0)
{
  setNumberOfRequiedRandomSeeds(2);
}

void
SobolSampler::sampleSetUp()
{
  _a_matrix.resize(_num_samples, _distributions.size());
  _b_matrix.resize(_num_samples, _distributions.size());
  for (std::size_t i = 0; i < _num_samples; ++i)
    for (auto j = beginIndex(_distributions); j < _distributions.size(); ++j)
    {
      _a_matrix(i, j) = _distributions[j]->quantile(this->rand(0));
      _b_matrix(i, j) = _distributions[j]->quantile(this->rand(1));
    }
}

void
SobolSampler::sampleTearDown()
{
  _a_matrix.resize(0, 0);
  _b_matrix.resize(0, 0);
}

std::vector<DenseMatrix<Real>>
SobolSampler::sample()
{
  // Create the output vector
  auto n = _distributions.size() + 2;
  std::vector<DenseMatrix<Real>> output(n);

  // Include the A and B matrices
  output[0] = _a_matrix;
  output[1] = _b_matrix;

  // Create the AB matrices
  for (auto idx = beginIndex(_distributions, 2); idx < n; ++idx)
  {
    output[idx] = _a_matrix;
    for (std::size_t i = 0; i < _num_samples; ++i)
      output[idx](i, idx - 2) = _b_matrix(i, idx - 2);
  }

  return output;
}
