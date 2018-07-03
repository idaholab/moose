#include "RELAP7InitComponentsAction.h"

registerMooseAction("RELAP7App", RELAP7InitComponentsAction, "RELAP7:init_components");

template <>
InputParameters
validParams<RELAP7InitComponentsAction>()
{
  InputParameters params = validParams<RELAP7Action>();

  return params;
}

RELAP7InitComponentsAction::RELAP7InitComponentsAction(InputParameters parameters)
  : RELAP7Action(parameters)
{
}

void
RELAP7InitComponentsAction::act()
{
  _simulation.initComponents();
}
