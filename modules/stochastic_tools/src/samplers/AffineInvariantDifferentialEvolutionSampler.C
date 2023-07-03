//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AffineInvariantDifferentialEvolutionSampler.h"
#include "Normal.h"

registerMooseObjectAliased("StochasticToolsApp", AffineInvariantDifferentialEvolutionSampler, "AffineInvariantDES");

/*
 Tuning options for the internal parameters
  1. Braak2006_static:
  - the gamma param is set to 2.38 / sqrt(2 * dim)
  - the b param is set to 1e-4
*/

InputParameters
AffineInvariantDifferentialEvolutionSampler::validParams()
{
  InputParameters params = ParallelMarkovChainMonteCarloBase::validParams();
  params.addClassDescription("Perform Affine Invariant Ensemble MCMC with stretch sampler.");
  params.addRequiredParam<ReporterName>("previous_state",
                                "Reporter value with the previous state of all the walkers.");
  params.addParam<Real>("step_size", 2.0, "Step size for each of the walkers.");
  MooseEnum tuning_option("Braak2006_static", "Braak2006_static");
  params.addParam<MooseEnum>("tuning_option", tuning_option, "The tuning option for internal parameters.");
  return params;
}

AffineInvariantDifferentialEvolutionSampler::AffineInvariantDifferentialEvolutionSampler(const InputParameters & parameters)
  : ParallelMarkovChainMonteCarloBase(parameters),
    _previous_state(getReporterValue<std::vector<std::vector<Real>>>("previous_state")),
    _tuning_option(getParam<MooseEnum>("tuning_option"))
{
  if (_num_parallel_proposals < 5)
    mooseError("At least five parallel proposals should be used for the Differential Evolution Sampler.");
  
  if (_num_parallel_proposals < _priors.size())
    mooseWarning("It is recommended that the parallel proposals be greater than or equal to the inferred parameters.");
}

void
AffineInvariantDifferentialEvolutionSampler::computeDifferential(const Real & state1, const Real & state2, const unsigned int & seed, Real & diff)
{
  Real gamma;
  Real b;
  tuneParams(gamma, b);
  diff = gamma * (state1 - state2) + Normal::quantile(getRand(seed), 0.0, b);
}

void
AffineInvariantDifferentialEvolutionSampler::tuneParams(Real & gamma, Real & b)
{
  if (_tuning_option == "Braak2006_static")
  {
    gamma = 2.38 / std::sqrt(2 * _priors.size());
    b = 1e-4;
  }
}

void
AffineInvariantDifferentialEvolutionSampler::sampleSetUp(const SampleMode /*mode*/)
{
  if (_step < 1 || _check_step == _step)
    return;
  _check_step = _step;

  unsigned int seed_value = _step > 0 ? (_step - 1) : 0;
  
  // Filling the new_samples vector of vectors with new proposal samples
  unsigned int j = 0;
  bool indicator;
  unsigned int index_req1;
  unsigned int index_req2;
  Real diff;
  while (j < _num_parallel_proposals)
  {
    indicator = 0;
    randomIndex2(_num_parallel_proposals, j, seed_value, index_req1, index_req2);
    for (unsigned int i = 0; i < _priors.size(); ++i)
    {
      computeDifferential(_previous_state[index_req1][i], _previous_state[index_req2][i], seed_value, diff);
      _new_samples[j][i] = (_step > 2) ? (_previous_state[j][i] + diff) : _priors[i]->quantile(getRand(seed_value));
      if (_lb)
        indicator = (_new_samples[j][i] < (*_lb)[i] || _new_samples[j][i] > (*_ub)[i]) ? 1 : indicator;
    }
    j = (!indicator) ? ++j : j;
  }
}
