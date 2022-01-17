#include "THMInitSimulationAction.h"
#include "THMProblem.h"

registerMooseAction("ThermalHydraulicsApp", THMInitSimulationAction, "THM:init_simulation");

InputParameters
THMInitSimulationAction::validParams()
{
  InputParameters params = Action::validParams();

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
