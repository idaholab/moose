//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdaptiveSamplerAction.h"
#include "AddSamplerAction.h"
#include "FEProblem.h"

registerMooseAction("StochasticToolsApp", AdaptiveSamplerAction, "add_user_object");
registerMooseAction("StochasticToolsApp", AdaptiveSamplerAction, "add_postprocessor");

InputParameters
AdaptiveSamplerAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Adds extra objects pertaining to adaptive samplers.");
  return params;
}

AdaptiveSamplerAction::AdaptiveSamplerAction(const InputParameters & params) : Action(params) {}

void
AdaptiveSamplerAction::act()
{
  const auto samplers = _app.actionWarehouse().getActions<AddSamplerAction>();
  SamplerName sampler_name;

  // determine whether the sampler is an adaptive sampler
  for (auto & sampler : samplers)
    if (adaptiveSamplerNames().count(sampler->getMooseObjectType()) > 0)
    {
      if (!sampler_name.empty())
        mooseError("The case with multiple adaptive samplers is not currently supported");
      sampler_name = sampler->name();
    }

  if (sampler_name.empty())
    return;

  if (_current_task == "add_user_object")
  {
    InputParameters params = _factory.getValidParams("Terminator");
    params.set<std::string>("expression") = adaptiveSamplingCompletedPostprocessorName() + " > 0";
    params.set<std::string>("message") = "Sampling completed!";
    params.set<ExecFlagEnum>("execute_on") = {EXEC_TIMESTEP_END};
    _problem->addUserObject("Terminator", "_terminate" + sampler_name, params);
  }
  else if (_current_task == "add_postprocessor")
  {
    InputParameters params = _factory.getValidParams("AdaptiveSamplingCompletedPostprocessor");
    params.set<SamplerName>("sampler") = sampler_name;
    params.set<ExecFlagEnum>("execute_on") = {EXEC_TIMESTEP_END};
    params.set<std::vector<OutputName>>("outputs") = {"none"};
    _problem->addUserObject("AdaptiveSamplingCompletedPostprocessor",
                            adaptiveSamplingCompletedPostprocessorName(),
                            params);
  }
}
