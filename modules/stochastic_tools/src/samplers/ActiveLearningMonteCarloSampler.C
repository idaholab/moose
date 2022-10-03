//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ActiveLearningMonteCarloSampler.h"
#include "Distribution.h"

registerMooseObject("StochasticToolsApp", ActiveLearningMonteCarloSampler);

InputParameters
ActiveLearningMonteCarloSampler::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Monte Carlo Sampler.");
  params.addRequiredParam<dof_id_type>("num_rows", "The number of rows per matrix to generate.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions",
      "The distribution names to be sampled, the number of distributions provided defines the "
      "number of columns per matrix.");
  params.addRequiredParam<ReporterName>("flag_sample", "Flag samples if the surrogate prediction was inadequate.");
  params.addParam<unsigned int>(
      "num_random_seeds",
      100000,
      "Initialize a certain number of random seeds. Change from the default only if you have to.");
  return params;
}

ActiveLearningMonteCarloSampler::ActiveLearningMonteCarloSampler(const InputParameters & parameters)
  : Sampler(parameters),
    ReporterInterface(this),
    _distribution_names(getParam<std::vector<DistributionName>>("distributions")),
    _flag_sample(getReporterValue<std::vector<bool>>("flag_sample")),
    _num_random_seeds(getParam<unsigned int>("num_random_seeds")),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep())
{
  for (const DistributionName & name : _distribution_names)
    _distributions.push_back(&getDistributionByName(name));
  setNumberOfRows(getParam<dof_id_type>("num_rows"));
  setNumberOfCols(_distributions.size());
  _inputs_sto.resize(getParam<dof_id_type>("num_rows"));
  _inputs_gp_fails.resize(getParam<dof_id_type>("num_rows"));
  for (unsigned int i = 0; i < _inputs_sto.size(); ++i)
  {
    _inputs_sto[i].resize(_distributions.size());
    _inputs_gp_fails[i].resize(_distributions.size());
  }
  _check_step = 0;
  setNumberOfRandomSeeds(_num_random_seeds);
  _track_gp_fails = 0;
  _allowed_gp_fails = getParam<dof_id_type>("num_rows");
}

Real
ActiveLearningMonteCarloSampler::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  if (col_index == 0 && _step > 0 && _check_step != _step)
    {
      for (dof_id_type i = 0; i < getParam<dof_id_type>("num_rows"); ++i)
      {
        if (_flag_sample[i] == true)
        {
          _inputs_gp_fails[_track_gp_fails] = _inputs_sto[_track_gp_fails];
          ++_track_gp_fails;
        }
        for (dof_id_type j = 0; j < _distributions.size(); ++j)
            _inputs_sto[i][j] = _distributions[j]->quantile(getRand(_step));
      }
    } else if (_step == 0)
    {
      for (dof_id_type j = 0; j < _distributions.size(); ++j)
        _inputs_sto[row_index][j] = _distributions[j]->quantile(getRand(_step));
    }
  _check_step = _step;
  if (_track_gp_fails >= _allowed_gp_fails)
  {
    _track_gp_fails = 0;
    return _inputs_gp_fails[row_index][col_index];
  }
  else
    return _inputs_sto[row_index][col_index];
}