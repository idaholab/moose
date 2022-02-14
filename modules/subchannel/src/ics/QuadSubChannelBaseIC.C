#include "QuadSubChannelBaseIC.h"
#include "QuadSubChannelMesh.h"

InputParameters
QuadSubChannelBaseIC::validParams()
{
  return InitialCondition::validParams();
}

QuadSubChannelBaseIC::QuadSubChannelBaseIC(const InputParameters & params)
  : InitialCondition(params), _mesh(getMesh(_fe_problem.mesh()))
{
}

QuadSubChannelMesh &
QuadSubChannelBaseIC::getMesh(MooseMesh & mesh)
{
  QuadSubChannelMesh * m = dynamic_cast<QuadSubChannelMesh *>(&mesh);
  if (m)
    return dynamic_cast<QuadSubChannelMesh &>(mesh);
  else
    mooseError(name(),
               ": This initial condition works only with quadrilateral mesh. Update your input "
               "file to use [QuadSubChannelMesh] block for mesh.");
}
