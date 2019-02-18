#include "THMAddVariablesAction.h"

registerMooseAction("THMApp", THMAddVariablesAction, "THM:add_variables");

template <>
InputParameters
validParams<THMAddVariablesAction>()
{
  InputParameters params = validParams<THMAction>();
  return params;
}

THMAddVariablesAction::THMAddVariablesAction(InputParameters params) : THMAction(params) {}

void
THMAddVariablesAction::act()
{
  _simulation.addVariables();
}
