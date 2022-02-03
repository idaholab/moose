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
#include "SamplerPostprocessorTransfer.h"

registerMooseObject("StochasticToolsApp", StochasticResults);

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

  params.addDeprecatedParam<std::vector<SamplerName>>(
      "samplers",
      "A list of sampler names of associated data.",
      "This parameter is no longer needed, please remove it from your input file.");

  // If 'parallel_type = REPLICATED' broadcast the vector automatically
  params.set<bool>("_auto_broadcast") = true;
  return params;
}

StochasticResults::StochasticResults(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters)
{
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
    const std::string & vector_name, const VectorPostprocessorValue && current)
{
  auto data_ptr = std::find_if(_sample_vectors.begin(),
                               _sample_vectors.end(),
                               [&vector_name](StochasticResultsData & data)
                               { return data.name == vector_name; });

  mooseAssert(data_ptr != _sample_vectors.end(),
              "Unable to locate a vector with the supplied name of '" << vector_name << "'.");
  data_ptr->current = current;
}

void
StochasticResults::initVector(const std::string & vector_name)
{
  _sample_vectors.emplace_back(vector_name, &declareVector(vector_name));
}
