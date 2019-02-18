#include "IntegrityCheckAction.h"
#include "THMApp.h"

registerMooseAction("THMApp", IntegrityCheckAction, "THM:integrity_check");

template <>
InputParameters
validParams<IntegrityCheckAction>()
{
  InputParameters params = validParams<THMAction>();

  return params;
}

IntegrityCheckAction::IntegrityCheckAction(InputParameters parameters) : THMAction(parameters) {}

void
IntegrityCheckAction::act()
{
  THMApp & app = dynamic_cast<THMApp &>(_app);

  if (!app.checkJacobian())
    _simulation.integrityCheck();
}
