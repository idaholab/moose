//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AffineInvariantStretchSampler.h"

InputParameters
AffineInvariantStretchSampler::validParams()
{
  InputParameters params = ParallelMarkovChainMonteCarloBase::validParams();
  params.addClassDescription("Perform Affine Invariant Ensemble MCMC with stretch sampler.");
  params.addRequiredParam<ReporterName>("previous_state",
                                "Reporter value with the previous state of all the walkers.");
  params.addParam<Real>("step_size", 2.0, "Step size for each of the walkers.");
  return params;
}

AffineInvariantStretchSampler::AffineInvariantStretchSampler(const InputParameters & parameters)
  : ParallelMarkovChainMonteCarloBase(parameters),
    _step_size(getParam<Real>("step_size")),
    _previous_state(getReporterValue<std::vector<std::vector<Real>>>("previous_state"))
{
  if (_num_parallel_proposals < 3)
    mooseError("At least three parallel proposals should be used for the Stretch Sampler.");
  
  if (_num_parallel_proposals < _priors.size())
    mooseWarning("It is recommended that the parallel proposals be greater than or equal to the inferred parameters.");
}

void
AffineInvariantStretchSampler::sampleSetUp(const SampleMode /*mode*/)
{
  if (_step < 1 || _check_step == _step)
    return;
  _check_step = _step;

  unsigned int seed_value = _step > 0 ? (_step - 1) : 0;
  
  // Filling the new_samples vector of vectors with new proposal samples
  unsigned int j = 0;
  bool indicator;
  Real z;
  unsigned int index_req = 0;
  while (j < _num_parallel_proposals)
  {
    indicator = 0;
    z = Utility::pow<2>((_step_size - 1.0) * getRand(seed_value) + 1.0) / _step_size;
    randomIndex(_num_parallel_proposals, j, seed_value, index_req);
    for (unsigned int i = 0; i < _priors.size(); ++i)
    {
      _new_samples[j][i] = (_step > 2) ? (_previous_state[j][i] - z * (_previous_state[j][i] - _previous_state[index_req][i])) : _priors[i]->quantile(getRand(seed_value));
      if (_lb)
        indicator = (_new_samples[j][i] < (*_lb)[i] || _new_samples[j][i] > (*_ub)[i]) ? 1 : indicator;
    }
    j = (!indicator) ? ++j : j;
  }
}
