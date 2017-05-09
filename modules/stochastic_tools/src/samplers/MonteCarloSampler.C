/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "MonteCarloSampler.h"

template <>
InputParameters
validParams<MonteCarloSampler>()
{
  InputParameters params = validParams<Sampler>();
  params.addRequiredParam<unsigned int>("n_samples", "Number of samples for Monte Carlo Sampling");
  params.addClassDescription("Monte Carlo Sampler.");
  return params;
}

MonteCarloSampler::MonteCarloSampler(const InputParameters & parameters)
  : Sampler(parameters), _num_samples(getParam<unsigned int>("n_samples"))
{
  /// initial samples of perturbed parameters
  generateSamples();
}

MonteCarloSampler::~MonteCarloSampler() {}

void
MonteCarloSampler::generateSamples()
{
  _current_sample += 1;
  Real weight = 1.0;
  _var_value_map.clear();
  for (auto & var_name : _var_names)
  {
    if (_reseed_for_new_sample)
    {
      unsigned int randomSeed = getRandomLong();
      _var_dist_map[var_name]->setSeed(randomSeed);
    }
    Real val = _var_dist_map[var_name]->getRandomNumber();
    _var_value_map[var_name] = val;
    if (_var_value_hist.find(var_name) != _var_value_hist.end())
      _var_value_hist[var_name].push_back(val);
    else
      mooseError("The variable: ",
                 var_name,
                 " is not found in the predetermined map, i.e. _var_value_hist");
  }
  /// The probability weight for Monte Carlo sampler is constant
  /// This should be normalized before any data analysis of outputs
  _probability_weight.push_back(weight);
}
