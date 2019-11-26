//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SobolSampler.h"
#include "Distribution.h"

registerMooseObject("StochasticToolsApp", SobolSampler);

template <>
InputParameters
validParams<SobolSampler>()
{
  InputParameters params = validParams<Sampler>();
  params.addClassDescription("Sobol variance-based sensitivity analysis Sampler.");
  params.addRequiredParam<dof_id_type>("num_rows", "The number of rows per matrix to generate.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions",
      "The distribution names to be sampled, the number of distributions provided defines the "
      "number of columns per matrix.");
  return params;
}

SobolSampler::SobolSampler(const InputParameters & parameters)
  : Sampler(parameters),
    _a_matrix(0, 0),
    _b_matrix(0, 0),
    _distribution_names(getParam<std::vector<DistributionName>>("distributions")),
    _num_rows_per_matrix(getParam<dof_id_type>("num_rows"))
{
  setNumberOfRandomSeeds(2);

  for (const DistributionName & name : _distribution_names)
    _distributions.push_back(&getDistributionByName(name));

  setNumberOfCols(_distribution_names.size());
  setNumberOfRows(_num_rows_per_matrix * (_distribution_names.size() + 2));
}

void
SobolSampler::sampleSetUp()
{
  _a_matrix.resize(_num_rows_per_matrix, getNumberOfCols());
  _b_matrix.resize(_num_rows_per_matrix, getNumberOfCols());
  for (dof_id_type i = 0; i < _num_rows_per_matrix; ++i)
    for (dof_id_type j = 0; j < getNumberOfCols(); ++j)
    {
      _a_matrix(i, j) = _distributions[j]->quantile(this->getRand(0));
      _b_matrix(i, j) = _distributions[j]->quantile(this->getRand(1));
    }
}

Real
SobolSampler::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  dof_id_type matrix_index = row_index / _num_rows_per_matrix;
  dof_id_type r = row_index - matrix_index * _num_rows_per_matrix;

  if (matrix_index == 0)
    return _a_matrix(r, col_index);
  else if (matrix_index == 1)
    return _b_matrix(r, col_index);
  else if (col_index == matrix_index - 2)
    return _b_matrix(r, col_index);
  else
    return _a_matrix(r, col_index);
}

void
SobolSampler::sampleTearDown()
{
  _a_matrix.resize(0, 0);
  _b_matrix.resize(0, 0);
}
