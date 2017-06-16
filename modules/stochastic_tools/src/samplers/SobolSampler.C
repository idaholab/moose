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
  params.addParam<std::vector<unsigned int>>("seeds",
                                             std::vector<unsigned int>({19800624, 19801009}),
                                             "Seeds for the Sobol Monte Carlo matrices , there "
                                             "should be two seeds one for each matrix A and B.");

  return params;
}

SobolSampler::SobolSampler(const InputParameters & parameters)
  : Sampler(parameters),
    _num_samples(getParam<unsigned int>("n_samples")),
    _a_matrix(0, 0),
    _b_matrix(0, 0)
{
  checkSeedNumber(2);
}

void
SobolSampler::sampleSetUp()
{
  _a_matrix.resize(_num_samples, _distributions.size());
  _b_matrix.resize(_num_samples, _distributions.size());
  for (unsigned int i = 0; i < _num_samples; ++i)
    for (unsigned int j = 0; j < _distributions.size(); ++j)
    {
      _a_matrix(i, j) = _distributions[j]->quantile(_generator.rand(0));
      _b_matrix(i, j) = _distributions[j]->quantile(_generator.rand(1));
    }
}

void
SobolSampler::sampleTearDown()
{
  _a_matrix.resize(0, 0);
  _b_matrix.resize(0, 0);
}

DenseMatrix<Real>
SobolSampler::sampleDistribution(Distribution &, unsigned int dist_index)
{
  DenseMatrix<Real> output = _a_matrix;
  for (unsigned int i = 0; i < _num_samples; ++i)
    output(i, dist_index) = _b_matrix(i, dist_index);
  return output;
}
