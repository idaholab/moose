//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdaptiveImportanceSampler.h"
#include "AdaptiveMonteCarloUtils.h"
#include "Distribution.h"
#include "Normal.h"
#include "Uniform.h"

registerMooseObjectAliased("StochasticToolsApp", AdaptiveImportanceSampler, "AdaptiveImportance");

InputParameters
AdaptiveImportanceSampler::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Adaptive Importance Sampler.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions",
      "The distribution names to be sampled, the number of distributions provided defines the "
      "number of columns per matrix.");
  params.addRequiredParam<ReporterName>("inputs_reporter", "Reporter with input parameters.");
  params.addRequiredParam<std::vector<Real>>("proposal_std",
                                             "Standard deviations of the proposal distributions");
  params.addRequiredParam<Real>("output_limit", "Limiting values of the VPPs");
  params.addRequiredParam<std::vector<Real>>(
      "initial_values", "Initial input values to get the importance sampler started");
  params.addRequiredRangeCheckedParam<int>(
      "num_samples_train",
      "num_samples_train>0",
      "Number of samples to learn the importance distribution");
  params.addRequiredRangeCheckedParam<int>(
      "num_importance_sampling_steps",
      "num_importance_sampling_steps>0",
      "Number of importance sampling steps (after the importance distribution has been trained)");
  params.addRequiredParam<Real>(
      "std_factor", "Factor to be multiplied to the standard deviation of the importance samples");
  params.addParam<bool>("use_absolute_value", false, "Use absolute value of the sub app output");
  params.addParam<unsigned int>(
      "num_random_seeds",
      100000,
      "Initialize a certain number of random seeds. Change from the default only if you have to.");
  return params;
}

AdaptiveImportanceSampler::AdaptiveImportanceSampler(const InputParameters & parameters)
  : Sampler(parameters),
    ReporterInterface(this),
    _proposal_std(getParam<std::vector<Real>>("proposal_std")),
    _initial_values(getParam<std::vector<Real>>("initial_values")),
    _output_limit(getParam<Real>("output_limit")),
    _num_samples_train(getParam<int>("num_samples_train")),
    _num_importance_sampling_steps(getParam<int>("num_importance_sampling_steps")),
    _std_factor(getParam<Real>("std_factor")),
    _use_absolute_value(getParam<bool>("use_absolute_value")),
    _num_random_seeds(getParam<unsigned int>("num_random_seeds")),
    _is_sampling_completed(false),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep()),
    _inputs(getReporterValue<std::vector<std::vector<Real>>>("inputs_reporter"))
{
  // Filling the `distributions` vector with the user-provided distributions.
  for (const DistributionName & name : getParam<std::vector<DistributionName>>("distributions"))
    _distributions.push_back(&getDistributionByName(name));

  /* Adaptive Importance Sampling (AdaptiveImportanceSampler) relies on a Markov Chain Monte Carlo
     (MCMC) algorithm. As such, in MOOSE, any use of MCMC algorithms requires that the `num_steps`
     parameter in the main App's executioner would control the total number of samples. Therefore,
     the `num_rows` parameter typically used by exisiting non-MCMC samplers to set the total number
     of samples has no use here and is fixed to 1.*/
  setNumberOfRows(1);

  // Setting the number of columns in the sampler matrix (equal to the number of distributions).
  setNumberOfCols(_distributions.size());

  /* `inputs_sto` is a member variable that aids in forming the importance distribution.
     One dimension of this variable is equal to the number of distributions. The other dimension
     of the variable, at the last step, is equal to the number of samples the user desires.*/
  _inputs_sto.resize(_distributions.size());

  // Mapping all the input distributions to a standard normal space
  for (unsigned int i = 0; i < _distributions.size(); ++i)
    _inputs_sto[i].push_back(Normal::quantile(_distributions[i]->cdf(_initial_values[i]), 0, 1));

  /* `prev_value` is a member variable for tracking the previously accepted samples in the
     MCMC algorithm and proposing the next sample.*/
  _prev_value.resize(_distributions.size());

  // `check_step` is a member variable for ensuring that the MCMC algorithm proceeds in a sequential
  // fashion.
  _check_step = 0;

  // Storage for means of input values for proposing the next sample
  _mean_sto.resize(_distributions.size());

  // Storage for standard deviations of input values for proposing the next sample
  _std_sto.resize(_distributions.size());

  setNumberOfRandomSeeds(_num_random_seeds);
}

Real
AdaptiveImportanceSampler::computeSample(dof_id_type /*row_index*/, dof_id_type col_index)
{
  const bool sample = _step > 1 && col_index == 0 && _check_step != _step;

  if (sample && _is_sampling_completed)
    mooseError("Internal bug: the adaptive sampling is supposed to be completed but another sample "
               "has been requested.");

  if (_step <= _num_samples_train)
  {
    /* This is the importance distribution training step. Markov Chains are set up
       to sample from the importance region or the failure region using the Metropolis
       algorithm. Given that the previous sample resulted in a model failure, the next
       sample is proposed such that it is very likely to result in a model failure as well.
       The `initial_values` and `proposal_std` parameters provided by the user affects the
       formation of the importance distribution. */
    if (sample)
    {
      for (dof_id_type j = 0; j < _distributions.size(); ++j)
        _prev_value[j] = Normal::quantile(_distributions[j]->cdf(_inputs[j][0]), 0.0, 1.0);
      Real acceptance_ratio = 0.0;
      for (dof_id_type i = 0; i < _distributions.size(); ++i)
        acceptance_ratio += std::log(Normal::pdf(_prev_value[i], 0.0, 1.0)) -
                            std::log(Normal::pdf(_inputs_sto[i].back(), 0.0, 1.0));
      if (acceptance_ratio > std::log(getRand(_step)))
      {
        for (dof_id_type i = 0; i < _distributions.size(); ++i)
          _inputs_sto[i].push_back(_prev_value[i]);
      }
      else
      {
        for (dof_id_type i = 0; i < _distributions.size(); ++i)
          _inputs_sto[i].push_back(_inputs_sto[i].back());
      }
      for (dof_id_type i = 0; i < _distributions.size(); ++i)
        _prev_value[i] = Normal::quantile(getRand(_step), _inputs_sto[i].back(), _proposal_std[i]);
    }
  }
  else if (sample)
  {
    /* This is the importance sampling step using the importance distribution created
       in the previous step. Once the importance distribution is known, sampling from
       it is similar to a regular Monte Carlo sampling. */
    for (dof_id_type i = 0; i < _distributions.size(); ++i)
    {
      if (_step == _num_samples_train + 1)
      {
        _mean_sto[i] = AdaptiveMonteCarloUtils::computeMean(_inputs_sto[i], 1);
        _std_sto[i] = AdaptiveMonteCarloUtils::computeSTD(_inputs_sto[i], 1);
      }
      _prev_value[i] = (Normal::quantile(getRand(_step), _mean_sto[i], _std_factor * _std_sto[i]));
    }

    // check if we have performed all the importance sampling steps
    if (_step >= _num_samples_train + _num_importance_sampling_steps)
      _is_sampling_completed = true;
  }

  _check_step = _step;
  return _distributions[col_index]->quantile(Normal::cdf(_prev_value[col_index], 0.0, 1.0));
}
