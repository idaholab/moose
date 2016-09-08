#include "RELAP7Mesh.h"

template<>
InputParameters validParams<RELAP7Mesh>()
{
  InputParameters params = validParams<MooseMesh>();

  return params;
}

RELAP7Mesh::RELAP7Mesh(const InputParameters & parameters) :
    MooseMesh(parameters),
    _dim(getParam<MooseEnum>("dim"))
{
}

RELAP7Mesh::RELAP7Mesh(const RELAP7Mesh & other_mesh) :
    MooseMesh(other_mesh),
    _dim(other_mesh._dim)
{
}

RELAP7Mesh::~RELAP7Mesh()
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
  return *(new RELAP7Mesh(*this));
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
