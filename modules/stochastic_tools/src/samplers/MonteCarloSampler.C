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
  params.addParam<dof_id_type>(
      "n_matrices", 1, "Number of matrices to create, each matrix will contain 'n_samples'.");
  return params;
}

MonteCarloSampler::MonteCarloSampler(const InputParameters & parameters)
  : Sampler(parameters), _num_matrices(getParam<dof_id_type>("n_matrices"))
{
  setNumberOfMatrices(_num_matrices);
  if (_is_distributed_sample_data && n_processors() > _num_samples * _num_matrices)
    mooseError("In MonteCarloSampler, the number of MPI processes cannot be larger than the number "
               "of samples x matrices(",
               _num_samples * _num_matrices,
               ") when sample data is distributed across all processors.");
}

std::vector<DenseMatrix<Real>> &
MonteCarloSampler::sample()
{
  if (_compute_sample_once && _sample_data.size() != 0)
    return _sample_data;

  std::vector<DenseMatrix<Real>> output(_num_matrices);

  if (_is_distributed_sample_data)
  {
    dof_id_type total_rows, local_rows, local_row_begin, local_row_end;
    total_rows = 0;
    for (unsigned int i = 0; i < _num_matrices; i++)
      total_rows += _num_samples;

    MooseUtils::linearPartitionItems(
        total_rows, n_processors(), processor_id(), local_rows, local_row_begin, local_row_end);

    std::vector<unsigned int> matrix_local_rows(_num_matrices, 0);
    for (MooseIndex(total_rows) m = 0; m < total_rows; ++m)
    {
      Sampler::Location loc = getLocation(m);
      if (m >= local_row_begin && m < local_row_end)
        matrix_local_rows[loc.sample()]++;
    }

    for (MooseIndex(_num_matrices) m = 0; m < _num_matrices; ++m)
      output[m].resize(matrix_local_rows[m], _distributions.size());

    std::vector<unsigned int> local_i(_num_matrices, 0);
    for (MooseIndex(total_rows) m = 0; m < total_rows; ++m)
    {
      Sampler::Location loc = getLocation(m);

      for (MooseIndex(_distributions) j = 0; j < _distributions.size(); ++j)
      {
        Real quant = _distributions[j]->quantile(rand());
        if (m >= local_row_begin && m < local_row_end)
          output[loc.sample()](local_i[loc.sample()], j) = quant;
      }

      if (m >= local_row_begin && m < local_row_end)
        local_i[loc.sample()]++;
    }
  }
  else
  {
    for (MooseIndex(_num_matrices) m = 0; m < _num_matrices; ++m)
    {
      output[m].resize(_num_samples, _distributions.size());
      for (MooseIndex(_num_samples) i = 0; i < _num_samples; ++i)
        for (MooseIndex(_distributions) j = 0; j < _distributions.size(); ++j)
          output[m](i, j) = _distributions[j]->quantile(rand());
    }
  }

  _sample_data = output;

  return _sample_data;
}
