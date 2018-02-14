#include "RELAP7InitComponentsAction.h"

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
