#include "RELAP7SetupMeshAction.h"

registerMooseAction("RELAP7App", RELAP7SetupMeshAction, "RELAP7:init_displaced_problem");

template <>
InputParameters
validParams<RELAP7SetupMeshAction>()
{
  InputParameters params = validParams<RELAP7Action>();
  return params;
}

RELAP7SetupMeshAction::RELAP7SetupMeshAction(InputParameters params) : RELAP7Action(params) {}

void
RELAP7SetupMeshAction::act()
{
  _simulation.setupMesh();
}
