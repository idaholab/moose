//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BayesianActiveLearningSampler.h"
#include "Normal.h"
#include "Uniform.h"

registerMooseObject("StochasticToolsApp", BayesianActiveLearningSampler);

InputParameters
BayesianActiveLearningSampler::validParams()
{
  InputParameters params = PMCMCBase::validParams();
  params.addClassDescription("Fast Bayesian inference with the parallel active learning (partly "
                             "inspired from El Gammal et al. 2023).");
  params.addRequiredParam<ReporterName>(
      "sorted_indices", "The sorted sample indices in order of importance to evaluate the subApp.");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "num_tries",
      "num_tries>0",
      "Number of samples to propose in each iteration (not all are sent for subApp evals).");
  return params;
}

BayesianActiveLearningSampler::BayesianActiveLearningSampler(const InputParameters & parameters)
  : PMCMCBase(parameters),
    _sorted_indices(getReporterValue<std::vector<unsigned int>>("sorted_indices")),
    _num_tries(getParam<unsigned int>("num_tries")),
    _inputs_test(_num_tries, std::vector<Real>(_priors.size())),
    _var_test(_num_tries)
{
}

void
BayesianActiveLearningSampler::fillVector(std::vector<Real> & vector,
                                          const unsigned int & seed_value,
                                          std::size_t & rn_ind)
{
  for (unsigned int i = 0; i < _priors.size(); ++i)
    vector[i] = _priors[i]->quantile(getRandStateless(rn_ind++, seed_value));
}

const std::vector<std::vector<Real>> &
BayesianActiveLearningSampler::getSampleTries() const
{
  return _inputs_test;
}

const std::vector<Real> &
BayesianActiveLearningSampler::getVarSampleTries() const
{
  return _var_test;
}

void
BayesianActiveLearningSampler::proposeSamples(const unsigned int seed_value, std::size_t & rn_ind)
{
  /* If step is 1, randomly generate the samples.
  Else, generate the samples informed by the GP from the reporter "sorted_indices" */
  for (dof_id_type i = 0; i < _num_parallel_proposals; ++i)
  {
    if (_t_step <= 1)
    {
      fillVector(_new_samples[i], seed_value, rn_ind);
      if (_var_prior)
        _new_var_samples[i] = _var_prior->quantile(getRandStateless(rn_ind++, seed_value));
    }
    else
    {
      _new_samples[i] = _inputs_test[_sorted_indices[i]];
      if (_var_prior)
        _new_var_samples[i] = _var_test[_sorted_indices[i]];
    }
  }

  /* Finally, generate several new samples randomly for the GP to try and pass it to the
  reporter */
  for (dof_id_type i = 0; i < _num_tries; ++i)
  {
    fillVector(_inputs_test[i], seed_value, rn_ind);
    if (_var_prior)
      _var_test[i] = _var_prior->quantile(getRandStateless(rn_ind++, seed_value));
  }
}
