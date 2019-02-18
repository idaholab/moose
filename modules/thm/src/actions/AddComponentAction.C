#include "AddComponentAction.h"
#include "Simulation.h"

registerMooseAction("THMApp", AddComponentAction, "THM:add_component");

template <>
InputParameters
validParams<AddComponentAction>()
{
  InputParameters params = validParams<THMObjectAction>();
  return params;
}

AddComponentAction::AddComponentAction(InputParameters params) : THMObjectAction(params) {}

void
AddComponentAction::act()
{
  _simulation.addComponent(_type, _name, _moose_object_pars);
}
