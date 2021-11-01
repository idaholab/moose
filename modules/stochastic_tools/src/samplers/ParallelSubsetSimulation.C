//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParallelSubsetSimulation.h"
#include "AdaptiveMonteCarloUtils.h"
#include "Normal.h"
#include "Uniform.h"

registerMooseObjectAliased("StochasticToolsApp", ParallelSubsetSimulation, "ParallelSubsetSimulation");

InputParameters
ParallelSubsetSimulation::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Subset Simulation Sampler.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions",
      "The distribution names to be sampled, the number of distributions provided defines the "
      "number of columns per matrix.");
  params.addRequiredParam<ReporterName>(
      "output_reporter", "Reporter with results of samples created by trainer.");
  params.addRequiredParam<ReporterName>(
      "inputs_reporter", "Reporter with input parameters.");
  params.addRequiredParam<Real>(
      "subset_probability",
      "Conditional probability of each subset");
  params.addRequiredParam<int>(
      "num_samplessub",
      "Number of samples per subset");
  params.addParam<bool>(
      "use_absolute_value", false,
      "Use absolute value of the sub app output");
  params.addRequiredParam<ReporterName>(
      "data_reporter", "Reporter with input parameters.");
  return params;
}

ParallelSubsetSimulation::ParallelSubsetSimulation(const InputParameters & parameters)
  : Sampler(parameters), ReporterInterface(this),
    _distribution_names(getParam<std::vector<DistributionName>>("distributions")),
    _num_samplessub(getParam<int>("num_samplessub")),
    _use_absolute_value(getParam<bool>("use_absolute_value")),
    _subset_probability(getParam<Real>("subset_probability")),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep()),
    _perf_compute_sample(registerTimedSection("computeSample", 4))
{
  for (const DistributionName & name : _distribution_names)
    _distributions.push_back(&getDistributionByName(name));

  setNumberOfRows(n_processors());
  setNumberOfCols(_distributions.size());
  _acceptance_ratio = 0.0;
  _inputs_sto.resize(_distributions.size());
  _inputs_sorted.resize(_distributions.size());
  _markov_seed.resize(_distributions.size());
  _new_sample_vec.resize(_distributions.size());
  for (dof_id_type j = 0; j < _distributions.size(); ++j)
  {
    _markov_seed[j].resize(n_processors());
    _new_sample_vec[j].resize(n_processors());
  }
  _subset = 0;
  _check_even = 0;
  setNumberOfRandomSeeds(100000);
  _proposal_std.resize(_distributions.size());
  _seed_value = n_processors();
}

Real
ParallelSubsetSimulation::computeSample(dof_id_type row_index, dof_id_type col_index)
{

  if (_step <= (_num_samplessub / n_processors()))
  {
    _subset = std::floor((_step * n_processors()) / _num_samplessub);
    if (_step > 1 && col_index == 0 && _check_even != _step)
    {
      _seed_value = _step * n_processors();
      for (dof_id_type j = 0; j < _distributions.size(); ++j)
      {
        for (dof_id_type ss = 0; ss < n_processors(); ++ss)
          _inputs_sto[j].push_back(Normal::quantile(_distributions[j]->cdf(getReporterValue<std::vector<std::vector<Real>>>("inputs_reporter")[j][ss]),0,1));
      }
      std::vector<Real> Tmp1 = (_use_absolute_value) ? AdaptiveMonteCarloUtils::computeVectorABS(getReporterValue<std::vector<Real>>("data_reporter")) : getReporterValue<std::vector<Real>>("data_reporter");
      _communicator.allgather(Tmp1);
      for (dof_id_type ss = 0; ss < n_processors(); ++ss)
        _outputs_sto.push_back(Tmp1[ss]);
    }
    _check_even = _step;
    return _distributions[col_index]->quantile(getRand(_seed_value));
  } else
  {
    _subset = std::floor(((_step-1) * n_processors()) / _num_samplessub);
    if (col_index == 0 && _check_even != _step)
    {
      _seed_value = _step * n_processors() + 1;
      for (dof_id_type j = 0; j < _distributions.size(); ++j)
      {
        for (dof_id_type ss = 0; ss < n_processors(); ++ss)
          _inputs_sto[j].push_back(Normal::quantile(_distributions[j]->cdf(getReporterValue<std::vector<std::vector<Real>>>("inputs_reporter")[j][ss]),0,1));
      }
      std::vector<Real> Tmp1 = (_use_absolute_value) ? AdaptiveMonteCarloUtils::computeVectorABS(getReporterValue<std::vector<Real>>("data_reporter")) : getReporterValue<std::vector<Real>>("data_reporter");
      _communicator.allgather(Tmp1);
      for (dof_id_type ss = 0; ss < n_processors(); ++ss)
        _outputs_sto.push_back(Tmp1[ss]);
      _count_max = std::floor(1 / _subset_probability);
      if (_subset > (std::floor(((_step-2) * n_processors()) / _num_samplessub)))
      {
        _ind_sto = -1;
        _count = INT_MAX;
        for (dof_id_type j = 0; j < _distributions.size(); ++j)
        {
          _inputs_sorted[j].resize(std::floor(_num_samplessub * _subset_probability));
          _inputs_sorted[j] = AdaptiveMonteCarloUtils::sortINPUT(_inputs_sto[j], _outputs_sto, _num_samplessub, _subset, _subset_probability);
        }
      }
      if (_count >= _count_max)
      {
        for (dof_id_type jj = 0; jj < n_processors(); ++jj)
        {
          ++_ind_sto;
          for (dof_id_type k = 0; k < _distributions.size(); ++k)
            _markov_seed[k][jj] = _inputs_sorted[k][_ind_sto];
        }
        _count = 0;
      } else
      {
        for (dof_id_type jj = 0; jj < n_processors(); ++jj)
        {
          for (dof_id_type k = 0; k < _distributions.size(); ++k)
            _markov_seed[k][jj] = _inputs_sto[k][_inputs_sto[k].size()-n_processors()+jj];
        }
      }
      ++_count;
      Real rv;
      for (dof_id_type jj = 0; jj < n_processors(); ++jj)
      {
        for (dof_id_type i = 0; i < _distributions.size(); ++i)
        {
          rv = Normal::quantile(getRand(_seed_value-1), _markov_seed[i][jj], 1.0);
          _acceptance_ratio = std::log(Normal::pdf(rv, 0, 1)) - std::log(Normal::pdf(_markov_seed[i][jj], 0, 1));

          if (_acceptance_ratio > std::log(getRand(_seed_value)))
            _new_sample_vec[i][jj] = rv;
          else
            _new_sample_vec[i][jj] = _markov_seed[i][jj];
        }
      }
    }
    _check_even = _step;
    return _distributions[col_index]->quantile(Normal::cdf(_new_sample_vec[col_index][row_index],0,1));
  }
}
