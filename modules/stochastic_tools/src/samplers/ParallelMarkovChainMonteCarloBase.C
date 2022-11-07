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
#include "DelimitedFileReader.h"

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
  params.addRequiredParam<FileName>("file_name", "Name of the CSV file with configuration values.");
  params.addParam<std::string>(
      "file_column_name", "Name of column in CSV file to use, by default first column is used.");
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

  MooseUtils::DelimitedFileReader reader(getParam<FileName>("file_name"));
  reader.read();
  if (isParamValid("file_column_name"))
    _confg_values = reader.getData(getParam<std::string>("file_column_name"));
  else
    _confg_values = reader.getData(0);

  // Setting the number of sampler rows to be equal to the number of parallel proposals
  setNumberOfRows((_num_parallel_proposals + 1) * _confg_values.size());

  // Setting the number of columns in the sampler matrix (equal to the number of distributions).
  setNumberOfCols(_priors.size()+1);

  // Resizing the new samples vector of vectors
  _new_samples.resize((_num_parallel_proposals + 1) * _confg_values.size());
  for (unsigned int i = 0; i < ((_num_parallel_proposals + 1) * _confg_values.size()); ++i)
    _new_samples[i].resize(_priors.size()+1);
  
  setNumberOfRandomSeeds(_num_random_seeds);
}

dof_id_type
ParallelMarkovChainMonteCarloBase::getNumberOfConfigParams() const
{
  return _confg_values.size();
}

void
ParallelMarkovChainMonteCarloBase::sampleSetUp(const SampleMode /*mode*/)
{
  if (_step < 1 || _check_step == _step)
    return;
  _check_step = _step;

  unsigned int seed_value = _step > 0 ? (_step - 1) : 0;
  
  // Filling the new_samples vector of vectors with new proposal samples
  std::vector<Real> tmp(_priors.size() + 1);
  unsigned int count1 = 0;
  if (_step == 1)
  {
    for (unsigned int j = 0; j < _num_parallel_proposals; ++j)
    {
        for (unsigned int i = 0; i < _priors.size(); ++i)
            tmp[i] = Normal::quantile(getRand(seed_value), _initial_values[i], 1.0);
        for (unsigned int i = 0; i < _confg_values.size(); ++i)
        {
            tmp[_priors.size()] = _confg_values[i];
            _new_samples[count1] = tmp;
            count1 += 1;
        }
    }
    for (unsigned int i = 0; i < _priors.size(); ++i)
        tmp[i] = _initial_values[i];
    for (unsigned int i = 0; i < _confg_values.size(); ++i)
    {
        tmp[_priors.size()] = _confg_values[i];
        _new_samples[_num_parallel_proposals + i] = tmp;
    }
  }
  else
  {
    for (unsigned int j = 0; j < _num_parallel_proposals; ++j)
    {
        for (unsigned int i = 0; i < _priors.size(); ++i)
            tmp[i] = Normal::quantile(getRand(seed_value), _seed_inputs[i], 1.0);
        for (unsigned int i = 0; i < _confg_values.size(); ++i)
        {
            tmp[_priors.size()] = _confg_values[i];
            _new_samples[count1] = tmp;
            count1 += 1;
        }
    }
    for (unsigned int i = 0; i < _priors.size(); ++i)
        tmp[i] = _seed_inputs[i];
    for (unsigned int i = 0; i < _confg_values.size(); ++i)
    {
        tmp[_priors.size()] = _confg_values[i];
        _new_samples[_num_parallel_proposals + i] = tmp;
    }
  }
}

Real
ParallelMarkovChainMonteCarloBase::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  return _new_samples[row_index][col_index];
}
