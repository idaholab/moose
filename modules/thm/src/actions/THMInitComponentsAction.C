#include "THMInitComponentsAction.h"
#include "THMProblem.h"

registerMooseAction("THMApp", THMInitComponentsAction, "THM:init_components");

InputParameters
THMInitComponentsAction::validParams()
{
  InputParameters params = Action::validParams();

  return params;
}

THMInitComponentsAction::THMInitComponentsAction(InputParameters parameters) : Action(parameters) {}

void
THMInitComponentsAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
    thm_problem->initComponents();
}
