#include "AddComponentPhysicsAction.h"

template <>
InputParameters
validParams<AddComponentPhysicsAction>()
{
  InputParameters params = validParams<R7Action>();
  return params;
}

AddComponentPhysicsAction::AddComponentPhysicsAction(InputParameters params) : R7Action(params) {}

void
AddComponentPhysicsAction::act()
{
  _simulation.addComponentPhysics();
}
