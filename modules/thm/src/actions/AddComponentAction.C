#include "AddComponentAction.h"
#include "Simulation.h"

registerMooseAction("RELAP7App", AddComponentAction, "RELAP7:add_component");

template <>
InputParameters
validParams<AddComponentAction>()
{
  InputParameters params = validParams<RELAP7ObjectAction>();
  return params;
}

AddComponentAction::AddComponentAction(InputParameters params) : RELAP7ObjectAction(params) {}

void
AddComponentAction::act()
{
  _simulation.addComponent(_type, _name, _moose_object_pars);
}
