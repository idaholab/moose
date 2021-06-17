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

InputParameters
SobolSampler::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Sobol variance-based sensitivity analysis Sampler.");
  params.addParam<bool>("resample", true, "Create the re-sample matrix for second-order indices.");
  params.addRequiredParam<SamplerName>("sampler_a", "The 'sample' matrix.");
  params.addRequiredParam<SamplerName>("sampler_b", "The 're-sample' matrix.");
  params.suppressParameter<ExecFlagEnum>("execute_on");
  return params;
}

SobolSampler::SobolSampler(const InputParameters & parameters)
  : Sampler(parameters),
    _m1_matrix(0, 0),
    _m2_matrix(0, 0),
    _sampler_a(getSampler("sampler_a")),
    _sampler_b(getSampler("sampler_b")),
    _resample(getParam<bool>("resample")),
    _perf_sample_setup(registerTimedSection("sampleSetup", 3)),
    _perf_sample_teardown(registerTimedSection("computeTearDown", 3)),
    _perf_compute_sample(registerTimedSection("computeSample", 4))
{
  if (_sampler_a.getNumberOfCols() != _sampler_b.getNumberOfCols())
    paramError("sampler_a", "The supplied Sampler objects must have the same number of columns.");

  if (_sampler_a.getNumberOfRows() != _sampler_b.getNumberOfRows())
    paramError("sampler_a", "The supplied Sampler objects must have the same number of rows.");

  // Compute the number of matrices
  const dof_id_type num_cols = _sampler_a.getNumberOfCols();
  _num_matrices = _resample ? 2 * num_cols + 2 : num_cols + 2;
  _num_rows_per_matrix = _sampler_a.getNumberOfRows();

  // Initialize this object
  setNumberOfCols(_sampler_a.getNumberOfCols());
  setNumberOfRows(_num_rows_per_matrix * _num_matrices);
}

void
SobolSampler::sampleSetUp(const Sampler::SampleMode)
{
  TIME_SECTION(_perf_sample_setup);

  // These must call getGlobalSamples because the matrix partition between the supplied objects
  // and this object differ.
  _m1_matrix = _sampler_a.getGlobalSamples();
  _m2_matrix = _sampler_b.getGlobalSamples();

  if (_m1_matrix == _m2_matrix)
    mooseError("The supplied sampler matrices must not be the same.");
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
  else if (matrix_index > getNumberOfCols())
  {
    if (col_index == (matrix_index - getNumberOfCols() - 1))
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
SobolSampler::sampleTearDown(const Sampler::SampleMode)
{
  TIME_SECTION(_perf_sample_teardown);

  _m1_matrix.resize(0, 0);
  _m2_matrix.resize(0, 0);
}
