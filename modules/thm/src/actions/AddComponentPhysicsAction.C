#include "AddComponentPhysicsAction.h"

registerMooseAction("RELAP7App", AddComponentPhysicsAction, "RELAP7:add_component_physics");

template <>
InputParameters
validParams<AddComponentPhysicsAction>()
{
  InputParameters params = validParams<RELAP7Action>();
  return params;
}

AddComponentPhysicsAction::AddComponentPhysicsAction(InputParameters params) : RELAP7Action(params)
{
}

void
AddComponentPhysicsAction::act()
{
  _simulation.addComponentPhysics();
}
