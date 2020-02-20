//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Stocastic Tools Includes
#include "StochasticResults.h"

// MOOSE includes
#include "Sampler.h"

registerMooseObject("StochasticToolsApp", StochasticResults);

defineLegacyParams(StochasticResults);

StochasticResultsData::StochasticResultsData(const VectorPostprocessorName & name,
                                             VectorPostprocessorValue * vpp)
  : name(name), vector(vpp)
{
}

InputParameters
StochasticResults::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription(
      "Storage container for stochastic simulation results coming from a Postprocessor.");
  params += SamplerInterface::validParams();

  params.addParam<std::vector<SamplerName>>("samplers",
                                            "A list of sampler names of associated data.");
  params.set<bool>("_auto_broadcast") = false;
  return params;
}

StochasticResults::StochasticResults(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters), SamplerInterface(this)
{
  if (isParamValid("samplers"))
    for (const SamplerName & name : getParam<std::vector<SamplerName>>("samplers"))
    {
      Sampler & sampler = getSamplerByName(name);
      _sample_vectors.emplace_back(sampler.name(), &declareVector(sampler.name()));
    }
}

void
StochasticResults::initialize()
{
  // Clear any existing data, unless the complete history is desired
  if (!containsCompleteHistory())
    for (auto & data : _sample_vectors)
      data.vector->clear();
}

void
StochasticResults::finalize()
{
  if (!isDistributed())
  {
    for (auto & data : _sample_vectors)
      _communicator.gather(0, data.current);
  }

  for (auto & data : _sample_vectors)
  {
    data.vector->insert(data.vector->end(), data.current.begin(), data.current.end());
    data.current.clear();
  }
}

void
StochasticResults::setCurrentLocalVectorPostprocessorValue(
    const std::string & name, const VectorPostprocessorValue && current)
{
  mooseAssert(!hasVectorPostprocessorByName(name),
              "The supplied name must be a valid vector postprocessor name.");
  auto data_ptr = std::find_if(_sample_vectors.begin(),
                               _sample_vectors.end(),
                               [&name](StochasticResultsData & data) { return data.name == name; });

  data_ptr->current = current;
}

// DEPRECATED
void
StochasticResults::init(Sampler & sampler)
{
  if (!isParamValid("samplers"))
  {
    paramWarning("samplers",
                 "Support for the 'StochasticResults' objects without the 'samplers' input "
                 "parameter is being removed, please update your input file(s).");
    _sample_vectors.emplace_back(sampler.name(), &declareVector(sampler.name()));
  }
}
