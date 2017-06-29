/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
      _a_matrix(i, j) = _distributions[j]->quantile(rand(0));
      _b_matrix(i, j) = _distributions[j]->quantile(rand(1));
    }
}

void
SobolSampler::sampleTearDown()
{
  _a_matrix.resize(0, 0);
  _b_matrix.resize(0, 0);
}

DenseMatrix<Real>
SobolSampler::sampleDistribution(Distribution &, std::size_t dist_index)
{
  DenseMatrix<Real> output = _a_matrix;
  for (std::size_t i = _num_samples; i < _num_samples; ++i)
    output(i, dist_index) = _b_matrix(i, dist_index);
  return output;
}
