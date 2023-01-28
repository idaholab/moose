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
#include "libmesh/utility.h"

registerMooseObjectAliased("StochasticToolsApp", AffineInvariantMarkovChainMonteCarlo, "AffineInvariantMCMC");

InputParameters
AffineInvariantMarkovChainMonteCarlo::validParams()
{
  InputParameters params = ParallelMarkovChainMonteCarloBase::validParams();
  // params.suppressParameter<ReporterName>("seed_inputs");
  // params.suppressParameter<std::vector<Real>>("std_prop");
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
  // Setting the number of sampler rows to be equal to the number of parallel proposals
  setNumberOfRows(_num_parallel_proposals * _confg_values.size());

  // Setting the number of columns in the sampler matrix (equal to the number of distributions).
  setNumberOfCols(_priors.size()+1);

  // Resizing the new samples vector of vectors
  _new_samples.resize(_num_parallel_proposals * _confg_values.size());
  for (unsigned int i = 0; i < (_num_parallel_proposals * _confg_values.size()); ++i)
    _new_samples[i].resize(_priors.size()+1);

    // _initial_walkers.resize(_num_parallel_proposals);
    // for (unsigned int i = 0; i < _initial_walkers.size(); ++i)
    //     _initial_walkers[i].resize(_priors.size());
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
  std::vector<Real> tmp(_priors.size()+1);
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
        _step_size_sto[count1] = 1.0;
        count1 += 1;
      }
    }
  }
  else
  {
    unsigned int j = 0;
    bool indicator = 0;
    while (j < _num_parallel_proposals)
    {
      Real z = Utility::pow<2>((_step_size - 1.0) * getRand(seed_value) + 1.0) / _step_size;
      unsigned int index_req = AdaptiveMonteCarloUtils::randomIndex(_num_parallel_proposals, j);
      // std::cout << "z val " << z << std::endl;
      std::cout << "index_req " << index_req << std::endl;
      for (unsigned int i = 0; i < _priors.size(); ++i)
      {
        // std::cout << "_previous_state[j][i] " << _previous_state[j][i] << std::endl;
        // std::cout << "_previous_state[index_req][i] " << _previous_state[index_req][i] << std::endl;
        Real req_val = _previous_state[j][i] - z * (_previous_state[j][i] - _previous_state[index_req][i]);
        if (_lb)
        {
          if (req_val < (*_lb)[i] || req_val > (*_ub)[i])
            indicator = 1;
          else
            indicator = 0;
        }
        tmp[i] = req_val;
      }
      if (!indicator)
      {
        for (unsigned int i = 0; i < _confg_values.size(); ++i)
        {
          tmp[_priors.size()] = _confg_values[i];
          _new_samples[count1] = tmp;
          _step_size_sto[count1] = z;
          count1 += 1;
        }
        ++j;
      }
      else 
        z = Utility::pow<2>((_step_size - 1.0) * getRand(seed_value) + 1.0) / _step_size;
    }
  }
  // for (unsigned int i = 0; i < ((_num_parallel_proposals) * _confg_values.size()); ++i)
  //   std::cout << "New samples: " << Moose::stringify(_new_samples[i]) << std::endl; //
  // for (unsigned int i = 0; i < _num_parallel_proposals; ++i)
  //   std::cout << "Old samples: " << Moose::stringify(_previous_state[i]) << std::endl;
  
     
}

Real
AffineInvariantMarkovChainMonteCarlo::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  std::vector<Real> init;
  init = {0.05, 0.05, 8};

  if (_step == 0)
    return (init[col_index]);
  else
    return (_new_samples[row_index][col_index]);

  // return (_new_samples[row_index][col_index]);
}
