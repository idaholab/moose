//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AIS.h"
#include "Distribution.h"
#include "Normal.h"
#include "Uniform.h"
// #include "AdaptiveMonteCarloUtils.h"

registerMooseObjectAliased("StochasticToolsApp", AIS, "AIS");
registerMooseObjectReplaced("StochasticToolsApp",
                            AIS,
                            "07/01/2020 00:00",
                            MonteCarlo);

InputParameters
AIS::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Adaptive Importance Sampler.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions",
      "The distribution names to be sampled, the number of distributions provided defines the "
      "number of columns per matrix.");
  params.addRequiredParam<ReporterName>(
      "output_reporter", "Reporter with results of samples created by trainer.");
  params.addRequiredParam<std::vector<ReporterName>>(
      "inputs_reporter", "Reporter with input parameters.");
  params.addRequiredParam<std::vector<Real>>(
      "proposal_std",
      "Standard deviations of the proposal distributions");
  params.addRequiredParam<Real>(
      "output_limit",
      "Limiting values of the VPPs");
  params.addRequiredParam<std::vector<Real>>(
      "initial_values",
      "Initial input values to get the importance sampler started");
  params.addRequiredParam<int>(
      "num_samples_train",
      "Number of samples to learn the importance distribution");
  params.addRequiredParam<Real>(
      "std_factor",
      "Factor to be multiplied to the standard deviation of the importance samples");
  params.addParam<bool>(
      "use_absolute_value", false,
      "Use absolute value of the sub app output");
  params.addParam<unsigned int>(
      "num_random_seeds", 100000,
      "Initialize a certain number of random seeds. Change from the default only if you have to.");
  return params;
}

AIS::AIS(const InputParameters & parameters)
  : Sampler(parameters), ReporterInterface(this),
    _inputs_names(getParam<std::vector<ReporterName>>("inputs_reporter")),
    _distribution_names(getParam<std::vector<DistributionName>>("distributions")),
    _proposal_std(getParam<std::vector<Real>>("proposal_std")),
    _initial_values(getParam<std::vector<Real>>("initial_values")),
    _output_limit(getParam<Real>("output_limit")),
    _num_samples_train(getParam<int>("num_samples_train")),
    _std_factor(getParam<Real>("std_factor")),
    _use_absolute_value(getParam<bool>("use_absolute_value")),
    _num_random_seeds(getParam<unsigned int>("num_random_seeds")),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep()),
    _perf_compute_sample(registerTimedSection("computeSample", 4))
{
  // Filling the `distributions` vector with the user-provided distributions.
  for (const DistributionName & name : _distribution_names)
    _distributions.push_back(&getDistributionByName(name));

  /* Adaptive Importance Sampling (AIS) relies on a Markov Chain Monte Carlo (MCMC) algorithm.
     As such, in MOOSE, any use of MCMC algorithms requires that the `num_steps` parameter in the
     main App's executioner would control the total number of samples. Therefore, the `num_rows` parameter
     typically used by exisiting non-MCMC samplers to set the total number of samples
     has no use here and is fixed to 1.*/
  int num_rows = 1;
  setNumberOfRows(num_rows);

  // Setting the number of columns in the sampler matrix (equal to the number of distributions).
  setNumberOfCols(_distributions.size());

  /* `inputs_sto` is a member variable that aids in forming the importance distribution.
     One dimension of this variable is equal to the number of distributions. The other dimension
     of the variable, at the last step, is equal to the number of samples the user desires.*/
  _inputs_sto.resize(_distributions.size());

  // Mapping all the input distributions to a standard normal space
  for (unsigned int i = 0; i < _distributions.size(); ++i)
    _inputs_sto[i].push_back(Normal::quantile(_distributions[i]->cdf(_initial_values[i]),0,1));

  /* `prev_value` is a member variable for tracking the previously accepted samples in the
     MCMC algorithm and proposing the next sample.*/
  _prev_value.resize(_distributions.size());

  // `check_step` is a member variable for ensuring that the MCMC algorithm proceeds in a sequential fashion.
  _check_step = 0;

  // Storage for means of input values for proposing the next sample
  _mean_sto.resize(_distributions.size());

  // Storage for standard deviations of input values for proposing the next sample
  _std_sto.resize(_distributions.size());

  setNumberOfRandomSeeds(_num_random_seeds);
}

Real
AIS::computeSample(dof_id_type /*row_index*/, dof_id_type col_index)
{
  TIME_SECTION(_perf_compute_sample);

  if (_step <= _num_samples_train)
  {
    /* This is the importance distribution training step. Markov Chains are set up
       to sample from the importance region or the failure region using the Metropolis
       algorithm. Given that the previous sample resulted in a model failure, the next
       sample is proposed such that it is very likely to result in a model failure as well.
       The `initial_values` and `proposal_std` parameters provided by the user affects the
       formation of the importance distribution. */
    if (_step > 1 && col_index == 0 && _check_step != _step)
    {
      for (dof_id_type j = 0; j < _distributions.size(); ++j)
        _prev_value[j] = Normal::quantile(_distributions[j]->cdf(getReporterValueByName<Real>(_inputs_names[j])),0,1);
      _acceptance_ratio = 0.0;
      for (dof_id_type i = 0; i < _distributions.size(); ++i)
        _acceptance_ratio += std::log(Normal::pdf(_prev_value[i],0,1)) - std::log(Normal::pdf(_inputs_sto[i][_inputs_sto[i].size()-1],0,1));
      if (_acceptance_ratio > std::log(getRand(_step)))
      {
        for (dof_id_type i = 0; i < _distributions.size(); ++i)
          _inputs_sto[i].push_back(_prev_value[i]);
      } else
      {
        for (dof_id_type i = 0; i < _distributions.size(); ++i)
          _inputs_sto[i].push_back(_inputs_sto[i][_inputs_sto[i].size()-1]);
      }
    }
    _check_step = _step;
    _prev_value[col_index] = Normal::quantile(getRand(_step), _inputs_sto[col_index][_inputs_sto[col_index].size()-1], _proposal_std[col_index]);
    return _distributions[col_index]->quantile(Normal::cdf(_prev_value[col_index],0,1));
  } else if (_step == _num_samples_train + 1)
  {
    /* This is the importance sampling step using the importance distribution created
       in the previous step. Once the importance distribution is known, sampling from
       it is similar to a regular Monte Carlo sampling. */
    if (col_index == 0 && _check_step != _step)
    {
      for (dof_id_type i = 0; i < _distributions.size(); ++i)
      {
        _mean_sto[i] = StochasticTools::AdaptiveMonteCarloUtils::computeMEAN(_inputs_sto[i], 2);
        _std_sto[i] = StochasticTools::AdaptiveMonteCarloUtils::computeSTD(_inputs_sto[i], 2);
        _prev_value[i] = (Normal::quantile(getRand(_step), _mean_sto[i], _std_factor * _std_sto[i]));
      }
    }
    _check_step = _step;
    return _distributions[col_index]->quantile(Normal::cdf(_prev_value[col_index],0,1));
  } else
  {
    /* This is the importance sampling step using the importance distribution created
       in the previous step. Once the importance distribution is known, sampling from
       it is similar to a regular Monte Carlo sampling. */
    if (col_index == 0 && _check_step != _step)
    {
      for (dof_id_type i = 0; i < _distributions.size(); ++i)
        _prev_value[i] = (Normal::quantile(getRand(_step), _mean_sto[i], _std_factor * _std_sto[i]));
    }
    _check_step = _step;
    return _distributions[col_index]->quantile(Normal::cdf(_prev_value[col_index],0,1));
  }
}
