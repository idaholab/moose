//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CreateMeshSetupActionsForComponents.h"
#include "ActionWarehouse.h"
#include "ActionFactory.h"
#include "MooseObjectAction.h"

registerMooseAction("MooseApp", CreateMeshSetupActionsForComponents, "meta_action_component");

InputParameters
CreateMeshSetupActionsForComponents::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Adds the SetupMesh-Actions to the simulation, if they are not created by the [Mesh] block.");

  return params;
}

CreateMeshSetupActionsForComponents::CreateMeshSetupActionsForComponents(
    const InputParameters & params)
  : Action(params)
{
}

void
CreateMeshSetupActionsForComponents::act()
{
  if (_current_task == "meta_action_component")
  {
    // Create a default SetupMeshAction. If the user wants to use Mesh parameters, then
    // they must create a Mesh block in their input

    // Simulation already has a Mesh block, no need
    if (_awh.hasActions("setup_mesh") && _awh.hasActions("init_mesh"))
      return;

    // Build action parameters
    InputParameters action_params = _action_factory.getValidParams("SetupMeshAction");

    // Build action and add it to the warehouse
    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _action_factory.create("SetupMeshAction", "Mesh", action_params));
    _awh.addActionBlock(action);

    // Create a default SetupMeshCompleteAction
    action = std::static_pointer_cast<MooseObjectAction>(
        _action_factory.create("SetupMeshCompleteAction", "Mesh", action_params));
    _awh.addActionBlock(action);
  }
  else
    mooseAssert(true, "Should not reach here");
}
