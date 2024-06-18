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
#include "FEProblem.h"
#include "Factory.h"
#include "MooseApp.h"
#include "PhysicsBase.h"

InputParameters
ActionComponent::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Base class for components that are defined using action.");
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

  // These tasks are conceptually what we image a mostly-geometrical component should do
  if (_current_task == "add_mesh_generator")
    addMeshGenerators();
  else if (_current_task == "add_positions")
    addPositionsObject();
  else if (_current_task == "add_user_object")
    addUserObjects();
  else if (_current_task == "setup_component")
    setupComponent();
  // If we define the Physics in a Physics block. See PhysicsComponentHelper
  else if (_current_task == "init_component_physics")
    addPhysics();
  // These tasks are there to match what the current combined Physics + Component do
  // These combined components will likely still exist in the future, when it makes more
  // sense to include the physics than to split it off into its own block
  else if (_current_task == "add_variable")
    addNonlinearVariables();
  else
    // For a new task that isn't registered to ActionComponent in the framework
    actOnAdditionalTasks();
}
