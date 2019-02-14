#include "THMInitComponentsAction.h"

registerMooseAction("THMApp", THMInitComponentsAction, "THM:init_components");

template <>
InputParameters
validParams<THMInitComponentsAction>()
{
  InputParameters params = validParams<THMAction>();

  return params;
}

THMInitComponentsAction::THMInitComponentsAction(InputParameters parameters) : THMAction(parameters)
{
}

void
THMInitComponentsAction::act()
{
  _simulation.initComponents();
}
