#include "THMPrintComponentLoopsAction.h"
#include "THMProblem.h"

registerMooseAction("THMApp", THMPrintComponentLoopsAction, "THM:print_component_loops");

InputParameters
THMPrintComponentLoopsAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addParam<bool>("print_component_loops", false, "Flag to print component loops");

  return params;
}

THMPrintComponentLoopsAction::THMPrintComponentLoopsAction(InputParameters params) : Action(params)
{
}

void
THMPrintComponentLoopsAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem && getParam<bool>("print_component_loops"))
    thm_problem->printComponentLoops();
}
