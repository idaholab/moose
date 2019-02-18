#include "THMSetupMeshAction.h"

registerMooseAction("THMApp", THMSetupMeshAction, "THM:setup_mesh");

template <>
InputParameters
validParams<THMSetupMeshAction>()
{
  InputParameters params = validParams<THMAction>();
  return params;
}

THMSetupMeshAction::THMSetupMeshAction(InputParameters params) : THMAction(params) {}

void
THMSetupMeshAction::act()
{
  _simulation.setupMesh();
}
