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

registerMooseObjectAliased("StochasticToolsApp", SobolSampler, "Sobol");
registerMooseObjectReplaced("StochasticToolsApp", SobolSampler, "07/01/2020 00:00", Sobol);

defineLegacyParams(SobolSampler);

InputParameters
SobolSampler::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Sobol variance-based sensitivity analysis Sampler.");
  params.addRequiredParam<dof_id_type>("num_rows", "The number of rows per matrix to generate.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions",
      "The distribution names to be sampled, the number of distributions provided defines the "
      "number of columns per matrix.");
  params.addParam<bool>("resample", true, "Create the re-sample matrix for second-order indices.");
  return params;
}

SobolSampler::SobolSampler(const InputParameters & parameters)
  : Sampler(parameters),
    _m1_matrix(0, 0),
    _m2_matrix(0, 0),
    _distribution_names(getParam<std::vector<DistributionName>>("distributions")),
    _resample(getParam<bool>("resample")),
    _num_inputs(_distribution_names.size()),
    _num_matrices(_resample ? 2 * _distribution_names.size() + 2 : _distribution_names.size() + 2),
    _num_rows_per_matrix(getParam<dof_id_type>("num_rows")),
    _perf_sample_setup(registerTimedSection("sampleSetup", 3)),
    _perf_sample_teardown(registerTimedSection("computeTearDown", 3)),
    _perf_compute_sample(registerTimedSection("computeSample", 4))
{
  setNumberOfRandomSeeds(2);

  for (const DistributionName & name : _distribution_names)
    _distributions.push_back(&getDistributionByName(name));

  setNumberOfCols(_distribution_names.size());
  setNumberOfRows(_num_rows_per_matrix * _num_matrices);
}

void
SobolSampler::sampleSetUp()
{
  TIME_SECTION(_perf_sample_setup);

  _m1_matrix.resize(_num_rows_per_matrix, getNumberOfCols());
  _m2_matrix.resize(_num_rows_per_matrix, getNumberOfCols());
  for (dof_id_type i = 0; i < _num_rows_per_matrix; ++i)
    for (dof_id_type j = 0; j < getNumberOfCols(); ++j)
    {
      _m1_matrix(i, j) = _distributions[j]->quantile(this->getRand(0));
      _m2_matrix(i, j) = _distributions[j]->quantile(this->getRand(1));
    }
}

Real
SobolSampler::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  TIME_SECTION(_perf_compute_sample);

  dof_id_type matrix_index = row_index / _num_rows_per_matrix;
  dof_id_type r = row_index - matrix_index * _num_rows_per_matrix;

  // M2 Matrix
  if (matrix_index == 0)
    return _m2_matrix(r, col_index);

  // M1 Matrix
  else if (matrix_index == _num_matrices - 1)
    return _m1_matrix(r, col_index);

  // N_-i Matrices
  else if (matrix_index > _num_inputs)
  {
    if (col_index == (matrix_index - _num_inputs - 1))
      return _m2_matrix(r, col_index);
    else
      return _m1_matrix(r, col_index);
  }

  // N_i Matrices
  else // if (matrix_index < _num_inputs + 1)
  {
    if (col_index == matrix_index - 1)
      return _m1_matrix(r, col_index);
    else
      return _m2_matrix(r, col_index);
  }

  mooseError("Invalid row and column index, if you are seeing this Andrew messed up because this "
             "should be impossible to reach.");
}

void
SobolSampler::sampleTearDown()
{
  TIME_SECTION(_perf_sample_teardown);

  _m1_matrix.resize(0, 0);
  _m2_matrix.resize(0, 0);
}
