#include "BuildMeshAction.h"
#include "Simulation.h"

registerMooseAction("THMApp", BuildMeshAction, "THM:build_mesh");

template <>
InputParameters
validParams<BuildMeshAction>()
{
  InputParameters params = validParams<THMAction>();
  return params;
}

BuildMeshAction::BuildMeshAction(InputParameters params) : THMAction(params) {}

void
BuildMeshAction::act()
{
  _simulation.buildMesh();
}
