#include "THMInitSimulationAction.h"
#include "THMProblem.h"

registerMooseAction("THMApp", THMInitSimulationAction, "THM:init_simulation");

template <>
InputParameters
validParams<THMInitSimulationAction>()
{
  InputParameters params = validParams<Action>();

  return params;
}

THMInitSimulationAction::THMInitSimulationAction(InputParameters parameters) : Action(parameters) {}

void
THMInitSimulationAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
    thm_problem->initSimulation();
}
