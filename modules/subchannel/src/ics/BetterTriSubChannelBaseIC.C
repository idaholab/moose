#include "BetterTriSubChannelBaseIC.h"
#include "BetterTriSubChannelMesh.h"

InputParameters
BetterTriSubChannelBaseIC::validParams()
{
  return InitialCondition::validParams();
}

BetterTriSubChannelBaseIC::BetterTriSubChannelBaseIC(const InputParameters & params)
  : InitialCondition(params), _mesh(dynamic_cast<BetterTriSubChannelMesh &>(_fe_problem.mesh()))
{
}
