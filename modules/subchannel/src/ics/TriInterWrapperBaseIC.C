#include "TriInterWrapperBaseIC.h"
#include "TriInterWrapperMesh.h"

InputParameters
TriInterWrapperBaseIC::validParams()
{
  return InitialCondition::validParams();
}

TriInterWrapperBaseIC::TriInterWrapperBaseIC(const InputParameters & params)
  : InitialCondition(params), _mesh(getMesh(_fe_problem.mesh()))
{
}

TriInterWrapperMesh &
TriInterWrapperBaseIC::getMesh(MooseMesh & mesh)
{
  TriInterWrapperMesh * m = dynamic_cast<TriInterWrapperMesh *>(&mesh);
  if (m)
    return dynamic_cast<TriInterWrapperMesh &>(mesh);
  else
    mooseError(name(),
               ": This initial condition works only with triangular subchannel geometry. Update "
               "your input file to use TriInterWrapperMesh in the mesh block.");
}
