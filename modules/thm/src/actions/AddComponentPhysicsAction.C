#include "AddComponentPhysicsAction.h"

registerMooseAction("THMApp", AddComponentPhysicsAction, "THM:add_component_physics");

template <>
InputParameters
validParams<AddComponentPhysicsAction>()
{
  InputParameters params = validParams<THMAction>();
  return params;
}

AddComponentPhysicsAction::AddComponentPhysicsAction(InputParameters params) : THMAction(params) {}

void
AddComponentPhysicsAction::act()
{
  _simulation.addComponentPhysics();
}
