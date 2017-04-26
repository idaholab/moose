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

#include "Sampler.h"
#include "Distribution.h"
#include <limits>

template <>
InputParameters
validParams<Sampler>()
{
  InputParameters params = validParams<MooseObject>();
  params += validParams<RandomInterface>();
  params.addParam<bool>("reseed", false, "Reseed for each new sample if this value is true");
  params.addRequiredParam<std::vector<std::string>>(
      "perturb_parameters", "The names of the parameters that you want to perturb");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions",
      "The names of distributions that you want to use to perturb the given parameters");
  params.registerBase("Sampler");
  return params;
}

Sampler::Sampler(const InputParameters & parameters)
  : MooseObject(parameters),
    RandomInterface(parameters,
                    *parameters.get<FEProblemBase *>("_fe_problem_base"),
                    parameters.get<THREAD_ID>("_tid"),
                    false),
    DistributionInterface(this),
    Restartable(parameters, "Samplers"),
    _tid(getParam<THREAD_ID>("_tid")),
    _reseed_for_new_sample(getParam<bool>("reseed")),
    _dist_names(getParam<std::vector<DistributionName>>("distributions")),
    _var_names(getParam<std::vector<std::string>>("perturb_parameters")),
    _current_sample(0),
    _failed_runs(true)
{
  _var_dist_map.clear();
  _probability_weight.clear();
  _var_value_hist.clear();
  if (_var_names.size() != _dist_names.size())
    mooseError("The size of perturb_parameters (",
               _var_names.size(),
               ") != the size of distributions (",
               _dist_names.size(),
               ").");
  if (!_var_names.empty())
  {
    for (unsigned int i = 0; i < _var_names.size(); ++i)
    {
      _var_dist_map[_var_names[i]] = &getDistributionByName(_dist_names[i]);
      _var_value_map[_var_names[i]] = 0.0;
      _var_value_hist[_var_names[i]] = std::vector<Real>();
    }
  }
}

void
Sampler::generateSamples()
{
}

std::vector<std::string>
Sampler::getSampledVariableNames()
{
  return _var_names;
}

std::vector<Real>
Sampler::getSampledValues(const std::vector<std::string> & variableNames)
{
  std::vector<Real> vals;
  for (auto & var_name : variableNames)
  {
    Real val = getSampledValue(var_name);
    vals.push_back(val);
  }
  return vals;
}

Real
Sampler::getSampledValue(const std::string & variableName)
{
  std::map<std::string, Real>::iterator it;
  it = _var_value_map.find(variableName);
  if (it == _var_value_map.end())
    mooseError(
        "Could not find the parameter: ", variableName, " in the list of perturbed parameters!");
  return it->second;
}

std::vector<Real>
Sampler::getProbabilityWeights()
{
  return _probability_weight;
}

bool
Sampler::checkRuns()
{
  return _failed_runs;
}
