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

void
GPUMesh::update()
{
  initMap();
  initElement();
}

void
GPUMesh::initMap()
{
  if (!_maps)
    _maps = std::make_shared<GPUMeshMap>();

  _maps->_subdomain_id_mapping.clear();
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
        if (node.processor_id() == _mesh.processor_id())
        {
          if (!_maps->_local_node_id_mapping.count(&node))
            _maps->_local_node_id_mapping[&node] = _maps->_local_node_id_mapping.size();

          _maps->_subdomain_node_ids[subdomain].insert(_maps->_local_node_id_mapping[&node]);
        }
    }
  }
}

void
GPUMesh::initElement()
{
  // Cache element data

  auto num_elems = getGPULocalElementMap().size();

  _elem_info.create(num_elems);
  _elem_neighbor.create(_mesh.getMaxSidesPerElem(), num_elems);
  _elem_neighbor = libMesh::DofObject::invalid_id;

  for (auto subdomain : _mesh.meshSubdomains())
  {
    auto sid = getGPUSubdomainID(subdomain);

    for (auto & [elem, local_id] : getGPUSubdomainLocalElementMap(subdomain))
    {
      auto eid = getGPUElementID(elem);
      auto elem_type = getGPUElementTypeID(elem);

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

  auto num_nodes = getGPULocalNodeMap().size();

  _points.create(num_nodes);
  _nodes.create(_mesh.getMaxNodesPerElem(), num_elems);
  _nodes_face.create(_mesh.getMaxNodesPerSide(), _mesh.getMaxSidesPerElem(), num_elems);

  for (auto & [node, nid] : getGPULocalNodeMap())
    _points[nid] = *node;

  for (auto & [elem, eid] : getGPULocalElementMap())
  {
    for (unsigned int node = 0; node < elem->n_nodes(); ++node)
      _nodes(node, eid) = getGPUNodeID(elem->node_ptr(node));

    for (unsigned int side = 0; side < elem->n_sides(); ++side)
      for (unsigned int node = 0; node < elem->side_ptr(side)->n_nodes(); ++node)
        _nodes_face(node, side, eid) = getGPUNodeID(elem->side_ptr(side)->node_ptr(node));
  }

  _points.copy();
  _nodes.copy();
  _nodes_face.copy();
}

SubdomainID
GPUMesh::getGPUSubdomainID(const SubdomainID subdomain) const
{
  if (!_maps->_subdomain_id_mapping.count(subdomain))
    mooseError("Subdomain ", subdomain, " does not have GPU subdomain ID");

  return _maps->_subdomain_id_mapping.at(subdomain);
}

unsigned int
GPUMesh::getGPUElementTypeID(const Elem * elem) const
{
  mooseAssert(elem, "Element pointer is null");

  if (!_maps->_elem_type_id_mapping.count(elem->type()))
    mooseError("Element type ", elem->type(), " does not have GPU element type ID");

  return _maps->_elem_type_id_mapping.at(elem->type());
}

dof_id_type
GPUMesh::getGPUElementID(const Elem * elem) const
{
  mooseAssert(elem, "Element pointer is null");

  if (!_maps->_local_elem_id_mapping.count(elem))
    mooseError("Element ", elem->id(), " does not have GPU element ID");

  return _maps->_local_elem_id_mapping.at(elem);
}

dof_id_type
GPUMesh::getGPUSubdomainLocalElementID(const Elem * elem) const
{
  mooseAssert(elem, "Element pointer is null");

  if (!_maps->_subdomain_local_elem_id_mapping.at(elem->subdomain_id()).count(elem))
    mooseError("Element ", elem->id(), " does not have GPU element ID");

  return _maps->_subdomain_local_elem_id_mapping.at(elem->subdomain_id()).at(elem);
}

dof_id_type
GPUMesh::getGPUNodeID(const Node * node) const
{
  mooseAssert(node, "Node pointer is null");

  if (!_maps->_local_node_id_mapping.count(node))
    mooseError("Node ", node->id(), " does not have GPU node ID");

  return _maps->_local_node_id_mapping.at(node);
}
