#include "THMAddVariablesAction.h"
#include "THMProblem.h"

registerMooseAction("THMApp", THMAddVariablesAction, "THM:add_variables");

template <>
InputParameters
validParams<THMAddVariablesAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}

THMAddVariablesAction::THMAddVariablesAction(InputParameters params) : Action(params) {}

void
THMAddVariablesAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
    thm_problem->addVariables();
}
