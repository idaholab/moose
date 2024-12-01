//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ActionComponent.h"
#include "ActionFactory.h"

InputParameters
ActionComponent::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Base class for components that are defined using actions.");
  params.addParam<bool>("verbose", false, "Whether the component setup should be verbose");
  return params;
}

ActionComponent::ActionComponent(const InputParameters & params)
  : Action(params), _dimension(libMesh::invalid_uint), _verbose(getParam<bool>("verbose"))
{
}

void
ActionComponent::act()
{
  // This is inspired by the PhysicsBase definition of act(). We register components to the
  // task they use, and the base class calls the appropriate virtual member functions
  mooseDoOnce(checkRequiredTasks());

  // These tasks are conceptually what we imagine a mostly-geometrical component should do
  if (_current_task == "add_mesh_generator")
    addMeshGenerators();
  else if (_current_task == "add_positions")
    addPositionsObject();
  else if (_current_task == "add_user_object")
    addUserObjects();
  else if (_current_task == "setup_component")
    setupComponent();
  // If we define the Physics in a Physics block. See PhysicsComponentBase
  else if (_current_task == "init_component_physics")
    addPhysics();
  // These tasks are there to match what the current combined Physics + Component do
  // These combined components will likely still exist in the future, when it makes more
  // sense to include the physics than to split it off into its own block
  else if (_current_task == "add_variable")
    addSolverVariables();
  else
    // For a new task that isn't registered to ActionComponent in the framework
    actOnAdditionalTasks();
}

void
ActionComponent::checkRequiredTasks() const
{
  const auto registered_tasks = _action_factory.getTasksByAction(type());

  // Check for missing tasks
  for (const auto & required_task : _required_tasks)
    if (!registered_tasks.count(required_task))
      mooseWarning(
          "Task '" + required_task +
          "' has been declared as required by a Component parent class of derived class '" +
          type() +
          "' but this task is not registered to the derived class. Registered tasks for "
          "this Component are: " +
          Moose::stringify(registered_tasks));
}
