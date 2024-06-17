//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AffineInvariantDESGPry.h"
#include "AdaptiveMonteCarloUtils.h"
#include "Uniform.h"
#include "DelimitedFileReader.h"

registerMooseObject("StochasticToolsApp", AffineInvariantDESGPry);

InputParameters
AffineInvariantDESGPry::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Affine invariant differential evolution sampler with GPry.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "prior_distributions", "The prior distributions of the parameters to be calibrated.");
  params.addParam<DistributionName>(
      "prior_variance", "The prior distribution of the variance parameter to be calibrated.");
  params.addRequiredParam<unsigned int>(
      "num_parallel_proposals",
      "Number of proposals to make and corresponding subApps executed in "
      "parallel.");
  params.addParam<std::vector<Real>>("lower_bound", "Lower bounds for making the next proposal.");
  params.addParam<std::vector<Real>>("upper_bound", "Upper bounds for making the next proposal.");
  params.addParam<Real>("variance_bound", "Upper bound for variance for making the next proposal.");
  params.addRequiredParam<std::vector<Real>>("initial_values",
                                             "The starting values of the inputs to be calibrated.");
  params.addParam<unsigned int>(
      "num_random_seeds",
      100000,
      "Initialize a certain number of random seeds. Change from the default only if you have to.");
  params.addRequiredParam<ReporterName>(
      "previous_state", "Reporter value with the previous state of all the walkers.");
  params.addRequiredParam<ReporterName>(
      "previous_state_var",
      "Reporter value with the previous state of all the walkers for variance.");
  MooseEnum tuning_option("Braak2006_static", "Braak2006_static");
  params.addParam<MooseEnum>(
      "tuning_option", tuning_option, "The tuning option for internal parameters.");
  params.addParam<std::vector<Real>>("scales", "Scales for the parameters.");
//   params.addRequiredParam<FileName>("starting_inputs", "Name of the CSV file with starting input values.");
  return params;
}

AffineInvariantDESGPry::AffineInvariantDESGPry(const InputParameters & parameters)
  : Sampler(parameters),
    TransientInterface(this),
    _num_parallel_proposals(getParam<unsigned int>("num_parallel_proposals")),
    _lower_bound(isParamValid("lower_bound") ? &getParam<std::vector<Real>>("lower_bound")
                                             : nullptr),
    _upper_bound(isParamValid("upper_bound") ? &getParam<std::vector<Real>>("upper_bound")
                                             : nullptr),
    _variance_bound(isParamValid("variance_bound") ? &getParam<Real>("variance_bound") : nullptr),
    _check_step(0),
    _initial_values(getParam<std::vector<Real>>("initial_values")),
    _previous_state(getReporterValue<std::vector<std::vector<Real>>>("previous_state")),
    _previous_state_var(getReporterValue<std::vector<Real>>("previous_state_var")),
    _tuning_option(getParam<MooseEnum>("tuning_option")),
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

  // Setting the number of sampler rows to be equal to the number of parallel proposals
  setNumberOfRows(_num_parallel_proposals);

  // Setting the number of columns in the sampler matrix (equal to the number of distributions).
  setNumberOfCols(_priors.size());

  // Resizing the vectors and vector of vectors
  _new_samples.resize(_num_parallel_proposals, std::vector<Real>(_priors.size(), 0.0));
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

  if (_num_parallel_proposals < 5)
    paramError(
        "num_parallel_proposals",
        "At least five parallel proposals should be used for the Differential Evolution Sampler.");

  if (_num_parallel_proposals < _priors.size())
    mooseWarning(
        "It is recommended that the parallel proposals be greater than or equal to the "
        "inferred parameters. This will allow the sampler to not get stuck on a hyper-plane.");

  if (isParamValid("scales"))
  {
    _scales = getParam<std::vector<Real>>("scales");
    if (_scales.size() != _priors.size())
      paramError("scales",
                 "The number of scales provided should match the number of tunable params.");
  }
  else
    _scales.assign(_priors.size(), 1.0);

  // Read the experimental configurations from a csv file
//   MooseUtils::DelimitedFileReader reader(getParam<FileName>("starting_inputs"));
//   reader.read();
//   _starting_values.resize(_priors.size());
//   for (unsigned int i = 0; i < _priors.size(); ++i)
//     _starting_values[i] = reader.getData(i);
}

void
AffineInvariantDESGPry::computeDifferential(
    const Real & state1, const Real & state2, const Real & rnd, const Real & scale, Real & diff)
{
  Real gamma;
  Real b;
  tuneParams(gamma, b, scale);
  diff = gamma * (state1 - state2) + Normal::quantile(rnd, 0.0, b);
}

void
AffineInvariantDESGPry::tuneParams(Real & gamma, Real & b, const Real & scale)
{
  if (_tuning_option == "Braak2006_static")
  {
    gamma = 2.38 / std::sqrt(2 * _priors.size());
    b = 1e-6 * scale;
  }
}

void
AffineInvariantDESGPry::proposeSamples(const unsigned int seed_value)
{
  unsigned int j = 0;
  bool indicator;
  unsigned int index_req1, index_req2;
  Real diff;
  while (j < _num_parallel_proposals)
  {
    indicator = 0;
    randomIndexPair(_num_parallel_proposals, j, seed_value, index_req1, index_req2);
    for (unsigned int i = 0; i < _priors.size(); ++i)
    {
      computeDifferential(_previous_state[index_req1][i],
                          _previous_state[index_req2][i],
                          getRand(seed_value),
                          _scales[i],
                          diff);
      _new_samples[j][i] =
          (_t_step > decisionStep())
              ? (_previous_state[j][i] + diff)
              : _priors[i]->quantile(getRand(seed_value)); // _starting_values[i][j]
      if (_lower_bound)
        indicator =
            (_new_samples[j][i] < (*_lower_bound)[i] || _new_samples[j][i] > (*_upper_bound)[i])
                ? 1
                : indicator;
    }
    if (_var_prior)
    {
      computeDifferential(_previous_state_var[index_req1],
                          _previous_state_var[index_req2],
                          getRand(seed_value),
                          1.0,
                          diff);
      _new_var_samples[j] = (_t_step > decisionStep())
                                ? (_previous_state_var[j] + diff)
                                : _var_prior->quantile(getRand(seed_value));
      if (_new_var_samples[j] < 0.0)
        indicator = 1;
      if (_variance_bound)
        indicator = (_new_var_samples[j] > (*_variance_bound)) ? 1 : indicator;
    }
    if (!indicator)
      ++j;
  }
}

void
AffineInvariantDESGPry::sampleSetUp(const SampleMode /*mode*/)
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
AffineInvariantDESGPry::randomIndex(const unsigned int & upper_bound,
                       const unsigned int & exclude,
                       const unsigned int & seed,
                       unsigned int & req_index)
{
  req_index = exclude;
  while (req_index == exclude)
    req_index = getRandl(seed, 0, upper_bound);
}

void
AffineInvariantDESGPry::randomIndexPair(const unsigned int & upper_bound,
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

const std::vector<Real> &
AffineInvariantDESGPry::getRandomNumbers() const
{
  return _rnd_vec;
}

const std::vector<Real> &
AffineInvariantDESGPry::getVarSamples() const
{
  return _new_var_samples;
}

const std::vector<std::vector<Real>> &
AffineInvariantDESGPry::getSamples() const
{
  return _new_samples;
}

const std::vector<const Distribution *>
AffineInvariantDESGPry::getPriors() const
{
  return _priors;
}

const Distribution *
AffineInvariantDESGPry::getVarPrior() const
{
  return _var_prior;
}

Real
AffineInvariantDESGPry::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  if (_t_step < 1)
    for (unsigned int i = 0; i < _num_parallel_proposals; ++i)
      _new_samples[i] = _initial_values;

  return _new_samples[row_index][col_index];
}
