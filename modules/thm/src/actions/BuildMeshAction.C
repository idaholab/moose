#include "BuildMeshAction.h"
#include "Simulation.h"

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
