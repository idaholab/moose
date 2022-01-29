#include "THMPreconditioningIntegrityCheckAction.h"
#include "THMProblem.h"
#include "ThermalHydraulicsApp.h"

registerMooseAction("ThermalHydraulicsApp",
                    THMPreconditioningIntegrityCheckAction,
                    "THM:preconditioning_integrity_check");

InputParameters
THMPreconditioningIntegrityCheckAction::validParams()
{
  InputParameters params = Action::validParams();

  return params;
}

THMPreconditioningIntegrityCheckAction::THMPreconditioningIntegrityCheckAction(
    InputParameters parameters)
  : Action(parameters)
{
}

void
THMPreconditioningIntegrityCheckAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
    thm_problem->couplingMatrixIntegrityCheck();
}
