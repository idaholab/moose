#include "IdentifyLoopsAction.h"

template <>
InputParameters
validParams<IdentifyLoopsAction>()
{
  InputParameters params = validParams<RELAP7Action>();

  return params;
}

IdentifyLoopsAction::IdentifyLoopsAction(InputParameters parameters) : RELAP7Action(parameters) {}

void
IdentifyLoopsAction::act()
{
  _simulation.identifyLoops();
}
