#include "TriSubChannelBaseIC.h"
#include "TriSubChannelMesh.h"

InputParameters
TriSubChannelBaseIC::validParams()
{
  return InitialCondition::validParams();
}

TriSubChannelBaseIC::TriSubChannelBaseIC(const InputParameters & params)
  : InitialCondition(params), _mesh(dynamic_cast<TriSubChannelMesh &>(_fe_problem.mesh()))
{
}
