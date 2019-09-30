#include "IdentifyLoopsAction.h"
#include "THMProblem.h"

registerMooseAction("THMApp", IdentifyLoopsAction, "THM:identify_loops");

template <>
InputParameters
validParams<IdentifyLoopsAction>()
{
  InputParameters params = validParams<Action>();

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
