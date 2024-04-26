//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#include "BayesianGPrySampler.h"
#include "Normal.h"
#include "Uniform.h"

registerMooseObject("StochasticToolsApp", BayesianGPrySampler);

InputParameters
BayesianGPrySampler::validParams()
{
  InputParameters params = PMCMCBase::validParams();
  params.addClassDescription(
      "Fast Bayesian inference with the GPry algorithm by El Gammal et al. 2023: sampler step.");
  params.addRequiredParam<ReporterName>("sorted_indices",
                                        "The sorted sample indices in order of importance to evaluate the subApp.");
  params.addRequiredRangeCheckedParam<unsigned int>(
    "num_tries",
    "num_tries>0",
    "Number of samples to propose in each iteration (not all are sent for subApp evals).");
  return params;
}

BayesianGPrySampler::BayesianGPrySampler(const InputParameters & parameters)
  : PMCMCBase(parameters),
    _sorted_indices(getReporterValue<std::vector<unsigned int>>("sorted_indices")),
    _num_tries(getParam<unsigned int>("num_tries"))
{
  _inputs_all.resize(_num_tries, std::vector<Real>(_priors.size()));
  _var_all.resize(_num_tries);
  _sample_vector.resize(_priors.size());
}

void
BayesianGPrySampler::fillVector(std::vector<Real> & vector, const unsigned int & seed_value)
{
  for (unsigned int i = 0; i < _priors.size(); ++i)
    vector[i] = _priors[i]->quantile(getRand(seed_value));
}

const std::vector<std::vector<Real>> &
BayesianGPrySampler::getSampleTries() const
{
  return _inputs_all;
}

const std::vector<Real> &
BayesianGPrySampler::getVarSampleTries() const
{
  return _var_all;
}

void
BayesianGPrySampler::proposeSamples(const unsigned int seed_value)
{
  // If step is 1, randomly generate the samples
  // Else, generate the samples informed by the GP and NN combo from the reporter "sorted_indices"
  for (dof_id_type i = 0; i < _num_parallel_proposals; ++i)
  {
    if (_t_step <= 1)
    {
      fillVector(_sample_vector, seed_value);
      _new_samples[i] = _sample_vector;
      if (_var_prior)
        _new_var_samples[i] = _var_prior->quantile(getRand(seed_value));
    }
    else
    {
      _new_samples[i] = _inputs_all[_sorted_indices[i]];
      if (_var_prior)
        _new_var_samples[i] = _var_all[_sorted_indices[i]];
    }
  }

  // Finally, generate several new samples randomly for the GP and NN to try and pass it to the reporter
  for (dof_id_type i = 0; i < _num_tries; ++i)
  {
    fillVector(_sample_vector, seed_value);
    _inputs_all[i] = _sample_vector;
    if (_var_prior)
      _var_all[i] = _var_prior->quantile(getRand(seed_value));
  }
}

#endif
