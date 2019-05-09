//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MonteCarloSampler.h"

registerMooseObject("StochasticToolsApp", MonteCarloSampler);

template <>
InputParameters
validParams<MonteCarloSampler>()
{
  InputParameters params = validParams<Sampler>();
  params.addClassDescription("Monte Carlo Sampler.");
  params.addRequiredParam<dof_id_type>(
      "n_samples",
      "Number of Monte Carlo samples to perform for each distribution within each matrix.");
  params.addParam<dof_id_type>(
      "n_matrices", 1, "Number of matrices to create, each matrix will contain 'n_samples'.");
  return params;
}

MonteCarloSampler::MonteCarloSampler(const InputParameters & parameters)
  : Sampler(parameters),
    _num_matrices(getParam<dof_id_type>("n_matrices")),
    _num_samples(getParam<dof_id_type>("n_samples"))
{
}

std::vector<DenseMatrix<Real>>
MonteCarloSampler::sample()
{
  std::vector<DenseMatrix<Real>> output(_num_matrices);
  for (MooseIndex(_num_matrices) m = 0; m < _num_matrices; ++m)
  {
    output[m].resize(_num_samples, _distributions.size());
    for (MooseIndex(_num_samples) i = 0; i < _num_samples; ++i)
      for (MooseIndex(_distributions) j = 0; j < _distributions.size(); ++j)
        output[m](i, j) = _distributions[j]->quantile(rand());
  }
  return output;
}
