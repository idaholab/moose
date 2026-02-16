//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosMesh.h"

#include "MooseMesh.h"

#include "libmesh/reference_elem.h"

namespace Moose::Kokkos
{

void
Mesh::update()
{
  initMap();
  initElement();

  _initialized = true;
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
  _maps->_subdomain_node_ids.clear();

  for (auto subdomain : _mesh.meshSubdomains())
  {
    _maps->_subdomain_id_mapping[subdomain] = _maps->_subdomain_id_mapping.size();
    _maps->_subdomain_node_ids[subdomain];

    dof_id_type begin = libMesh::DofObject::invalid_id;
    dof_id_type eid = libMesh::DofObject::invalid_id;

    for (const auto elem : _mesh.getMesh().active_local_subdomain_elements_ptr_range(subdomain))
    {
      if (!_maps->_elem_type_id_mapping.count(elem->type()))
        _maps->_elem_type_id_mapping[elem->type()] = _maps->_elem_type_id_mapping.size();

      eid = _maps->_local_elem_id_mapping.size();
      _maps->_local_elem_id_mapping[elem] = eid;

      if (begin == libMesh::DofObject::invalid_id)
        begin = eid;

      for (const auto & node : elem->node_ref_range())
      {
        auto [it, emplaced] = _maps->_local_node_id_mapping.try_emplace(&node);

        if (emplaced)
          it->second = _maps->_local_node_id_mapping.size() - 1;

        // A node can belong to multiple subdomains
        if (node.processor_id() == _mesh.processor_id())
          for (auto subdomain : _mesh.getNodeBlockIds(node))
            _maps->_subdomain_node_ids[subdomain].insert(it->second);
      }
    }

    _maps->_subdomain_elem_id_ranges[subdomain] = eid != libMesh::DofObject::invalid_id
                                                      ? std::make_pair(begin, eid + 1)
                                                      : std::make_pair(begin, eid);
  }

  for (auto boundary : _mesh.meshBoundaryIds())
    _maps->_boundary_id_mapping[boundary] = _maps->_boundary_id_mapping.size();
}

void
Mesh::initElement()
{
  // Cache reference element data

  auto num_elem_types = getNumLocalElementTypes();

  _num_sides.create(num_elem_types);
  _num_nodes.create(num_elem_types);
  _num_side_nodes.create(num_elem_types);

  for (auto & [elem_type, elem_type_id] : getElementTypeMap())
  {
    auto elem = &libMesh::ReferenceElem::get(elem_type);

    _num_sides[elem_type_id] = elem->n_sides();
    _num_nodes[elem_type_id] = elem->n_nodes();

    _num_side_nodes[elem_type_id].create(elem->n_sides());

    for (unsigned int side = 0; side < elem->n_sides(); ++side)
      _num_side_nodes[elem_type_id][side] = elem->side_ptr(side)->n_nodes();
  }

  _num_sides.copyToDevice();
  _num_nodes.copyToDevice();
  _num_side_nodes.copyToDeviceNested();

  // Cache element data

  auto num_elems = getNumLocalElements();
  auto num_subdomains = getNumSubdomains();

  _elem_info.create(num_elems);
  _elem_neighbor.create(_mesh.getMaxSidesPerElem(), num_elems);
  _elem_neighbor = libMesh::DofObject::invalid_id;

  for (auto & [elem, eid] : getContiguousElementMap())
  {
    auto elem_type = getElementTypeID(elem);

    _elem_info[eid].type = elem_type;
    _elem_info[eid].id = eid;
    _elem_info[eid].subdomain = getContiguousSubdomainID(elem->subdomain_id());
  }

  _elem_info.copyToDevice();
  _elem_neighbor.copyToDevice();

  _starting_elem_id.create(num_subdomains);

  for (auto subdomain : _mesh.meshSubdomains())
    _starting_elem_id[getContiguousSubdomainID(subdomain)] =
        *getSubdomainContiguousElementIDRange(subdomain).begin();

  _starting_elem_id.copyToDevice();

  // Cache node data

  auto num_nodes = getContiguousNodeMap().size();

  _points.create(num_nodes);
  _nodes.create(_mesh.getMaxNodesPerElem(), num_elems);
  _nodes_face.create(_mesh.getMaxNodesPerSide(), _mesh.getMaxSidesPerElem(), num_elems);
  _boundary_nodes.create(_mesh.meshBoundaryIds().size());

  for (auto & [node, nid] : getContiguousNodeMap())
    _points[nid] = *node;

  for (auto & [elem, eid] : getContiguousElementMap())
  {
    for (unsigned int node = 0; node < elem->n_nodes(); ++node)
      _nodes(node, eid) = getContiguousNodeID(elem->node_ptr(node));

    for (unsigned int side = 0; side < elem->n_sides(); ++side)
      for (unsigned int node = 0; node < elem->side_ptr(side)->n_nodes(); ++node)
        _nodes_face(node, side, eid) = getContiguousNodeID(elem->side_ptr(side)->node_ptr(node));
  }

  for (auto boundary : _mesh.meshBoundaryIds())
  {
    std::set<ContiguousNodeID> nodes;

    for (const auto node : as_range(_mesh.getMesh().bid_nodes_begin(boundary),
                                    _mesh.getMesh().bid_nodes_end(boundary)))
      if (node->processor_id() == _mesh.processor_id())
        nodes.insert(getContiguousNodeID(node));

    _boundary_nodes[getContiguousBoundaryID(boundary)] = nodes;
  }

  _points.copyToDevice();
  _nodes.copyToDevice();
  _nodes_face.copyToDevice();
  _boundary_nodes.copyToDevice();
}

ContiguousSubdomainID
Mesh::getContiguousSubdomainID(const SubdomainID subdomain) const
{
  if (!_maps->_subdomain_id_mapping.count(subdomain))
    mooseError("Subdomain ", subdomain, " does not have Kokkos contiguous subdomain ID");

  return _maps->_subdomain_id_mapping.at(subdomain);
}

ContiguousBoundaryID
Mesh::getContiguousBoundaryID(const BoundaryID boundary) const
{
  if (!_maps->_boundary_id_mapping.count(boundary))
    mooseError("Boundary ", boundary, " does not have Kokkos contiguous boundary ID");

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

ContiguousElementID
Mesh::getContiguousElementID(const Elem * elem) const
{
  mooseAssert(elem, "Element pointer is null");

  if (!_maps->_local_elem_id_mapping.count(elem))
    mooseError("Element ", elem->id(), " does not have Kokkos contiguous element ID");

  return _maps->_local_elem_id_mapping.at(elem);
}

ContiguousNodeID
Mesh::getContiguousNodeID(const Node * node) const
{
  mooseAssert(node, "Node pointer is null");

  if (!_maps->_local_node_id_mapping.count(node))
    mooseError("Node ", node->id(), " does not have Kokkos contiguous node ID");

  return _maps->_local_node_id_mapping.at(node);
}

} // namespace Moose::Kokkos
