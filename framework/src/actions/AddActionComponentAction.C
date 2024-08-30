//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "AddActionComponentAction.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseApp.h"

registerMooseAction("MooseApp", AddActionComponentAction, "meta_action");

InputParameters
AddActionComponentAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Action responsible for creating component actions. A component "
                             "action is a component derived from an Action base class");
  params.addRequiredParam<std::string>("type",
                                       "Type of the ComponentAction to create with this Action");
  params.addParam<bool>("isObjectAction", true, "Indicates that this is a MooseObjectAction.");
  params.addParamNamesToGroup("isObjectAction", "Advanced");
  return params;
}

AddActionComponentAction::AddActionComponentAction(const InputParameters & params)
  : Action(params),
    _component_type(getParam<std::string>("type")),
    _component_params(_action_factory.getValidParams(_component_type))
{
  _component_params.blockFullpath() = params.blockFullpath();

  // Verify that a Mesh syntax has been passed, as we use the mesh creation tasks
  // from SetupMeshAction, etc
  if (!_awh.hasTask("setup_mesh") || !_awh.hasTask("init_mesh"))
    mooseError("ActionComponents require a [Mesh] block to be defined, even if empty");
}

void
AddActionComponentAction::act()
{
  if (_current_task == "meta_action")
  {
    // Set some component parameters
    _component_params.set<bool>("_built_by_moose") = true;
    _component_params.set<std::string>("registered_identifier") = "(AutoBuilt)";

    _component_params.applyParameters(parameters());

    // Create and add the action to the warehouse
    auto action_component = MooseSharedNamespace::static_pointer_cast<Action>(
        _action_factory.create(_component_type, name(), _component_params));
    _awh.addActionBlock(action_component);
  }
}

void
AddActionComponentAction::addRelationshipManagers(
    Moose::RelationshipManagerType input_rm_component_type)
{
  addRelationshipManagers(input_rm_component_type, _component_params);
}
