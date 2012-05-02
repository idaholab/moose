#include "AddComponentPhysicsAction.h"

template<>
InputParameters validParams<AddComponentPhysicsAction>()
{
  InputParameters params = validParams<R7Action>();
  return params;
}

AddComponentPhysicsAction::AddComponentPhysicsAction(const std::string & name, InputParameters params) :
    R7Action(name, params)
{
}

AddComponentPhysicsAction::~AddComponentPhysicsAction()
{
}

void
AddComponentPhysicsAction::act()
{
  _simulation.addComponentPhysics();
}
