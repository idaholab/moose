#include "THMMesh.h"

registerMooseObject("THMApp", THMMesh);

template <>
InputParameters
validParams<THMMesh>()
{
  InputParameters params = validParams<MooseMesh>();

  return params;
}

THMMesh::THMMesh(const InputParameters & parameters)
  : MooseMesh(parameters), _dim(getParam<MooseEnum>("dim"))
{
}

THMMesh::THMMesh(const THMMesh & other_mesh) : MooseMesh(other_mesh), _dim(other_mesh._dim) {}

unsigned int
THMMesh::dimension() const
{
  return _dim;
}

unsigned int
THMMesh::effectiveSpatialDimension() const
{
  return _dim;
}

MooseMesh &
THMMesh::clone() const
{
  mooseError("CRITICAL ERROR: calling clone() is not allowed and should not happen.");
}

std::unique_ptr<MooseMesh>
THMMesh::safeClone() const
{
  return libmesh_make_unique<THMMesh>(*this);
}

void
THMMesh::buildMesh()
{
  getMesh().set_spatial_dimension(_dim);
}

void
THMMesh::prep()
{
  prepare(true);
}
