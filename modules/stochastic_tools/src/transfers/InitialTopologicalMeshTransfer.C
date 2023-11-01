//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "InitialTopologicalMeshTransfer.h"
#include "SamplerTransientMultiApp.h"
#include "SamplerFullSolveMultiApp.h"
#include "Sampler.h"
#include "MooseUtils.h"
#include "pcrecpp.h"
#include "TopologyOptimizationSampler.h"

registerMooseObject("StochasticToolsApp", InitialTopologicalMeshTransfer);

InputParameters
InitialTopologicalMeshTransfer::validParams()
{
  InputParameters params = StochasticToolsTransfer::validParams();
  params.addClassDescription("This transfer initializes the TopologyOptimizationSampler with the "
                             "initial guess of the geometry of the physics problem.");
  params.suppressParameter<MultiMooseEnum>("direction");
  params.suppressParameter<MultiAppName>("multi_app");
  params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;
  params.suppressParameter<ExecFlagEnum>("execute_on");
  return params;
}

InitialTopologicalMeshTransfer::InitialTopologicalMeshTransfer(const InputParameters & parameters)
  : StochasticToolsTransfer(parameters), _top_opt_sampler(nullptr)
{
  if (isParamValid("to_multi_app"))
    mooseError("Transfers from subapp to main app");
  _top_opt_sampler = dynamic_cast<TopologyOptimizationSampler *>(_sampler_ptr);
  if (!_top_opt_sampler)
    mooseError("Sampler must be of type TopologyOptimizationSampler");

  if (getFromMultiApp()->numLocalApps() > 1)
    mooseError("There should be at most one local app per processor.");
}

void
InitialTopologicalMeshTransfer::execute()
{
  if (getFromMultiApp()->hasApp())
    _top_opt_sampler->setParametersFromSubapp(_from_meshes[0]);
}
