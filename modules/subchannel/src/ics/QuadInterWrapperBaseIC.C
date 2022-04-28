#include "QuadInterWrapperBaseIC.h"
#include "QuadInterWrapperMesh.h"

InputParameters
QuadInterWrapperBaseIC::validParams()
{
  return InitialCondition::validParams();
}

QuadInterWrapperBaseIC::QuadInterWrapperBaseIC(const InputParameters & params)
  : InitialCondition(params), _mesh(getMesh(_fe_problem.mesh()))
{
}

QuadInterWrapperMesh &
QuadInterWrapperBaseIC::getMesh(MooseMesh & mesh)
{
  QuadInterWrapperMesh * m = dynamic_cast<QuadInterWrapperMesh *>(&mesh);
  if (m)
    return dynamic_cast<QuadInterWrapperMesh &>(mesh);
  else
    mooseError(name(),
               ": This initial condition works only with quadrilateral mesh. Update your input "
               "file to use [QuadInterWrapperMesh] block for mesh.");
}
