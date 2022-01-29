#include "THMAddVariablesAction.h"
#include "THMProblem.h"

registerMooseAction("ThermalHydraulicsApp", THMAddVariablesAction, "THM:add_variables");

InputParameters
THMAddVariablesAction::validParams()
{
  InputParameters params = Action::validParams();
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
