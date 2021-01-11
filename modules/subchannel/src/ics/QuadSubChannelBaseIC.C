#include "QuadSubChannelBaseIC.h"
#include "QuadSubChannelMesh.h"

InputParameters
QuadSubChannelBaseIC::validParams()
{
  return InitialCondition::validParams();
}

QuadSubChannelBaseIC::QuadSubChannelBaseIC(const InputParameters & params)
  : InitialCondition(params), _mesh(dynamic_cast<QuadSubChannelMesh &>(_fe_problem.mesh()))
{
}
