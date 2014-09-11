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
  _simulation.addComponent(_type, getShortName(), getObjectParams());
}
