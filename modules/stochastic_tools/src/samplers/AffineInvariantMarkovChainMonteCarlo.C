//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AffineInvariantMarkovChainMonteCarlo.h"
#include "AdaptiveMonteCarloUtils.h"
#include "Normal.h"
#include "TruncatedNormal.h"
#include "Uniform.h"

registerMooseObjectAliased("StochasticToolsApp", AffineInvariantMarkovChainMonteCarlo, "AffineInvariantMCMC");

InputParameters
AffineInvariantMarkovChainMonteCarlo::validParams()
{
  InputParameters params = ParallelMarkovChainMonteCarloBase::validParams();
  params.suppressParameter<ReporterName>("seed_inputs");
  params.suppressParameter<std::vector<Real>>("std_prop");
  params.addParam<Real>("step_size", 2.0, "Step size for each of the walkers.");
  params.addParam<ReporterName>("previous_state",
                                "Reporter with seed inputs values for the next proposals.");
  return params;
}

AffineInvariantMarkovChainMonteCarlo::AffineInvariantMarkovChainMonteCarlo(const InputParameters & parameters)
  : ParallelMarkovChainMonteCarloBase(parameters),
    _step_size(getParam<Real>("step_size")),
    _previous_state(getReporterValue<std::vector<std::vector<Real>>>("previous_state"))
{
    _initial_walkers.resize(_num_parallel_proposals);
    for (unsigned int i = 0; i < _initial_walkers.size(); ++i)
        _initial_walkers[i].resize(_priors.size());
}

// void
// AffineInvariantMarkovChainMonteCarlo::initializeWalkers(const unsigned int seed)
// {
//   for (unsigned int i = 0; i < _num_parallel_proposals; ++i)
//   {
//     for (unsigned int j = 0; j < _priors.size(); ++j)
//         _initial_values[i][j] = _priors[j]->quantile(getRand(seed));
//   }
// }

void
AffineInvariantMarkovChainMonteCarlo::sampleSetUp(const SampleMode /*mode*/)
{
  if (_step < 1 || _check_step == _step)
    return;
  _check_step = _step;

  unsigned int seed_value = _step > 0 ? (_step - 1) : 0;
  
  // Filling the new_samples vector of vectors with new proposal samples
  std::vector<Real> tmp(_priors.size());
  unsigned int count1 = 0;
  if (_step < 3)
  {
    for (unsigned int j = 0; j < _num_parallel_proposals; ++j)
    {
      for (unsigned int i = 0; i < _priors.size(); ++i)
        tmp[i] = _priors[i]->quantile(getRand(seed_value));
      for (unsigned int i = 0; i < _confg_values.size(); ++i)
      {
        tmp[_priors.size()] = _confg_values[i];
        _new_samples[count1] = tmp;
        count1 += 1;
      }
    }
  }
  else
  {
    for (unsigned int j = 0; j < _num_parallel_proposals; ++j)
    {
      for (unsigned int i = 0; i < _priors.size(); ++i)
      {
        if (_lb)
          tmp[i] = TruncatedNormal::quantile(getRand(seed_value), _seed_inputs[i], _std_prop[i], (*_lb)[i], (*_ub)[i]);
        else
          tmp[i] = Normal::quantile(getRand(seed_value), _seed_inputs[i], _std_prop[i]);
      }
      for (unsigned int i = 0; i < _confg_values.size(); ++i)
      {
        tmp[_priors.size()] = _confg_values[i];
        _new_samples[count1] = tmp;
        count1 += 1;
      }
    }
  }
}

Real
AffineInvariantMarkovChainMonteCarlo::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  return (_new_samples[row_index][col_index]);
}
