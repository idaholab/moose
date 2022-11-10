//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptimizationAction.h"
#include "ActionWarehouse.h"
#include "ActionFactory.h"
#include "MooseEnum.h"
#include "SetupMeshAction.h"

registerMooseAction("OptimizationApp", OptimizationAction, "auto_create_mesh");
registerMooseAction("OptimizationApp", OptimizationAction, "auto_create_problem");
registerMooseAction("OptimizationApp", OptimizationAction, "auto_create_executioner");

InputParameters
OptimizationAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Action for performing some common functions for running optimization simulations.");
  params.addParam<bool>(
      "auto_create_mesh",
      true,
      "Automatically setup the Mesh block for a main application without a simulation.");
  params.addParam<bool>(
      "auto_create_problem",
      true,
      "Automatically setup the Problem block for a main application without a simulation.");
  return params;
}

OptimizationAction::OptimizationAction(const InputParameters & params) : Action(params) {}

void
OptimizationAction::act()
{
  // [Mesh]
  if (_current_task == "auto_create_mesh" && getParam<bool>("auto_create_mesh") &&
      !_awh.hasActions("setup_mesh"))
  {
    // Build the Action parameters
    InputParameters action_params = _action_factory.getValidParams("SetupMeshAction");
    action_params.set<std::string>("type") = "GeneratedMesh";

    // Create The Action
    auto action = std::static_pointer_cast<MooseObjectAction>(
        _action_factory.create("SetupMeshAction", "Mesh", action_params));

    // Set the object parameters
    InputParameters & params = action->getObjectParams();
    params.set<MooseEnum>("dim") = "1";
    params.set<unsigned int>("nx") = 1;

    // Add Action to the warehouse
    _awh.addActionBlock(action);
  }

  // [Problem]
  else if (_current_task == "auto_create_problem" && getParam<bool>("auto_create_problem") &&
           !_awh.hasActions("create_problem"))
  {
    // Build the Action parameters
    InputParameters action_params = _action_factory.getValidParams("CreateProblemAction");

    // Create the action
    auto action = std::static_pointer_cast<MooseObjectAction>(
        _action_factory.create("CreateProblemAction", "Problem", action_params));

    // Set the object parameters
    InputParameters & params = action->getObjectParams();
    params.set<bool>("kernel_coverage_check") = false;
    params.set<bool>("skip_nl_system_check") = true;

    // Add Action to the warehouse
    _awh.addActionBlock(action);
  }
}
