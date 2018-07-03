#include "IntegrityCheckAction.h"
#include "RELAP7App.h"

registerMooseAction("RELAP7App", IntegrityCheckAction, "RELAP7:integrity_check");

template <>
InputParameters
validParams<IntegrityCheckAction>()
{
  InputParameters params = validParams<RELAP7Action>();

  return params;
}

IntegrityCheckAction::IntegrityCheckAction(InputParameters parameters) : RELAP7Action(parameters) {}

void
IntegrityCheckAction::act()
{
  RELAP7App & app = dynamic_cast<RELAP7App &>(_app);

  if (!app.checkJacobian())
    _simulation.integrityCheck();
}
