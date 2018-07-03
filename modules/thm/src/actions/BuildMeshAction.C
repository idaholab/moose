#include "BuildMeshAction.h"
#include "Simulation.h"

registerMooseAction("RELAP7App", BuildMeshAction, "RELAP7:build_mesh");

template <>
InputParameters
validParams<BuildMeshAction>()
{
  InputParameters params = validParams<RELAP7Action>();
  return params;
}

BuildMeshAction::BuildMeshAction(InputParameters params) : RELAP7Action(params) {}

void
BuildMeshAction::act()
{
  _simulation.buildMesh();
}
