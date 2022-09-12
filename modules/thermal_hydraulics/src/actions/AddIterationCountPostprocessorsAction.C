//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddIterationCountPostprocessorsAction.h"
#include "MooseObjectAction.h"
#include "ActionFactory.h"
#include "ActionWarehouse.h"

registerMooseAction("ThermalHydraulicsApp", AddIterationCountPostprocessorsAction, "meta_action");

InputParameters
AddIterationCountPostprocessorsAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addParam<bool>(
      "count_iterations", false, "Add postprocessors for linear and nonlinear iterations");
  params.addClassDescription("Adds postprocessors for linear and nonlinear iterations");
  return params;
}

AddIterationCountPostprocessorsAction::AddIterationCountPostprocessorsAction(
    const InputParameters & parameters)
  : Action(parameters), _add_pps(getParam<bool>("count_iterations"))
{
}

void
AddIterationCountPostprocessorsAction::act()
{
  if (_add_pps)
  {
    const std::vector<std::string> it_per_step_class_names = {"NumLinearIterations",
                                                              "NumNonlinearIterations"};
    const std::vector<std::string> it_per_step_names = {"num_linear_iterations_per_step",
                                                        "num_nonlinear_iterations_per_step"};
    const std::vector<std::string> total_it_names = {"num_linear_iterations",
                                                     "num_nonlinear_iterations"};

    for (unsigned int i = 0; i < it_per_step_class_names.size(); i++)
    {
      // iterations per time step
      {
        const std::string class_name = "AddPostprocessorAction";
        InputParameters action_params = _action_factory.getValidParams(class_name);
        action_params.set<std::string>("type") = it_per_step_class_names[i];
        auto action = std::static_pointer_cast<MooseObjectAction>(
            _action_factory.create(class_name, it_per_step_names[i], action_params));
        _awh.addActionBlock(action);
      }
      // cumulative iterations
      {
        const std::string class_name = "AddPostprocessorAction";
        InputParameters action_params = _action_factory.getValidParams(class_name);
        action_params.set<std::string>("type") = "CumulativeValuePostprocessor";
        auto action = std::static_pointer_cast<MooseObjectAction>(
            _action_factory.create(class_name, total_it_names[i], action_params));
        action->getObjectParams().set<PostprocessorName>("postprocessor") = it_per_step_names[i];
        _awh.addActionBlock(action);
      }
    }
  }
}
