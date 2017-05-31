#include "IdentifyLoopsAction.h"

template <>
InputParameters
validParams<IdentifyLoopsAction>()
{
  InputParameters params = validParams<R7Action>();

  return params;
}

IdentifyLoopsAction::IdentifyLoopsAction(InputParameters parameters) : R7Action(parameters) {}

void
IdentifyLoopsAction::act()
{
  _simulation.identifyLoops();
}
