//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUMesh.h"

#include "MooseMesh.h"

namespace Moose
{
namespace Kokkos
{

void
Mesh::update()
{
  initMap();
  initElement();
}

void
Mesh::initMap()
{
  if (!_maps)
    _maps = std::make_shared<MeshMap>();

  _maps->_subdomain_id_mapping.clear();
  _maps->_boundary_id_mapping.clear();
  _maps->_elem_type_id_mapping.clear();
  _maps->_local_elem_id_mapping.clear();
  _maps->_local_node_id_mapping.clear();
  _maps->_subdomain_elem_id_mapping.clear();
  _maps->_subdomain_local_elem_id_mapping.clear();
  _maps->_subdomain_elem_ids.clear();
  _maps->_subdomain_node_ids.clear();

  for (auto subdomain : _mesh.meshSubdomains())
  {
    _maps->_subdomain_id_mapping[subdomain] = _maps->_subdomain_id_mapping.size();
    _maps->_subdomain_elem_id_mapping[subdomain];
    _maps->_subdomain_local_elem_id_mapping[subdomain];
    _maps->_subdomain_elem_ids[subdomain];
    _maps->_subdomain_node_ids[subdomain];

    dof_id_type subdomain_local_id = 0;

    for (const auto elem : _mesh.getMesh().active_local_subdomain_elements_ptr_range(subdomain))
    {
      if (!_maps->_elem_type_id_mapping.count(elem->type()))
        _maps->_elem_type_id_mapping[elem->type()] = _maps->_elem_type_id_mapping.size();

      auto eid = _maps->_local_elem_id_mapping.size();

      _maps->_local_elem_id_mapping[elem] = eid;
      _maps->_subdomain_elem_id_mapping[subdomain][elem] = eid;
      _maps->_subdomain_local_elem_id_mapping[subdomain][elem] = subdomain_local_id++;
      _maps->_subdomain_elem_ids[subdomain].insert(eid);

      for (const auto & node : elem->node_ref_range())
      {
        if (!_maps->_local_node_id_mapping.count(&node))
          _maps->_local_node_id_mapping[&node] = _maps->_local_node_id_mapping.size();

        if (node.processor_id() == _mesh.processor_id())
          _maps->_subdomain_node_ids[subdomain].insert(_maps->_local_node_id_mapping[&node]);
      }
    }
  }

  for (auto boundary : _mesh.meshBoundaryIds())
    _maps->_boundary_id_mapping[boundary] = _maps->_boundary_id_mapping.size();
}

void
Mesh::initElement()
{
  // Cache element data

  auto num_elems = getLocalElementMap().size();

  _elem_info.create(num_elems);
  _elem_neighbor.create(_mesh.getMaxSidesPerElem(), num_elems);
  _elem_neighbor = libMesh::DofObject::invalid_id;

  for (auto subdomain : _mesh.meshSubdomains())
  {
    auto sid = getSubdomainID(subdomain);

    for (auto & [elem, local_id] : getSubdomainLocalElementMap(subdomain))
    {
      auto eid = getElementID(elem);
      auto elem_type = getElementTypeID(elem);

      _elem_info[eid].type = elem_type;
      _elem_info[eid].id = eid;
      _elem_info[eid].local_id = local_id;
      _elem_info[eid].subdomain = sid;
      _elem_info[eid].n_sides = elem->n_sides();
      _elem_info[eid].n_nodes = elem->n_nodes();

      for (unsigned int side = 0; side < elem->n_sides(); ++side)
        _elem_info[eid].n_nodes_side[side] = elem->side_ptr(side)->n_nodes();
    }
  }

  _elem_info.copy();
  _elem_neighbor.copy();

  // Cache node data

  auto num_nodes = getLocalNodeMap().size();

  _points.create(num_nodes);
  _nodes.create(_mesh.getMaxNodesPerElem(), num_elems);
  _nodes_face.create(_mesh.getMaxNodesPerSide(), _mesh.getMaxSidesPerElem(), num_elems);
  _boundary_nodes.create(_mesh.meshBoundaryIds().size());

  for (auto & [node, nid] : getLocalNodeMap())
    _points[nid] = *node;

  for (auto & [elem, eid] : getLocalElementMap())
  {
    for (unsigned int node = 0; node < elem->n_nodes(); ++node)
      _nodes(node, eid) = getNodeID(elem->node_ptr(node));

    for (unsigned int side = 0; side < elem->n_sides(); ++side)
      for (unsigned int node = 0; node < elem->side_ptr(side)->n_nodes(); ++node)
        _nodes_face(node, side, eid) = getNodeID(elem->side_ptr(side)->node_ptr(node));
  }

  for (auto boundary : _mesh.meshBoundaryIds())
  {
    std::set<dof_id_type> nodes;

    for (const auto node : as_range(_mesh.getMesh().bid_nodes_begin(boundary),
                                    _mesh.getMesh().bid_nodes_end(boundary)))
      if (node->processor_id() == _mesh.processor_id())
        nodes.insert(getNodeID(node));

    _boundary_nodes[getBoundaryID(boundary)] = nodes;
  }

  _points.copy();
  _nodes.copy();
  _nodes_face.copy();
  _boundary_nodes.copy();
}

SubdomainID
Mesh::getSubdomainID(const SubdomainID subdomain) const
{
  if (!_maps->_subdomain_id_mapping.count(subdomain))
    mooseError("Subdomain ", subdomain, " does not have Kokkos subdomain ID");

  return _maps->_subdomain_id_mapping.at(subdomain);
}

BoundaryID
Mesh::getBoundaryID(const BoundaryID boundary) const
{
  if (!_maps->_boundary_id_mapping.count(boundary))
    mooseError("Boundary ", boundary, " does not have Kokkos boundary ID");

  return _maps->_boundary_id_mapping.at(boundary);
}

unsigned int
Mesh::getElementTypeID(const Elem * elem) const
{
  mooseAssert(elem, "Element pointer is null");

  if (!_maps->_elem_type_id_mapping.count(elem->type()))
    mooseError("Element type ", elem->type(), " does not have Kokkos element type ID");

  return _maps->_elem_type_id_mapping.at(elem->type());
}

dof_id_type
Mesh::getElementID(const Elem * elem) const
{
  mooseAssert(elem, "Element pointer is null");

  if (!_maps->_local_elem_id_mapping.count(elem))
    mooseError("Element ", elem->id(), " does not have Kokkos element ID");

  return _maps->_local_elem_id_mapping.at(elem);
}

dof_id_type
Mesh::getSubdomainLocalElementID(const Elem * elem) const
{
  mooseAssert(elem, "Element pointer is null");

  if (!_maps->_subdomain_local_elem_id_mapping.at(elem->subdomain_id()).count(elem))
    mooseError("Element ", elem->id(), " does not have Kokkos element ID");

  return _maps->_subdomain_local_elem_id_mapping.at(elem->subdomain_id()).at(elem);
}

dof_id_type
Mesh::getNodeID(const Node * node) const
{
  mooseAssert(node, "Node pointer is null");

  if (!_maps->_local_node_id_mapping.count(node))
    mooseError("Node ", node->id(), " does not have Kokkos node ID");

  return _maps->_local_node_id_mapping.at(node);
}

} // namespace Kokkos
} // namespace Moose
