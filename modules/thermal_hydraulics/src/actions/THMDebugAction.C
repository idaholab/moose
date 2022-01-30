#include "THMDebugAction.h"
#include "THMProblem.h"

registerMooseAction("ThermalHydraulicsApp", THMDebugAction, "THM:debug_action");

InputParameters
THMDebugAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addParam<bool>("check_jacobian", false, "Set to true to check jacobian");

  return params;
}

THMDebugAction::THMDebugAction(InputParameters params) : Action(params) {}

void
THMDebugAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
    thm_problem->setCheckJacobian(getParam<bool>("check_jacobian"));
}
