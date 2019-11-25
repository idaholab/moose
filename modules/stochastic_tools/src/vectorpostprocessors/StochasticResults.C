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

InputParameters
StochasticResults::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription(
      "Storage container for stochastic simulation results coming from a Postprocessor.");
  params += SamplerInterface::validParams();

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
}

void
StochasticResults::init(Sampler & sampler)
{
  _sampler = &sampler;
  _sample_vector = &declareVector(_sampler->name());
}

void
StochasticResults::finalize()
{
  if (_parallel_type == "REPLICATED")
    _communicator.gather(0, *_sample_vector);

  else if (_output_distributed_rank != 0 && _output_distributed_rank != Moose::INVALID_PROCESSOR_ID)
  {
    if (processor_id() == _output_distributed_rank)
      _communicator.send(0, *_sample_vector);

    else if (processor_id() == 0)
      _communicator.receive(_output_distributed_rank, *_sample_vector);
  }
}
