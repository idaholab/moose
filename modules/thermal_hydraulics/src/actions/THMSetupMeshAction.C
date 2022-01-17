#include "THMSetupMeshAction.h"
#include "THMProblem.h"

registerMooseAction("ThermalHydraulicsApp", THMSetupMeshAction, "THM:setup_mesh");

InputParameters
THMSetupMeshAction::validParams()
{
  InputParameters params = Action::validParams();
  return params;
}

THMSetupMeshAction::THMSetupMeshAction(InputParameters params) : Action(params) {}

void
THMSetupMeshAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
    thm_problem->setupMesh();
}
