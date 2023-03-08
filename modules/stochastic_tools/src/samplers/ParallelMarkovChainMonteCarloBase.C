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
#include "Uniform.h"
#include "DelimitedFileReader.h"

registerMooseObjectAliased("StochasticToolsApp", ParallelMarkovChainMonteCarloBase, "PMCMCBase");

InputParameters
ParallelMarkovChainMonteCarloBase::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Parallel Markov chain Monte Carlo base.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "prior_distributions",
      "The prior distributions of the parameters to be calibrated.");
  params.addRequiredParam<unsigned int>("num_parallel_proposals",
                                        "Number of proposals to made and corresponding subApps executed in "
                                        "parallel.");
  params.addRequiredParam<FileName>("file_name", "Name of the CSV file with configuration values.");
  params.addParam<std::string>(
      "file_column_name", "Name of column in CSV file to use, by default first column is used.");
  params.addParam<std::vector<Real>>("lb", "Lower bounds for making the next proposal.");
  params.addParam<std::vector<Real>>("ub", "Upper bounds for making the next proposal.");
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
    _num_parallel_proposals(getParam<unsigned int>("num_parallel_proposals")),
    _lb(isParamValid("lb") ? &getParam<std::vector<Real>>("lb") : nullptr),
    _ub(isParamValid("ub") ? &getParam<std::vector<Real>>("ub") : nullptr),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep()),
    _check_step(0),
    _num_random_seeds(getParam<unsigned int>("num_random_seeds"))
{
  // Filling the `priors` vector with the user-provided distributions.
  for (const DistributionName & name : getParam<std::vector<DistributionName>>("prior_distributions"))
    _priors.push_back(&getDistributionByName(name));

  // Read the experimental configurations from a csv file
  MooseUtils::DelimitedFileReader reader(getParam<FileName>("file_name"));
  reader.read();
  if (isParamValid("file_column_name"))
    _confg_values = reader.getData(getParam<std::string>("file_column_name"));
  else
    _confg_values = reader.getData(0);

  // Setting the number of sampler rows to be equal to the number of parallel proposals
  setNumberOfRows(_num_parallel_proposals * _confg_values.size());

  // Setting the number of columns in the sampler matrix (equal to the number of distributions).
  setNumberOfCols(_priors.size() + 1);

  // Resizing the new samples vector of vectors
  _new_samples.resize(_num_parallel_proposals, std::vector<Real>(_priors.size(), 0.0));
  _new_samples_confg.resize(_num_parallel_proposals * _confg_values.size(), std::vector<Real>(_priors.size() + 1, 0.0));
  _rnd_vec.resize(_num_parallel_proposals);

  setNumberOfRandomSeeds(_num_random_seeds);

  _check_step = 0;

  // Check whether both the lower and the upper bounds are specified and of same size
  bool bound_check1 = _lb && !_ub;
  bool bound_check2 = !_lb && _ub;
  if (bound_check1 || bound_check2)
    mooseError("Both lower and upper bounds should be specified.");
  bool size_check = _lb ? ((*_lb).size() != (*_ub).size()) : 0;
  if (size_check)
    mooseError("Lower and upper bounds should be of the same size.");

  // Check whether both the priors and bounds are of the same size
  if ((*_lb).size() != _priors.size())
    mooseError("The bounds and priors should be of the same size.");
}

void
ParallelMarkovChainMonteCarloBase::sampleSetUp(const SampleMode /*mode*/)
{
  if (_step < 1 || _check_step == _step)
    return;
  _check_step = _step;

  unsigned int seed_value = _step > 0 ? (_step - 1) : 0;

  // Filling the new_samples vector of vectors with new proposal samples
  for (unsigned int j = 0; j < _num_parallel_proposals; ++j)
  {
    for (unsigned int i = 0; i < _priors.size(); ++i)
      _new_samples[j][i] = _priors[i]->quantile(getRand(seed_value));
    _rnd_vec[j] = getRand(seed_value);
  }
}

void
ParallelMarkovChainMonteCarloBase::randomIndex(const unsigned int & ub, const unsigned int & exclude, const unsigned int & seed, unsigned int & req_index)
{
  req_index = exclude;
  while (req_index == exclude)
    req_index = getRandl(seed, 0, ub);
}

void
ParallelMarkovChainMonteCarloBase::randomIndex2(const unsigned int & ub, const unsigned int & exclude, const unsigned int & seed, unsigned int & req_index1, unsigned int & req_index2)
{
  randomIndex(ub, exclude, seed, req_index1);
  req_index2 = req_index1;
  while (req_index1 == req_index2)
    randomIndex(ub, exclude, seed, req_index2);
}

void
ParallelMarkovChainMonteCarloBase::combineWithConfg()
{
  unsigned int index1;
  int index2 = -1;
  std::vector<Real> tmp;
  for (unsigned int i = 0; i < _num_parallel_proposals * _confg_values.size(); ++i)
  {
    index1 = i % _num_parallel_proposals;
    index2 = (index1 == 0) ? ++index2 : index2;
    tmp = _new_samples[index1];
    tmp.push_back(_confg_values[index2]);
    _new_samples_confg[i] = tmp;
  }
}

const std::vector<Real> &
ParallelMarkovChainMonteCarloBase::getRandomNumbers() const
{
  return _rnd_vec;
}

Real
ParallelMarkovChainMonteCarloBase::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  combineWithConfg();
  return _new_samples_confg[row_index][col_index];
}
