//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMMesh.h"
#include "libmesh/node.h"
#include "libmesh/edge_edge2.h"
#include "libmesh/edge_edge3.h"
#include "libmesh/face_quad4.h"
#include "libmesh/face_quad9.h"

registerMooseObject("ThermalHydraulicsApp", THMMesh);

const BoundaryName THMMesh::INVALID_BOUNDARY_ID = "invalid_boundary_id";

InputParameters
THMMesh::validParams()
{
  InputParameters params = MooseMesh::validParams();
  // we do not allow renumbering, becuase we generate our meshes
  params.set<bool>("allow_renumbering") = false;
  return params;
}

THMMesh::THMMesh(const InputParameters & parameters)
  : MooseMesh(parameters),
    _dim(getParam<MooseEnum>("dim")),
    _next_node_id(0),
    _next_element_id(0),
    _next_subdomain_id(0),
    _next_boundary_id(0)
{
}

THMMesh::THMMesh(const THMMesh & other_mesh)
  : MooseMesh(other_mesh),
    _dim(other_mesh._dim),
    _next_node_id(other_mesh._next_node_id),
    _next_element_id(other_mesh._next_element_id),
    _next_subdomain_id(other_mesh._next_subdomain_id),
    _next_boundary_id(other_mesh._next_boundary_id)
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

std::unique_ptr<MooseMesh>
THMMesh::safeClone() const
{
  return std::make_unique<THMMesh>(*this);
}

void
THMMesh::buildMesh()
{
  getMesh().set_spatial_dimension(_dim);
}

dof_id_type
THMMesh::getNextNodeId()
{
  dof_id_type id = _next_node_id++;
  return id;
}

dof_id_type
THMMesh::getNextElementId()
{
  dof_id_type id = _next_element_id++;
  return id;
}

Node *
THMMesh::addNode(const Point & pt)
{
  dof_id_type id = getNextNodeId();
  Node * node = _mesh->add_point(pt, id);
  return node;
}

Elem *
THMMesh::addElement(libMesh::ElemType elem_type, const std::vector<dof_id_type> & node_ids)
{
  dof_id_type elem_id = getNextElementId();

  Elem * elem = libMesh::Elem::build(elem_type).release();
  elem->set_id(elem_id);
  _mesh->add_elem(elem);
  for (std::size_t i = 0; i < node_ids.size(); i++)
    elem->set_node(i) = _mesh->node_ptr(node_ids[i]);
  return elem;
}

Elem *
THMMesh::addElementEdge2(dof_id_type node0, dof_id_type node1)
{
  dof_id_type elem_id = getNextElementId();

  Elem * elem = new Edge2;
  elem->set_id(elem_id);
  _mesh->add_elem(elem);
  elem->set_node(0) = _mesh->node_ptr(node0);
  elem->set_node(1) = _mesh->node_ptr(node1);
  return elem;
}

Elem *
THMMesh::addElementEdge3(dof_id_type node0, dof_id_type node1, dof_id_type node2)
{
  dof_id_type elem_id = getNextElementId();

  Elem * elem = new Edge3;
  elem->set_id(elem_id);
  _mesh->add_elem(elem);
  elem->set_node(0) = _mesh->node_ptr(node0);
  elem->set_node(1) = _mesh->node_ptr(node1);
  elem->set_node(2) = _mesh->node_ptr(node2);
  return elem;
}

Elem *
THMMesh::addElementQuad4(dof_id_type node0, dof_id_type node1, dof_id_type node2, dof_id_type node3)
{
  dof_id_type elem_id = getNextElementId();

  Elem * elem = new Quad4;
  elem->set_id(elem_id);
  _mesh->add_elem(elem);
  elem->set_node(0) = _mesh->node_ptr(node0);
  elem->set_node(1) = _mesh->node_ptr(node1);
  elem->set_node(2) = _mesh->node_ptr(node2);
  elem->set_node(3) = _mesh->node_ptr(node3);
  return elem;
}

Elem *
THMMesh::addElementQuad9(dof_id_type node0,
                         dof_id_type node1,
                         dof_id_type node2,
                         dof_id_type node3,
                         dof_id_type node4,
                         dof_id_type node5,
                         dof_id_type node6,
                         dof_id_type node7,
                         dof_id_type node8)
{
  dof_id_type elem_id = getNextElementId();

  Elem * elem = new Quad9;
  elem->set_id(elem_id);
  _mesh->add_elem(elem);
  // vertices
  elem->set_node(0) = _mesh->node_ptr(node0);
  elem->set_node(1) = _mesh->node_ptr(node1);
  elem->set_node(2) = _mesh->node_ptr(node2);
  elem->set_node(3) = _mesh->node_ptr(node3);
  // mid-edges
  elem->set_node(4) = _mesh->node_ptr(node4);
  elem->set_node(5) = _mesh->node_ptr(node5);
  elem->set_node(6) = _mesh->node_ptr(node6);
  elem->set_node(7) = _mesh->node_ptr(node7);
  // center
  elem->set_node(8) = _mesh->node_ptr(node8);

  return elem;
}

SubdomainID
THMMesh::getNextSubdomainId()
{
  SubdomainID id = _next_subdomain_id++;
  return id;
}

BoundaryID
THMMesh::getNextBoundaryId()
{
  BoundaryID id = _next_boundary_id++;
  return id;
}
