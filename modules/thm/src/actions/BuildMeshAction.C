#include "BuildMeshAction.h"
#include "Simulation.h"

template<>
InputParameters validParams<BuildMeshAction>()
{
  InputParameters params = validParams<R7Action>();
  return params;
}

BuildMeshAction::BuildMeshAction(InputParameters params) :
    R7Action(params)
{
}

BuildMeshAction::~BuildMeshAction()
{
}

void
BuildMeshAction::act()
{
  _simulation.buildMesh();
}
