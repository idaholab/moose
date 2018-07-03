#include "RELAP7InitSimulationAction.h"

registerMooseAction("RELAP7App", RELAP7InitSimulationAction, "RELAP7:init_simulation");

template <>
InputParameters
validParams<RELAP7InitSimulationAction>()
{
  InputParameters params = validParams<RELAP7Action>();

  return params;
}

RELAP7InitSimulationAction::RELAP7InitSimulationAction(InputParameters parameters)
  : RELAP7Action(parameters)
{
}

void
RELAP7InitSimulationAction::act()
{
  _simulation.init();
}
