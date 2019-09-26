#include "THMInitComponentsAction.h"
#include "THMProblem.h"

registerMooseAction("THMApp", THMInitComponentsAction, "THM:init_components");

template <>
InputParameters
validParams<THMInitComponentsAction>()
{
  InputParameters params = validParams<Action>();

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
