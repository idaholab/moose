#include "RELAP7Mesh.h"

registerMooseObject("RELAP7App", RELAP7Mesh);

template <>
InputParameters
validParams<RELAP7Mesh>()
{
  InputParameters params = validParams<MooseMesh>();

  return params;
}

RELAP7Mesh::RELAP7Mesh(const InputParameters & parameters)
  : MooseMesh(parameters), _dim(getParam<MooseEnum>("dim"))
{
  getMesh().set_spatial_dimension(_dim);
}

RELAP7Mesh::RELAP7Mesh(const RELAP7Mesh & other_mesh) : MooseMesh(other_mesh), _dim(other_mesh._dim)
{
}

unsigned int
RELAP7Mesh::dimension() const
{
  return _dim;
}

MooseMesh &
RELAP7Mesh::clone() const
{
  mooseError("CRITICAL ERROR: calling clone() is not allowed and should not happen.");
}

std::unique_ptr<MooseMesh>
RELAP7Mesh::safeClone() const
{
  return libmesh_make_unique<RELAP7Mesh>(*this);
}

void
RELAP7Mesh::buildMesh()
{
}

void
RELAP7Mesh::prep()
{
  prepare(true);
}
