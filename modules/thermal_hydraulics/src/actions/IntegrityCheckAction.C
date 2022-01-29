#include "IntegrityCheckAction.h"
#include "THMProblem.h"
#include "ThermalHydraulicsApp.h"

registerMooseAction("ThermalHydraulicsApp", IntegrityCheckAction, "THM:integrity_check");

InputParameters
IntegrityCheckAction::validParams()
{
  InputParameters params = Action::validParams();

  return params;
}

IntegrityCheckAction::IntegrityCheckAction(InputParameters parameters) : Action(parameters) {}

void
IntegrityCheckAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
    thm_problem->integrityCheck();
}
