#include "AddComponentPhysicsAction.h"

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
