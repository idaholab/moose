//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AffineInvariantDES.h"
#include "Normal.h"
#include "Uniform.h"

registerMooseObject("StochasticToolsApp", AffineInvariantDES);

/*
 Tuning options for the internal parameters
  1. Braak2006_static:
  - the gamma param is set to 2.38 / sqrt(2 * dim)
  - the b param is set to (scale) * 1e-6
*/

InputParameters
AffineInvariantDES::validParams()
{
  InputParameters params = PMCMCBase::validParams();
  params.addClassDescription("Perform Affine Invariant Ensemble MCMC with differential sampler.");
  params.addRequiredParam<ReporterName>(
      "previous_state", "Reporter value with the previous state of all the walkers.");
  params.addRequiredParam<ReporterName>(
      "previous_state_var",
      "Reporter value with the previous state of all the walkers for variance.");
  MooseEnum tuning_option("Braak2006_static", "Braak2006_static");
  params.addParam<MooseEnum>(
      "tuning_option", tuning_option, "The tuning option for internal parameters.");
  params.addParam<std::vector<Real>>("scales", "Scales for the parameters.");
  return params;
}

AffineInvariantDES::AffineInvariantDES(const InputParameters & parameters)
  : PMCMCBase(parameters),
    _previous_state(getReporterValue<std::vector<std::vector<Real>>>("previous_state")),
    _previous_state_var(getReporterValue<std::vector<Real>>("previous_state_var")),
    _tuning_option(getParam<MooseEnum>("tuning_option"))
{
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
}

void
AffineInvariantDES::computeDifferential(
    const Real & state1, const Real & state2, const Real & rnd, const Real & scale, Real & diff)
{
  Real gamma;
  Real b;
  tuneParams(gamma, b, scale);
  diff = gamma * (state1 - state2) + Normal::quantile(rnd, 0.0, b);
}

void
AffineInvariantDES::tuneParams(Real & gamma, Real & b, const Real & scale)
{
  if (_tuning_option == "Braak2006_static")
  {
    gamma = 2.38 / std::sqrt(2 * _priors.size());
    b = 1e-6 * scale;
  }
}

void
AffineInvariantDES::proposeSamples(const unsigned int seed_value)
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
      _new_samples[j][i] = (_t_step > decisionStep()) ? (_previous_state[j][i] + diff)
                                                      : _priors[i]->quantile(getRand(seed_value));
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
      _new_var_samples[j] = (_t_step > decisionStep()) ? (_previous_state_var[j] + diff)
                                                       : _var_prior->quantile(getRand(seed_value));
      if (_new_var_samples[j] < 0.0)
        indicator = 1;
    }
    if (!indicator)
      ++j;
  }
}
