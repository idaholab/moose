#include "THMBuildMeshAction.h"
#include "THMProblem.h"

registerMooseAction("THMApp", THMBuildMeshAction, "THM:build_mesh");

InputParameters
THMBuildMeshAction::validParams()
{
  InputParameters params = Action::validParams();
  return params;
}

THMBuildMeshAction::THMBuildMeshAction(InputParameters params) : Action(params) {}

void
THMBuildMeshAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
    thm_problem->buildMesh();
}
