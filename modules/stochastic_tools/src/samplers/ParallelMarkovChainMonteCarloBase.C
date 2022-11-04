//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParallelMarkovChainMonteCarloBase.h"
#include "AdaptiveMonteCarloUtils.h"
#include "Normal.h"
#include "Uniform.h"

registerMooseObjectAliased("StochasticToolsApp", ParallelMarkovChainMonteCarloBase, "PMCMCBase");

InputParameters
ParallelMarkovChainMonteCarloBase::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Parallel Markov chain Monte Carlo base.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "prior distributions",
      "The prior distributions of the parameters to be calibrated.");
  params.addRequiredParam<ReporterName>("seed_inputs",
                                        "Reporter with seed inputs values for the next proposals.");
  params.addRequiredParam<unsigned int>("num_parallel_proposals",
                                        "Number of proposals to made and corresponding subApps executed in "
                                        "parallel.");
  // params.addRequiredParam<LikelihoodName>("likelihood", "Name of the likelihood function.");
  params.addRequiredParam<std::vector<Real>>("initial_values", "The starting values of the inputs to be calibrated.");
  params.addParam<unsigned int>(
      "num_random_seeds",
      100000,
      "Initialize a certain number of random seeds. Change from the default only if you have to.");
  return params;
}

ParallelMarkovChainMonteCarloBase::ParallelMarkovChainMonteCarloBase(const InputParameters & parameters)
  : Sampler(parameters),
    ReporterInterface(this),
    LikelihoodInterface(this),
    _seed_inputs(getReporterValue<std::vector<Real>>("seed_inputs")),
    _num_parallel_proposals(getParam<unsigned int>("num_parallel_proposals")),
    // _likelihood(getLikelihoodByName(getParam<LikelihoodName>("likelihood"))),
    _initial_values(getParam<std::vector<Real>>("initial_values")),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep()),
    _check_step(0),
    _num_random_seeds(getParam<unsigned int>("num_random_seeds"))
{
  // Filling the `priors` vector with the user-provided distributions.
  for (const DistributionName & name : getParam<std::vector<DistributionName>>("distributions"))
    _priors.push_back(&getDistributionByName(name));

  // Setting the number of sampler rows to be equal to the number of parallel proposals
  setNumberOfRows(_num_parallel_proposals+1);

  // Setting the number of columns in the sampler matrix (equal to the number of distributions).
  setNumberOfCols(_priors.size());

  // Resizing the new samples vector of vectors
  _new_samples.resize(_priors.size());
  for (unsigned int i = 0; i < _priors.size(); ++i)
    _new_samples[i].resize(_num_parallel_proposals+1);
  
  setNumberOfRandomSeeds(_num_random_seeds);
}

void
ParallelMarkovChainMonteCarloBase::sampleSetUp(const SampleMode /*mode*/)
{
  if (_step < 1 || _check_step == _step)
    return;
  _check_step = _step;

  unsigned int seed_value = _step > 0 ? (_step - 1) : 0;
  
  // Filling the new_samples vector of vectors with new proposal samples
  if (_step == 1)
  {
    for (unsigned int i = 0; i < _priors.size(); ++i)
    {
        for (unsigned int j = 0; j < _num_parallel_proposals; ++j)
            _new_samples[i][j] = Normal::quantile(getRand(seed_value), _initial_values[i], 1.0);
        _new_samples[i][_num_parallel_proposals] = _initial_values[i];
    }   
  }
  else
  {
    for (unsigned int i = 0; i < _priors.size(); ++i)
    {
        for (unsigned int j = 0; j < _num_parallel_proposals; ++j)
            _new_samples[i][j] = Normal::quantile(getRand(seed_value), _seed_inputs[i], 1.0);
        _new_samples[i][_num_parallel_proposals] = _seed_inputs[i];
    }
  }
}

Real
ParallelMarkovChainMonteCarloBase::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  return _new_samples[col_index][row_index];
}
