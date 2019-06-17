#include "THMMesh.h"
#include "libmesh/node.h"

registerMooseObject("THMApp", THMMesh);

const BoundaryName THMMesh::INVALID_BOUNDARY_ID = "invalid_boundary_id";

template <>
InputParameters
validParams<THMMesh>()
{
  InputParameters params = validParams<MooseMesh>();

  return params;
}

THMMesh::THMMesh(const InputParameters & parameters)
  : MooseMesh(parameters), _dim(getParam<MooseEnum>("dim")), _next_node_id(0)
{
}

THMMesh::THMMesh(const THMMesh & other_mesh)
  : MooseMesh(other_mesh), _dim(other_mesh._dim), _next_node_id(other_mesh._next_node_id)
{
}

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

dof_id_type
THMMesh::getNextNodeId()
{
  unsigned int id = _next_node_id++;
  return id;
}

Node *
THMMesh::addNode(const Point & pt)
{
  dof_id_type id = getNextNodeId();
  Node * node = _mesh->add_point(pt, id);
  node->set_unique_id() = id;
  return node;
}
