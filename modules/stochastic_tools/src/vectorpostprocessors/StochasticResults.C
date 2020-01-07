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

  MooseEnum parallel_type("REPLICATED DISTRIBUTED", "REPLICATED");
  params.addParam<MooseEnum>(
      "parallel_type",
      parallel_type,
      "Specify if the stored data vector is replicated or distributed across processors.");

  params.addParam<processor_id_type>(
      "output_distributed_rank",
      Moose::INVALID_PROCESSOR_ID,
      "When 'parallel_type = DISTRIBUTED' set this to copy the data from the specified processor "
      "for output. This is mainly for testing since the data from that rank will override the data "
      "on the root process.");

  params.set<bool>("_is_broadcast") = false;
  return params;
}

StochasticResults::StochasticResults(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    SamplerInterface(this),
    _parallel_type(getParam<MooseEnum>("parallel_type")),
    _output_distributed_rank(getParam<processor_id_type>("output_distributed_rank"))
{

  if (_output_distributed_rank != Moose::INVALID_PROCESSOR_ID)
  {
    if (_parallel_type == "replicated")
      paramError("output_distributed_rank",
                 "The output rank cannot be used with 'parallel_type' set to replicated.");
    else if (_output_distributed_rank >= n_processors())
      paramError("output_distributed_rank",
                 "The supplied value is greater than the number of available processors: ",
                 _output_distributed_rank);
  }
  else
  {
    if ((_parallel_type == "DISTRIBUTED") && (getOutputs().count("none") == 0) &&
        (n_processors() > 1))
      paramWarning("parallel_type",
                   "The parallel_type was set to DISTRIBUTED and output is enabled for the object, "
                   "when running in parallel the results output will only contain the data on the "
                   "root processor. Output can be disabled by setting 'outputs = none' in the "
                   "input block. If output is desired the 'output_distributed_rank' can be set.");
  }

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
  if (_parallel_type == "REPLICATED")
  {
    for (auto & data : _sample_vectors)
      _communicator.gather(0, data.current);
  }

  else if (_output_distributed_rank != 0 && _output_distributed_rank != Moose::INVALID_PROCESSOR_ID)
  {
    if (processor_id() == _output_distributed_rank)
      for (auto & data : _sample_vectors)
        _communicator.send(0, data.current);

    else if (processor_id() == 0)
      for (auto & data : _sample_vectors)
        _communicator.receive(_output_distributed_rank, data.current);
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
