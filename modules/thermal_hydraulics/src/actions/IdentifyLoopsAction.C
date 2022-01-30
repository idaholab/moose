#include "IdentifyLoopsAction.h"
#include "THMProblem.h"

registerMooseAction("ThermalHydraulicsApp", IdentifyLoopsAction, "THM:identify_loops");

InputParameters
IdentifyLoopsAction::validParams()
{
  InputParameters params = Action::validParams();

  return params;
}

IdentifyLoopsAction::IdentifyLoopsAction(InputParameters parameters) : Action(parameters) {}

void
IdentifyLoopsAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
    thm_problem->identifyLoops();
}
