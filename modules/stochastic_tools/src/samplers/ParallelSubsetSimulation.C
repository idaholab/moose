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

registerMooseObject("StochasticToolsApp", ParallelSubsetSimulation);

InputParameters
ParallelSubsetSimulation::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Parallel Subset Simulation sampler.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions",
      "The distribution names to be sampled, the number of distributions provided defines the "
      "number of columns per matrix.");
  params.addRequiredParam<ReporterName>(
      "output_reporter", "Reporter with results of samples created by trainer.");
  params.addRequiredParam<ReporterName>(
      "inputs_reporter", "Reporter with input parameters.");
  params.addRequiredRangeCheckedParam<Real>(
      "subset_probability", "subset_probability>0 && subset_probability<=1",
      "Conditional probability of each subset");
  params.addRequiredParam<int>(
      "num_samplessub",
      "Number of samples per subset");
  params.addParam<bool>(
      "use_absolute_value", false,
      "Use absolute value of the sub app output");
  params.addParam<unsigned int>(
      "num_random_seeds",
      100000,
      "Initialize a certain number of random seeds. Change from the default only if you have to.");
  return params;
}

ParallelSubsetSimulation::ParallelSubsetSimulation(const InputParameters & parameters)
  : Sampler(parameters), ReporterInterface(this),
    _num_samplessub(getParam<int>("num_samplessub")),
    _use_absolute_value(getParam<bool>("use_absolute_value")),
    _subset_probability(getParam<Real>("subset_probability")),
    _num_random_seeds(getParam<unsigned int>("num_random_seeds")),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep())
    // _perf_compute_sample(registerTimedSection("computeSample", 4))
{
  // Filling the `distributions` vector with the user-provided distributions.
  for (const DistributionName & name : getParam<std::vector<DistributionName>>("distributions"))
    _distributions.push_back(&getDistributionByName(name));

  // Changes required
  setNumberOfRows(n_processors());

  // Setting the number of columns in the sampler matrix (equal to the number of distributions).
  setNumberOfCols(_distributions.size());

  // The acceptance ratio for the Markov Chains aids in proposing the next samples
  _acceptance_ratio = 0.0;

  /* `inputs_sto` is a member variable that aids in deciding the next set of samples
  in the Subset Simulation algorithm by storing the input parameter values*/
  _inputs_sto.resize(_distributions.size());

  /* `inputs_sorted` is a member variable which also aids in deciding the next set of samples
  in the Subset Simulation algorithm by storing the sorted input parameter values
  by their corresponding output values*/
  _inputs_sorted.resize(_distributions.size());

  /* `markov_seed` is a member variable to store the seed input values for proposing
  the next set of Markov chain samples.*/
  _markov_seed.resize(_distributions.size());

  /* `new_sample_vec` is a member variable to store the next proposed set of samples
  which will be used in the next proposal step*/
  _new_sample_vec.resize(_distributions.size());

  for (dof_id_type j = 0; j < _distributions.size(); ++j)
  {
    _markov_seed[j].resize(n_processors());
    _new_sample_vec[j].resize(n_processors());
  }

  // Member variable to track the subset index
  _subset = 0;

  // `check_step` is a member variable for ensuring that the MCMC algorithm proceeds in a sequential
  // fashion
  _check_step = 0;

  setNumberOfRandomSeeds(_num_random_seeds);
}

const int &
ParallelSubsetSimulation::getNumSamplesSub() const
{
  return _num_samplessub;
}

const bool &
ParallelSubsetSimulation::getUseAbsoluteValue() const
{
  return _use_absolute_value;
}

const Real &
ParallelSubsetSimulation::getSubsetProbability() const
{
  return _subset_probability;
}

Real
ParallelSubsetSimulation::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  const bool sample = _step > 1 && col_index == 0 && _check_step != _step;
  if (_step <= (int)(_num_samplessub / n_processors()))
  {
    // This is the first subset which uses a simple Monte Carlo sampling scheme
    _subset = std::floor((_step * n_processors()) / _num_samplessub);
    if (sample)
    {
      _seed_value = _step * n_processors();
      for (dof_id_type j = 0; j < _distributions.size(); ++j)
      {
        for (dof_id_type ss = 0; ss < n_processors(); ++ss)
          _inputs_sto[j].push_back(Normal::quantile(_distributions[j]->cdf(getReporterValue<std::vector<std::vector<Real>>>("inputs_reporter")[j][ss]),0,1));
      }
      std::vector<Real> Tmp1 = (_use_absolute_value) ? AdaptiveMonteCarloUtils::computeVectorABS(getReporterValue<std::vector<Real>>("output_reporter")) : getReporterValue<std::vector<Real>>("output_reporter");
      _communicator.allgather(Tmp1);
      for (dof_id_type ss = 0; ss < n_processors(); ++ss)
        _outputs_sto.push_back(Tmp1[ss]);
    }
    _check_step = _step;
    return _distributions[col_index]->quantile(getRand(_seed_value));
  } else
  {
    // This are the subsequent subsets which use Markov Chain Monte Carlo sampling scheme
    _subset = std::floor(((_step-1) * n_processors()) / _num_samplessub);
    if (sample)
    {
      _seed_value = _step * n_processors() + 1;
      for (dof_id_type j = 0; j < _distributions.size(); ++j)
      {
        for (dof_id_type ss = 0; ss < n_processors(); ++ss)
          _inputs_sto[j].push_back(Normal::quantile(_distributions[j]->cdf(getReporterValue<std::vector<std::vector<Real>>>("inputs_reporter")[j][ss]),0,1));
      }
      std::vector<Real> Tmp1 = (_use_absolute_value) ? AdaptiveMonteCarloUtils::computeVectorABS(getReporterValue<std::vector<Real>>("output_reporter")) : getReporterValue<std::vector<Real>>("output_reporter");
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
    _check_step = _step;
    return _distributions[col_index]->quantile(Normal::cdf(_new_sample_vec[col_index][row_index],0,1));
  }
}
