#include "BetterQuadSubChannelBaseIC.h"
#include "BetterQuadSubChannelMesh.h"

InputParameters
BetterQuadSubChannelBaseIC::validParams()
{
  return InitialCondition::validParams();
}

BetterQuadSubChannelBaseIC::BetterQuadSubChannelBaseIC(const InputParameters & params)
  : InitialCondition(params), _mesh(dynamic_cast<BetterQuadSubChannelMesh &>(_fe_problem.mesh()))
{
}
