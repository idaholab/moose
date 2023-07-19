//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AffineInvariantStretchSampler.h"

registerMooseObject("StochasticToolsApp", AffineInvariantStretchSampler);

InputParameters
AffineInvariantStretchSampler::validParams()
{
  InputParameters params = PMCMCBase::validParams();
  params.addClassDescription("Perform Affine Invariant Ensemble MCMC with stretch sampler.");
  params.addRequiredParam<ReporterName>(
      "previous_state", "Reporter value with the previous state of all the walkers.");
  params.addRequiredParam<ReporterName>(
      "previous_state_var",
      "Reporter value with the previous state of all the walkers for variance.");
  params.addParam<Real>("step_size", 2.0, "Step size for each of the walkers.");
  return params;
}

AffineInvariantStretchSampler::AffineInvariantStretchSampler(const InputParameters & parameters)
  : PMCMCBase(parameters),
    _step_size(getParam<Real>("step_size")),
    _previous_state(getReporterValue<std::vector<std::vector<Real>>>("previous_state")),
    _previous_state_var(getReporterValue<std::vector<Real>>("previous_state_var"))
{
  if (_num_parallel_proposals < 3)
    paramError("num_parallel_proposals",
               "At least three parallel proposals should be used for the Stretch Sampler.");

  if (_num_parallel_proposals < _priors.size())
    mooseWarning(
        "It is recommended that the parallel proposals be greater than or equal to the "
        "inferred parameters. This will allow the sampler to not get stuck on a hyper-plane.");

  // Assign the correct size to the step size vector
  _affine_step.resize(_num_parallel_proposals);
}

void
AffineInvariantStretchSampler::proposeSamples(const unsigned int seed_value)
{
  unsigned int j = 0;
  bool indicator;
  unsigned int index_req = 0;
  while (j < _num_parallel_proposals)
  {
    indicator = 0;
    _affine_step[j] = Utility::pow<2>((_step_size - 1.0) * getRand(seed_value) + 1.0) / _step_size;
    randomIndex(_num_parallel_proposals, j, seed_value, index_req);
    for (unsigned int i = 0; i < _priors.size(); ++i)
    {
      _new_samples[j][i] =
          (_t_step > decisionStep())
              ? (_previous_state[index_req][i] +
                 _affine_step[j] * (_previous_state[j][i] - _previous_state[index_req][i]))
              : _priors[i]->quantile(getRand(seed_value));
      if (_lower_bound)
        indicator =
            (_new_samples[j][i] < (*_lower_bound)[i] || _new_samples[j][i] > (*_upper_bound)[i])
                ? 1
                : indicator;
    }
    if (_var_prior)
    {
      _new_var_samples[j] =
          (_t_step > decisionStep())
              ? (_previous_state_var[index_req] +
                 _affine_step[j] * (_previous_state_var[j] - _previous_state_var[index_req]))
              : _var_prior->quantile(getRand(seed_value));
      if (_new_var_samples[j] < 0.0)
        indicator = 1;
    }
    if (!indicator)
      ++j;
  }
}

const std::vector<Real> &
AffineInvariantStretchSampler::getAffineStepSize() const
{
  return _affine_step;
}
