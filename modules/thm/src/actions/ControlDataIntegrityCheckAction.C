#include "ControlDataIntegrityCheckAction.h"
#include "THMApp.h"

registerMooseAction("THMApp", ControlDataIntegrityCheckAction, "THM:control_data_integrity_check");

template <>
InputParameters
validParams<ControlDataIntegrityCheckAction>()
{
  InputParameters params = validParams<THMAction>();

  return params;
}

ControlDataIntegrityCheckAction::ControlDataIntegrityCheckAction(InputParameters parameters)
  : THMAction(parameters)
{
}

void
ControlDataIntegrityCheckAction::act()
{
  THMApp & app = dynamic_cast<THMApp &>(_app);

  if (!app.checkJacobian())
    _simulation.controlDataIntegrityCheck();
}
