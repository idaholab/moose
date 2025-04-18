//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  params.addClassDescription("Monte Carlo Sampler for active learning with surrogate model.");
  params.addRequiredParam<dof_id_type>("num_batch",
                                       "The number of full model evaluations in the batch.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions",
      "The distribution names to be sampled, the number of distributions provided defines the "
      "number of columns per matrix.");
  params.addRequiredParam<ReporterName>("flag_sample",
                                        "Flag samples if the surrogate prediction was inadequate.");
  params.addParam<unsigned int>(
      "num_random_seeds",
      100000,
      "Initialize a certain number of random seeds. Change from the default only if you have to.");
  params.addRequiredRangeCheckedParam<int>(
      "num_samples",
      "num_samples>0",
      "Number of samples to use (the total number of steps taken will be equal to this number + "
      "the number of re-training steps).");
  return params;
}

ActiveLearningMonteCarloSampler::ActiveLearningMonteCarloSampler(const InputParameters & parameters)
  : Sampler(parameters),
    _flag_sample(getReporterValue<std::vector<bool>>("flag_sample")),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep()),
    _num_batch(getParam<dof_id_type>("num_batch")),
    _check_step(std::numeric_limits<int>::min()),
    _num_samples(getParam<int>("num_samples"))
{
  for (const DistributionName & name : getParam<std::vector<DistributionName>>("distributions"))
    _distributions.push_back(&getDistributionByName(name));
  setNumberOfRows(_num_batch);
  setNumberOfCols(_distributions.size());
  _inputs_sto.resize(_num_batch, std::vector<Real>(_distributions.size()));
  setNumberOfRandomSeeds(getParam<unsigned int>("num_random_seeds"));
}

void
ActiveLearningMonteCarloSampler::sampleSetUp(const Sampler::SampleMode /*mode*/)
{
  // If we've already done this step, skip
  if (_check_step == _step)
    return;

  if (_is_sampling_completed)
    mooseError("Internal bug: the adaptive sampling is supposed to be completed but another sample "
               "has been requested.");

  // Keep data where the GP failed
  if (_step > 0)
    for (dof_id_type i = 0; i < _num_batch; ++i)
      if (_flag_sample[i])
      {
        _inputs_gp_fails.push_back(_inputs_sto[i]);

        // When the GP fails, the current time step is 'wasted' and the retraining step doesn't
        // happen until the next time step. Therefore, keep track of the number of retraining steps
        // to increase the total number of steps taken.
        ++_retraining_steps;
      }

  // If we don't have enough failed inputs, generate new ones
  if (_inputs_gp_fails.size() < _num_batch)
  {
    for (dof_id_type i = 0; i < _num_batch; ++i)
      for (dof_id_type j = 0; j < _distributions.size(); ++j)
        _inputs_sto[i][j] = _distributions[j]->quantile(getRand(_step));
  }
  // If we do have enough failed inputs, assign them and clear the tracked ones
  else
  {
    _inputs_sto.assign(_inputs_gp_fails.begin(), _inputs_gp_fails.begin() + _num_batch);
    _inputs_gp_fails.erase(_inputs_gp_fails.begin(), _inputs_gp_fails.begin() + _num_batch);
  }

  _check_step = _step;

  // check if we have finished the sampling
  if (_step >= _num_samples + _retraining_steps)
    _is_sampling_completed = true;
}

Real
ActiveLearningMonteCarloSampler::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  return _inputs_sto[row_index][col_index];
}
