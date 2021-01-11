#include "SubChannelBaseIC.h"
#include "QuadSubChannelMesh.h"

InputParameters
SubChannelBaseIC::validParams()
{
  return InitialCondition::validParams();
}

SubChannelBaseIC::SubChannelBaseIC(const InputParameters & params)
  : InitialCondition(params), _mesh(dynamic_cast<QuadSubChannelMesh &>(_fe_problem.mesh()))
{
}
