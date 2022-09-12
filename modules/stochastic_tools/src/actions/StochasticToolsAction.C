//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StochasticToolsAction.h"
#include "ActionWarehouse.h"
#include "ActionFactory.h"
#include "MooseEnum.h"
#include "SetupMeshAction.h"

registerMooseAction("StochasticToolsApp", StochasticToolsAction, "auto_create_mesh");
registerMooseAction("StochasticToolsApp", StochasticToolsAction, "auto_create_problem");
registerMooseAction("StochasticToolsApp", StochasticToolsAction, "auto_create_executioner");

InputParameters
StochasticToolsAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Action for performing some common functions for running stochastic simulations.");
  params.addParam<bool>(
      "auto_create_mesh",
      true,
      "Automatically setup the Mesh block for a master application without a simulation.");
  params.addParam<bool>(
      "auto_create_problem",
      true,
      "Automatically setup the Problem block for a master application without a simulation.");
  params.addParam<bool>(
      "auto_create_executioner",
      true,
      "Automatically setup the Executioner block for a master application without a simulation.");
  return params;
}

StochasticToolsAction::StochasticToolsAction(const InputParameters & params) : Action(params) {}

void
StochasticToolsAction::act()
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
    params.set<bool>("solve") = false;
    params.set<bool>("kernel_coverage_check") = false;
    params.set<bool>("skip_nl_system_check") = true;

    // Add Action to the warehouse
    _awh.addActionBlock(action);
  }

  // [Executioner]
  else if (_current_task == "auto_create_executioner" &&
           getParam<bool>("auto_create_executioner") && !_awh.hasActions("setup_executioner"))
  {
    // Build the Action parameters
    InputParameters action_params = _action_factory.getValidParams("CreateExecutionerAction");
    action_params.set<std::string>("type") = "Steady";

    // Create the action
    auto action = std::static_pointer_cast<MooseObjectAction>(
        _action_factory.create("CreateExecutionerAction", "Executioner", action_params));

    // Add Action to the warehouse
    _awh.addActionBlock(action);
  }
}
