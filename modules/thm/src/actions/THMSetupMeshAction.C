#include "THMSetupMeshAction.h"
#include "THMProblem.h"

registerMooseAction("THMApp", THMSetupMeshAction, "THM:setup_mesh");

template <>
InputParameters
validParams<THMSetupMeshAction>()
{
  InputParameters params = validParams<Action>();
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
