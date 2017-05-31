#include "ControlDataIntegrityCheckAction.h"
#include "RELAP7App.h"

template <>
InputParameters
validParams<ControlDataIntegrityCheckAction>()
{
  InputParameters params = validParams<R7Action>();

  return params;
}

ControlDataIntegrityCheckAction::ControlDataIntegrityCheckAction(InputParameters parameters)
  : R7Action(parameters)
{
}

void
ControlDataIntegrityCheckAction::act()
{
  RELAP7App & app = dynamic_cast<RELAP7App &>(_app);

  if (!app.checkJacobian())
    _simulation.controlDataIntegrityCheck();
}
