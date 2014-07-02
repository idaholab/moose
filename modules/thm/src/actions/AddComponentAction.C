#include "AddComponentAction.h"
#include "Simulation.h"

template <>
InputParameters validParams<AddComponentAction>()
{
  InputParameters params = validParams<R7ObjectAction>();
  return params;
}

AddComponentAction::AddComponentAction(const std::string & name, InputParameters params) :
    R7ObjectAction(name, params)
{
}

void
AddComponentAction::act()
{
  Moose::out << "AddComponent: " << _type << " " << getShortName() << std::endl;
  _simulation.addComponent(_type, getShortName(), getObjectParams());
}
