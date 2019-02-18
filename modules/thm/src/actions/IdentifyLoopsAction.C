#include "IdentifyLoopsAction.h"

registerMooseAction("THMApp", IdentifyLoopsAction, "THM:identify_loops");

template <>
InputParameters
validParams<IdentifyLoopsAction>()
{
  InputParameters params = validParams<THMAction>();

  return params;
}

IdentifyLoopsAction::IdentifyLoopsAction(InputParameters parameters) : THMAction(parameters) {}

void
IdentifyLoopsAction::act()
{
  _simulation.identifyLoops();
}
