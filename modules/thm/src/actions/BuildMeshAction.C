#include "BuildMeshAction.h"
#include "Simulation.h"

template<>
InputParameters validParams<BuildMeshAction>()
{
  InputParameters params = validParams<R7Action>();
  return params;
}

BuildMeshAction::BuildMeshAction(const std::string & name, InputParameters params) :
    R7Action(name, params)
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
