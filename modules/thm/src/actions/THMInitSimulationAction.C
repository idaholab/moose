#include "THMInitSimulationAction.h"

registerMooseAction("THMApp", THMInitSimulationAction, "THM:init_simulation");

template <>
InputParameters
validParams<THMInitSimulationAction>()
{
  InputParameters params = validParams<THMAction>();

  return params;
}

THMInitSimulationAction::THMInitSimulationAction(InputParameters parameters) : THMAction(parameters)
{
}

void
THMInitSimulationAction::act()
{
  _simulation.init();
}
