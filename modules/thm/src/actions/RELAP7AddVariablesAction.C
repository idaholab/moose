#include "RELAP7AddVariablesAction.h"

template <>
InputParameters
validParams<RELAP7AddVariablesAction>()
{
  InputParameters params = validParams<RELAP7Action>();
  return params;
}

RELAP7AddVariablesAction::RELAP7AddVariablesAction(InputParameters params) : RELAP7Action(params) {}

void
RELAP7AddVariablesAction::act()
{
  _simulation.addVariables();
}
