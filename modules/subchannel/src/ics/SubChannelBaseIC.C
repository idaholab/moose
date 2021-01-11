#include "SubChannelBaseIC.h"
#include "SubChannelMesh.h"

InputParameters
SubChannelBaseIC::validParams()
{
  return InitialCondition::validParams();
}

SubChannelBaseIC::SubChannelBaseIC(const InputParameters & params)
  : InitialCondition(params), _mesh(dynamic_cast<SubChannelMesh &>(_fe_problem.mesh()))
{
}
