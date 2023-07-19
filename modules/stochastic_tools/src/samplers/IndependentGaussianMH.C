//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IndependentGaussianMH.h"
#include "Normal.h"
#include "TruncatedNormal.h"

registerMooseObject("StochasticToolsApp", IndependentGaussianMH);

InputParameters
IndependentGaussianMH::validParams()
{
  InputParameters params = PMCMCBase::validParams();
  params.addClassDescription("Perform M-H MCMC sampling with independent Gaussian propoposals.");
  params.addRequiredParam<ReporterName>("seed_inputs",
                                        "Reporter with seed inputs values for the next proposals.");
  params.addRequiredParam<std::vector<Real>>("std_prop",
                                             "Standard deviations for making the next proposal.");
  return params;
}

IndependentGaussianMH::IndependentGaussianMH(const InputParameters & parameters)
  : PMCMCBase(parameters),
    _seed_inputs(getReporterValue<std::vector<Real>>("seed_inputs")),
    _std_prop(getParam<std::vector<Real>>("std_prop"))
{
  // Error check for sizes of proposal stds
  if (_std_prop.size() != _priors.size())
    paramError("std_prop",
               "The number of proposal stds, initial values, and priors should be the same.");
}

void
IndependentGaussianMH::proposeSamples(const unsigned int seed_value)
{
  std::vector<Real> old_sample = (_t_step > decisionStep()) ? _seed_inputs : _initial_values;
  for (unsigned int j = 0; j < _num_parallel_proposals; ++j)
    for (unsigned int i = 0; i < _priors.size(); ++i)
    {
      if (_lower_bound)
        _new_samples[j][i] = TruncatedNormal::quantile(getRand(seed_value),
                                                       old_sample[i],
                                                       _std_prop[i],
                                                       (*_lower_bound)[i],
                                                       (*_upper_bound)[i]);
      else
        _new_samples[j][i] = Normal::quantile(getRand(seed_value), old_sample[i], _std_prop[i]);
    }
}
