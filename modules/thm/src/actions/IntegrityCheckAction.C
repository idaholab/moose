#include "IntegrityCheckAction.h"
#include "RELAP7App.h"

template <>
InputParameters
validParams<IntegrityCheckAction>()
{
  InputParameters params = validParams<R7Action>();

  return params;
}

IntegrityCheckAction::IntegrityCheckAction(InputParameters parameters) : R7Action(parameters) {}

IntegrityCheckAction::~IntegrityCheckAction() {}

void
IntegrityCheckAction::act()
{
  RELAP7App & app = dynamic_cast<RELAP7App &>(_app);

  if (!app.checkJacobian())
    _simulation.integrityCheck();
}
