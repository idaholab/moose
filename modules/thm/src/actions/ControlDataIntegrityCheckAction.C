#include "ControlDataIntegrityCheckAction.h"
#include "THMProblem.h"
#include "THMApp.h"

registerMooseAction("THMApp", ControlDataIntegrityCheckAction, "THM:control_data_integrity_check");

template <>
InputParameters
validParams<ControlDataIntegrityCheckAction>()
{
  InputParameters params = validParams<Action>();

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
