//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PMCMCBase.h"
#include "AdaptiveMonteCarloUtils.h"
#include "Uniform.h"
#include "DelimitedFileReader.h"

registerMooseObject("StochasticToolsApp", PMCMCBase);

InputParameters
PMCMCBase::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Parallel Markov chain Monte Carlo base.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "prior_distributions", "The prior distributions of the parameters to be calibrated.");
  params.addParam<DistributionName>(
      "prior_variance", "The prior distribution of the variance parameter to be calibrated.");
  params.addRequiredParam<unsigned int>(
      "num_parallel_proposals",
      "Number of proposals to make and corresponding subApps executed in "
      "parallel.");
  params.addRequiredParam<FileName>("file_name", "Name of the CSV file with configuration values.");
  params.addParam<std::string>(
      "file_column_name", "Name of column in CSV file to use, by default first column is used.");
  params.addParam<unsigned int>(
      "num_columns", "Number of columns to be used in the CSV file with the configuration values.");
  params.addParam<std::vector<Real>>("lower_bound", "Lower bounds for making the next proposal.");
  params.addParam<std::vector<Real>>("upper_bound", "Upper bounds for making the next proposal.");
  params.addRequiredParam<std::vector<Real>>("initial_values",
                                             "The starting values of the inputs to be calibrated.");
  params.addParam<unsigned int>(
      "num_random_seeds",
      100000,
      "Initialize a certain number of random seeds. Change from the default only if you have to.");
  return params;
}

PMCMCBase::PMCMCBase(const InputParameters & parameters)
  : Sampler(parameters),
    TransientInterface(this),
    _num_parallel_proposals(getParam<unsigned int>("num_parallel_proposals")),
    _lower_bound(isParamValid("lower_bound") ? &getParam<std::vector<Real>>("lower_bound")
                                             : nullptr),
    _upper_bound(isParamValid("upper_bound") ? &getParam<std::vector<Real>>("upper_bound")
                                             : nullptr),
    _check_step(0),
    _initial_values(getParam<std::vector<Real>>("initial_values")),
    _num_random_seeds(getParam<unsigned int>("num_random_seeds"))
{
  // Filling the `priors` vector with the user-provided distributions.
  for (const DistributionName & name :
       getParam<std::vector<DistributionName>>("prior_distributions"))
    _priors.push_back(&getDistributionByName(name));

  // Filling the `var_prior` object with the user-provided distribution for the variance.
  if (isParamValid("prior_variance"))
    _var_prior = &getDistributionByName(getParam<DistributionName>("prior_variance"));
  else
    _var_prior = nullptr;

  // Read the experimental configurations from a csv file
  MooseUtils::DelimitedFileReader reader(getParam<FileName>("file_name"));
  reader.read();
  _confg_values.resize(1);
  if (isParamValid("file_column_name"))
    _confg_values[0] = reader.getData(getParam<std::string>("file_column_name"));
  else if (isParamValid("num_columns"))
  {
    _confg_values.resize(getParam<unsigned int>("num_columns"));
    for (unsigned int i = 0; i < _confg_values.size(); ++i)
      _confg_values[i] = reader.getData(i);
  }
  else
    _confg_values[0] = reader.getData(0);

  // Setting the number of sampler rows to be equal to the number of parallel proposals
  setNumberOfRows(_num_parallel_proposals * _confg_values[0].size());

  // Setting the number of columns in the sampler matrix (equal to the number of distributions).
  setNumberOfCols(_priors.size() + _confg_values.size());

  // Resizing the vectors and vector of vectors
  _new_samples.resize(_num_parallel_proposals, std::vector<Real>(_priors.size(), 0.0));
  _new_samples_confg.resize(_num_parallel_proposals * _confg_values[0].size(),
                            std::vector<Real>(_priors.size() + _confg_values.size(), 0.0));
  _rnd_vec.resize(_num_parallel_proposals);
  _new_var_samples.assign(_num_parallel_proposals, 0.0);

  setNumberOfRandomSeeds(_num_random_seeds);

  _check_step = 0;

  // Check whether both the lower and the upper bounds are specified and of same size
  bool bound_check1 = _lower_bound && !_upper_bound;
  bool bound_check2 = !_lower_bound && _upper_bound;
  if (bound_check1 || bound_check2)
    mooseError("Both lower and upper bounds should be specified.");
  bool size_check = _lower_bound ? ((*_lower_bound).size() != (*_upper_bound).size()) : 0;
  if (size_check)
    mooseError("Lower and upper bounds should be of the same size.");

  // Check whether the priors, bounds, and initial values are all of the same size
  if (_priors.size() != _initial_values.size())
    mooseError("The priors and initial values should be of the same size.");
}

void
PMCMCBase::proposeSamples(const unsigned int seed_value)
{
  for (unsigned int j = 0; j < _num_parallel_proposals; ++j)
    for (unsigned int i = 0; i < _priors.size(); ++i)
      _new_samples[j][i] = _priors[i]->quantile(getRand(seed_value));
}

void
PMCMCBase::sampleSetUp(const SampleMode /*mode*/)
{
  if (_t_step < 1 || _check_step == _t_step)
    return;
  _check_step = _t_step;

  unsigned int seed_value = _t_step > 0 ? (_t_step - 1) : 0;

  // Filling the new_samples vector of vectors with new proposal samples
  proposeSamples(seed_value);

  // Draw random numbers to facilitate decision making later on
  for (unsigned int j = 0; j < _num_parallel_proposals; ++j)
    _rnd_vec[j] = getRand(seed_value);
}

void
PMCMCBase::randomIndex(const unsigned int & upper_bound,
                       const unsigned int & exclude,
                       const unsigned int & seed,
                       unsigned int & req_index)
{
  req_index = exclude;
  while (req_index == exclude)
    req_index = getRandl(seed, 0, upper_bound);
}

void
PMCMCBase::randomIndexPair(const unsigned int & upper_bound,
                           const unsigned int & exclude,
                           const unsigned int & seed,
                           unsigned int & req_index1,
                           unsigned int & req_index2)
{
  randomIndex(upper_bound, exclude, seed, req_index1);
  req_index2 = req_index1;
  while (req_index1 == req_index2)
    randomIndex(upper_bound, exclude, seed, req_index2);
}

void
PMCMCBase::combineWithExperimentalConfig()
{
  unsigned int index1;
  int index2 = -1;
  std::vector<Real> tmp;
  for (unsigned int i = 0; i < _num_parallel_proposals * _confg_values[0].size(); ++i)
  {
    index1 = i % _num_parallel_proposals;
    if (index1 == 0)
      ++index2;
    tmp = _new_samples[index1];
    for (unsigned int j = 0; j < _confg_values.size(); ++j)
      tmp.push_back(_confg_values[j][index2]);
    _new_samples_confg[i] = tmp;
  }
}

const std::vector<Real> &
PMCMCBase::getRandomNumbers() const
{
  return _rnd_vec;
}

const std::vector<Real> &
PMCMCBase::getVarSamples() const
{
  return _new_var_samples;
}

const std::vector<const Distribution *>
PMCMCBase::getPriors() const
{
  return _priors;
}

const Distribution *
PMCMCBase::getVarPrior() const
{
  return _var_prior;
}

Real
PMCMCBase::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  if (_t_step < 1)
    for (unsigned int i = 0; i < _num_parallel_proposals; ++i)
      _new_samples[i] = _initial_values;

  // Combine the proposed samples with experimental configurations
  combineWithExperimentalConfig();

  return _new_samples_confg[row_index][col_index];
}
