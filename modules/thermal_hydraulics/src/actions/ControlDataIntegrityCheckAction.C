#include "ControlDataIntegrityCheckAction.h"
#include "THMProblem.h"
#include "ThermalHydraulicsApp.h"

registerMooseAction("ThermalHydraulicsApp",
                    ControlDataIntegrityCheckAction,
                    "THM:control_data_integrity_check");

InputParameters
ControlDataIntegrityCheckAction::validParams()
{
  InputParameters params = Action::validParams();

  return params;
}

ControlDataIntegrityCheckAction::ControlDataIntegrityCheckAction(InputParameters parameters)
  : Action(parameters)
{
}

void
ControlDataIntegrityCheckAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
    thm_problem->controlDataIntegrityCheck();
}
