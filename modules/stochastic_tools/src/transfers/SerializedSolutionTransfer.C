//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// StochasticTools includes
#include "SerializedSolutionTransfer.h"
#include "NonlinearSystemBase.h"
#include "Sampler.h"

registerMooseObject("StochasticToolsApp", SerializedSolutionTransfer);

InputParameters
SerializedSolutionTransfer::validParams()
{
  InputParameters params = StochasticToolsTransfer::validParams();
  params.addClassDescription("Noice.");
  params.addRequiredParam<std::string>("parallel_storage_name", "Something here.");
  //  params.suppressParameter<MultiAppName>("to_multi_app");
  return params;
}

SerializedSolutionTransfer::SerializedSolutionTransfer(const InputParameters & parameters)
  : StochasticToolsTransfer(parameters)

{
}

void
SerializedSolutionTransfer::initialSetup()
{
  std::string parallel_storage_name = getParam<std::string>("parallel_storage_name");

  std::vector<UserObject *> reporters;
  _fe_problem.theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribName>(parallel_storage_name)
      .queryInto(reporters);

  if (reporters.empty())
    paramError(
        "parallel_storage_name", "Unable to find reporter with name '", parallel_storage_name, "'");
  else if (reporters.size() > 1)
    paramError("parallel_storage_name",
               "We found more than one reporter with the name '",
               parallel_storage_name,
               "'");

  _parallel_storage = dynamic_cast<ParallelSolutionStorage *>(reporters[0]);

  if (!_parallel_storage)
    paramError("parallel_storage_name",
               "The parallel storage reporter is not of type '",
               parallel_storage_name,
               "'");
}

void
SerializedSolutionTransfer::execute()
{
}

void
SerializedSolutionTransfer::initializeFromMultiapp()
{
}

void
SerializedSolutionTransfer::executeFromMultiapp()
{
}

void
SerializedSolutionTransfer::finalizeFromMultiapp()
{
}

void
SerializedSolutionTransfer::initializeToMultiapp()
{
}

void
SerializedSolutionTransfer::executeToMultiapp()
{
}

void
SerializedSolutionTransfer::finalizeToMultiapp()
{
}
