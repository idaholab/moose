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
  return params;
}

SobolSampler::SobolSampler(const InputParameters & parameters)
  : Sampler(parameters),
    _sampler_a(getSampler("sampler_a")),
    _sampler_b(getSampler("sampler_b")),
    _resample(getParam<bool>("resample")),
    _num_matrices(_resample ? 2 * _sampler_a.getNumberOfCols() + 2
                            : _sampler_a.getNumberOfCols() + 2)
{
  if (_sampler_a.getNumberOfCols() != _sampler_b.getNumberOfCols())
    paramError("sampler_a", "The supplied Sampler objects must have the same number of columns.");

  if (_sampler_a.getNumberOfRows() != _sampler_b.getNumberOfRows())
    paramError("sampler_a", "The supplied Sampler objects must have the same number of rows.");

  if (_sampler_a.name() == _sampler_b.name())
    paramError("sampler_a", "The supplied sampler matrices must not be the same.");

  // Initialize this object
  setNumberOfCols(_sampler_a.getNumberOfCols());
  setNumberOfRows(_sampler_a.getNumberOfRows() * _num_matrices);
}

Real
SobolSampler::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  const dof_id_type matrix_index = row_index % _num_matrices;

  if (matrix_index == 0 && col_index == 0)
  {
    _row_a = _sampler_a.getNextLocalRow();
    _row_b = _sampler_b.getNextLocalRow();
    if (std::equal(_row_a.begin(), _row_a.end(), _row_b.begin()))
      paramError("sampler_a", "The supplied sampler matrices must not be the same.");
  }

  // M2 Matrix
  if (matrix_index == 0)
    return _row_b[col_index];

  // M1 Matrix
  else if (matrix_index == _num_matrices - 1)
    return _row_a[col_index];

  // N_-i Matrices
  else if (matrix_index > getNumberOfCols())
  {
    if (col_index == (matrix_index - getNumberOfCols() - 1))
      return _row_b[col_index];
    else
      return _row_a[col_index];
  }

  // N_i Matrices
  else if (matrix_index <= getNumberOfCols())
  {
    if (col_index == matrix_index - 1)
      return _row_a[col_index];
    else
      return _row_b[col_index];
  }

  mooseError("Invalid row and column index, if you are seeing this Zach messed up because this "
             "should be impossible to reach.");
  return 0;
}

LocalRankConfig
SobolSampler::constructRankConfig(bool batch_mode) const
{
  std::vector<LocalRankConfig> all_rc(processor_id() + 1);
  for (processor_id_type r = 0; r <= processor_id(); ++r)
    all_rc[r] = rankConfig(r,
                           n_processors(),
                           _sampler_a.getNumberOfRows(),
                           _min_procs_per_row,
                           _max_procs_per_row,
                           batch_mode);
  LocalRankConfig & rc = all_rc.back();

  rc.num_local_sims *= _num_matrices;
  bool found_first = false;
  for (auto it = all_rc.rbegin(); it != all_rc.rend(); ++it)
    if (it->is_first_local_rank)
    {
      if (found_first)
        rc.first_local_sim_index += it->num_local_sims * (_num_matrices - 1);
      else
        found_first = true;
    }

  if (!batch_mode)
  {
    rc.num_local_apps = rc.num_local_sims;
    rc.first_local_app_index = rc.first_local_sim_index;
  }

  return rc;
}
