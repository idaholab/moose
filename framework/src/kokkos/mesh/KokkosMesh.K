//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosMesh.h"

#include "Assembly.h"
#include "MooseMesh.h"

#include "libmesh/elem_side_builder.h"
#include "libmesh/reference_elem.h"

namespace Moose::Kokkos
{

Mesh::Mesh(MooseMesh & mesh) : _mesh(&mesh), _is_displaced(mesh.isDisplaced()) {}

void
Mesh::initMap()
{
  if (!_maps)
    _maps = std::make_shared<MeshMap>();

  _dimension = _mesh->dimension();

  if (!_mesh->getMesh().has_elem_integer("kokkos_contiguous_elem_id"))
    _elem_id_integer = _mesh->getMesh().add_elem_integer("kokkos_contiguous_elem_id");
  else
    _elem_id_integer = _mesh->getMesh().get_elem_integer_index("kokkos_contiguous_elem_id");

  if (!_mesh->getMesh().has_node_integer("kokkos_contiguous_node_id"))
    _node_id_integer = _mesh->getMesh().add_node_integer("kokkos_contiguous_node_id");
  else
    _node_id_integer = _mesh->getMesh().get_node_integer_index("kokkos_contiguous_node_id");

  _num_local_elems = 0;
  _num_ghost_elems = 0;
  _num_local_nodes = 0;

  _maps->subdomain_id_mapping.clear();
  _maps->boundary_id_mapping.clear();
  _maps->elem_type_id_mapping.clear();
  _maps->subdomain_elem_id_ranges.clear();
  _maps->subdomain_node_ids.clear();
  _maps->boundary_node_ids.clear();

  std::set<std::pair<ElemType, unsigned int>> elem_types;
  std::unordered_set<Node *> ghost_nodes;
  std::unordered_set<dof_id_type> ghost_elem_ids;
  std::vector<Node *> local_nodes;
  std::vector<Elem *> ghost_elems;

  for (const auto subdomain : _mesh->meshSubdomains())
  {
    _maps->subdomain_id_mapping[subdomain] = _maps->subdomain_id_mapping.size();
    _maps->subdomain_node_ids[subdomain];

    dof_id_type begin = libMesh::DofObject::invalid_id;
    dof_id_type eid = libMesh::DofObject::invalid_id;

    for (auto elem : _mesh->getMesh().active_local_subdomain_elements_ptr_range(subdomain))
    {
      elem_types.emplace(elem->type(), elem->p_level());

      eid = _num_local_elems;
      elem->set_extra_integer(_elem_id_integer, _num_local_elems++);

      if (begin == libMesh::DofObject::invalid_id)
        begin = eid;

      for (auto & node : elem->node_ref_range())
        if (node.processor_id() != _mesh->processor_id())
          ghost_nodes.insert(&node);
    }

    _maps->subdomain_elem_id_ranges[subdomain] = eid != libMesh::DofObject::invalid_id
                                                     ? std::make_pair(begin, eid + 1)
                                                     : std::make_pair(begin, eid);
  }

  for (const auto boundary : _mesh->meshBoundaryIds())
  {
    _maps->boundary_id_mapping[boundary] = _maps->boundary_id_mapping.size();
    _maps->boundary_node_ids[boundary];
  }

  // Enumerate one off-process neighbor layer for element-side geometry data.
  if (_needs_element_side_geometry)
    for (const auto elem : _mesh->getMesh().active_local_element_ptr_range())
      for (const auto side : elem->side_index_range())
      {
        const auto neighbor = elem->neighbor_ptr(side);
        if (neighbor && neighbor != libMesh::remote_elem &&
            neighbor->processor_id() != _mesh->processor_id() &&
            ghost_elem_ids.insert(neighbor->id()).second)
        {
          const auto id = _num_local_elems + _num_ghost_elems++;
          ghost_elems.push_back(neighbor);
          neighbor->set_extra_integer(_elem_id_integer, id);
          elem_types.emplace(neighbor->type(), neighbor->p_level());
        }
      }

  // Setup nodes

  for (const auto & type : elem_types)
    _maps->elem_type_id_mapping[type] = _maps->elem_type_id_mapping.size();

  local_nodes.insert(local_nodes.end(), _mesh->localNodesBegin(), _mesh->localNodesEnd());
  local_nodes.insert(local_nodes.end(), ghost_nodes.begin(), ghost_nodes.end());

  _local_nodes.copyVector<true, false>(local_nodes);
  _ghost_elems.copyVector<true, false>(ghost_elems);

  for (const auto node : _local_nodes)
  {
    node->set_extra_integer(_node_id_integer, _num_local_nodes);

    if (node->processor_id() == _mesh->processor_id())
    {
      for (auto subdomain : _mesh->getNodeBlockIds(*node))
        _maps->subdomain_node_ids[subdomain].push_back(_num_local_nodes);
    }

    ++_num_local_nodes;
  }

  for (const auto bnd_node : as_range(_mesh->bndNodesBegin(), _mesh->bndNodesEnd()))
    if (bnd_node->_node->processor_id() == _mesh->processor_id())
      _maps->boundary_node_ids[bnd_node->_bnd_id].insert(getContiguousNodeID(bnd_node->_node));
}

void
Mesh::initElement()
{
  // Cache reference element data

  const auto num_elem_types = getNumLocalElementTypes();

  _num_sides.create(num_elem_types);
  _num_nodes.create(num_elem_types);
  _num_side_nodes.create(num_elem_types);
  _local_side_node.create(num_elem_types);

  for (auto & [elem_type_and_p_level, elem_type_id] : getElementTypeMap())
  {
    const auto [elem_type, _] = elem_type_and_p_level;
    auto elem = &libMesh::ReferenceElem::get(elem_type);

    _num_sides[elem_type_id] = elem->n_sides();
    _num_nodes[elem_type_id] = elem->n_nodes();

    _num_side_nodes[elem_type_id].create(elem->n_sides());
    _local_side_node[elem_type_id].create(elem->n_nodes(), elem->n_sides());

    for (const auto side : elem->side_index_range())
    {
      const auto side_ptr = elem->build_side_ptr(side);

      _num_side_nodes[elem_type_id][side] = side_ptr->n_nodes();

      for (const auto node : side_ptr->node_index_range())
        _local_side_node[elem_type_id](node, side) = elem->local_side_node(side, node);
    }

    _num_side_nodes[elem_type_id].moveToDevice();
    _local_side_node[elem_type_id].moveToDevice();
  }

  _num_sides.moveToDevice();
  _num_nodes.moveToDevice();
  _num_side_nodes.copyToDevice();
  _local_side_node.copyToDevice();

  // Cache element data

  const auto num_local_plus_one_neighbor_layer_elems =
      getNumLocalAndPossiblyOneNeighborLayerGhostElements();
  const auto num_elems = getNumLocalElements();
  const auto num_subdomains = getNumSubdomains();

  _elem_info.create(num_local_plus_one_neighbor_layer_elems);
  _elem_neighbor.create(_mesh->getMaxSidesPerElem(), num_elems);
  _extra_elem_ids.create(num_elems, _mesh->getMesh().n_elem_integers());
  _starting_elem_id.create(num_subdomains);

  _elem_neighbor = libMesh::DofObject::invalid_id;
  _extra_elem_ids = libMesh::DofObject::invalid_id;

  for (const auto subdomain : _mesh->meshSubdomains())
  {
    const auto sid = getContiguousSubdomainID(subdomain);

    for (const auto elem : _mesh->getMesh().active_local_subdomain_elements_ptr_range(subdomain))
    {
      const auto elem_type = getElementTypeID(elem);
      const auto eid = getContiguousElementID(elem);

      _elem_info[eid].type = elem_type;
      _elem_info[eid].id = eid;
      _elem_info[eid].subdomain = sid;

      for (const auto i : make_range(_mesh->getMesh().n_elem_integers()))
        _extra_elem_ids(eid, i) = elem->get_extra_integer(i);
    }

    _starting_elem_id[sid] = *getSubdomainContiguousElementIDRange(subdomain).begin();
  }

  // Cache ElementInfo for one-layer ghost neighbors so side-based computations can identify
  // off-process neighbor element subdomains.
  for (const auto ghost_elem : _ghost_elems)
  {
    const auto ghost_eid = getContiguousElementID(ghost_elem);
    _elem_info[ghost_eid].type = getElementTypeID(ghost_elem);
    _elem_info[ghost_eid].id = ghost_eid;
    _elem_info[ghost_eid].subdomain = getContiguousSubdomainID(ghost_elem->subdomain_id());
  }

  _elem_info.moveToDevice();
  _extra_elem_ids.moveToDevice();
  _starting_elem_id.moveToDevice();

  if (_needs_element_side_geometry)
  {
    _side_boundary_id.create(_mesh->getMaxSidesPerElem(), num_elems);
    _side_boundary_id = Moose::INVALID_BOUNDARY_ID;

    for (const auto elem : _mesh->getMesh().active_local_element_ptr_range())
    {
      const auto eid = getContiguousElementID(elem);

      for (const auto side : elem->side_index_range())
      {
        const auto neighbor = elem->neighbor_ptr(side);
        if (neighbor && neighbor != libMesh::remote_elem)
          _elem_neighbor(side, eid) = getContiguousElementID(neighbor);

        const auto boundary_ids = _mesh->getBoundaryIDs(elem, side);
        if (boundary_ids.size() > 1)
          mooseError("Kokkos element-side topology does not support multiple boundary IDs on a "
                     "single side. Element ",
                     eid,
                     " side ",
                     side,
                     " has ",
                     boundary_ids.size(),
                     " boundary IDs.");
        if (!boundary_ids.empty())
          _side_boundary_id(side, eid) = boundary_ids.front();
      }
    }

    _elem_neighbor.moveToDevice();
    _side_boundary_id.moveToDevice();
  }

  // Cache node topology

  _nodes.create(_mesh->getMaxNodesPerElem(), num_local_plus_one_neighbor_layer_elems);
  _boundary_nodes.create(_mesh->meshBoundaryIds().size());

  for (const auto elem : _mesh->getMesh().active_local_element_ptr_range())
  {
    const auto eid = getContiguousElementID(elem);

    for (const auto node : elem->node_index_range())
      _nodes(node, eid) = getContiguousNodeID(elem->node_ptr(node));
  }

  for (const auto boundary : _mesh->meshBoundaryIds())
    _boundary_nodes[getContiguousBoundaryID(boundary)].copySet<false, true>(
        _maps->boundary_node_ids[boundary]);

  _nodes.moveToDevice();
  _boundary_nodes.copyToDevice();
}

ContiguousSubdomainID
Mesh::getContiguousSubdomainID(const SubdomainID subdomain) const
{
  return libmesh_map_find(_maps->subdomain_id_mapping, subdomain);
}

ContiguousBoundaryID
Mesh::getContiguousBoundaryID(const BoundaryID boundary) const
{
  return libmesh_map_find(_maps->boundary_id_mapping, boundary);
}

unsigned int
Mesh::getElementTypeID(const Elem * elem) const
{
  mooseAssert(elem, "Element pointer is null");

  return libmesh_map_find(_maps->elem_type_id_mapping,
                          std::make_pair(elem->type(), elem->p_level()));
}

ContiguousElementID
Mesh::getContiguousElementID(const Elem * elem) const
{
  mooseAssert(elem, "Element pointer is null");
  return elem->get_extra_integer(_elem_id_integer);
}

ContiguousNodeID
Mesh::getContiguousNodeID(const Node * node) const
{
  mooseAssert(node, "Node pointer is null");
  return node->get_extra_integer(_node_id_integer);
}

void
Mesh::init(const Mesh & mesh)
{
  auto * const moose_mesh = _mesh;
  *this = mesh;
  _mesh = moose_mesh;

  if (!_mesh->getMesh().has_elem_integer("kokkos_contiguous_elem_id"))
    _elem_id_integer = _mesh->getMesh().add_elem_integer("kokkos_contiguous_elem_id");
  else
    _elem_id_integer = _mesh->getMesh().get_elem_integer_index("kokkos_contiguous_elem_id");

  if (!_mesh->getMesh().has_node_integer("kokkos_contiguous_node_id"))
    _node_id_integer = _mesh->getMesh().add_node_integer("kokkos_contiguous_node_id");
  else
    _node_id_integer = _mesh->getMesh().get_node_integer_index("kokkos_contiguous_node_id");

  for (auto elem : _mesh->getMesh().active_local_element_ptr_range())
  {
    const auto reference_elem = mesh._mesh->elemPtr(elem->id());
    elem->set_extra_integer(_elem_id_integer,
                            reference_elem->get_extra_integer(mesh._elem_id_integer));
  }

  _ghost_elems.createHost(mesh._ghost_elems.size());
  for (const auto i : index_range(mesh._ghost_elems))
  {
    const auto reference_elem = mesh._ghost_elems[i];
    _ghost_elems[i] = _mesh->elemPtr(reference_elem->id());
    _ghost_elems[i]->set_extra_integer(_elem_id_integer,
                                       reference_elem->get_extra_integer(mesh._elem_id_integer));
  }

  _local_nodes.createHost(mesh._local_nodes.size());
  for (const auto i : index_range(mesh._local_nodes))
  {
    const auto reference_node = mesh._local_nodes[i];
    _local_nodes[i] = _mesh->nodePtr(reference_node->id());
    _local_nodes[i]->set_extra_integer(_node_id_integer,
                                       reference_node->get_extra_integer(mesh._node_id_integer));
  }

  updateGeometry();
}

void
Mesh::update()
{
  initMap();
  initElement();

  updateGeometry();
}

void
Mesh::updateGeometry()
{
  mooseAssert(_maps, "Kokkos mesh data must be initialized before updating geometry.");

  _is_displaced = _mesh->isDisplaced();

  initNode();

  _element_geometry_initialized = false;
  _element_side_geometry_initialized = false;

  if (_needs_element_geometry)
    initElementGeometry();
  if (_needs_element_side_geometry)
    initElementSideGeometry();

  _initialized = true;
}

void
Mesh::initNode()
{
  _points.create(getNumLocalNodes());

  for (const auto node : getLocalNodes())
    _points[getContiguousNodeID(node)] = *node;

  _points.moveToDevice();
}

void
Mesh::initElementGeometry()
{
  const auto num_local_elems = getNumLocalElements();

  _elem_volume.create(num_local_elems);
  _elem_centroid.create(num_local_elems);

  _elem_volume = 0;
  _elem_centroid = Real3();

  for (const auto elem : _mesh->getMesh().active_local_element_ptr_range())
  {
    const auto eid = getContiguousElementID(elem);
    const auto elem_centroid = elem->vertex_average();
    Real coord_factor;
    ::coordTransformFactor(*_mesh, elem->subdomain_id(), elem_centroid, coord_factor);

    _elem_volume[eid] = elem->volume() * coord_factor;
    _elem_centroid[eid] = elem_centroid;
  }

  _elem_volume.moveToDevice();
  _elem_centroid.moveToDevice();

  _element_geometry_initialized = true;
}

void
Mesh::initElementSideGeometry()
{
  mooseAssert(_element_geometry_initialized,
              "initElementGeometry() must be called before initElementSideGeometry().");

  const auto num_elems = getNumLocalElements();
  const auto max_sides = _mesh->getMaxSidesPerElem();

  _side_area.create(max_sides, num_elems);
  _side_centroid.create(max_sides, num_elems);
  _side_normal.create(max_sides, num_elems);
  _elem_centroid_to_side_centroid.create(max_sides, num_elems);
  _elem_centroid_to_side_centroid_distance.create(max_sides, num_elems);
  _elem_centroid_to_neighbor_centroid.create(max_sides, num_elems);
  _elem_centroid_to_neighbor_centroid_distance.create(max_sides, num_elems);

  _side_area = 0;
  _side_centroid = Real3();
  _side_normal = Real3();
  _elem_centroid_to_side_centroid = Real3();
  _elem_centroid_to_side_centroid_distance = 0;
  _elem_centroid_to_neighbor_centroid = Real3();
  _elem_centroid_to_neighbor_centroid_distance = 0;

  libMesh::ElemSideBuilder side_builder;

  for (const auto elem : _mesh->getMesh().active_local_element_ptr_range())
  {
    const auto eid = getContiguousElementID(elem);
    const auto elem_centroid = elem->vertex_average();

    for (const auto side : elem->side_index_range())
    {
      auto & face = side_builder(*elem, side);
      const auto face_centroid = face.vertex_average();
      const auto * const neighbor = elem->neighbor_ptr(side);
      Real coord_factor;
      mooseAssert(!neighbor || neighbor != libMesh::remote_elem,
                  "First layer neighbors should be ghosted");
      ::coordTransformFactor(*_mesh,
                             elem->subdomain_id(),
                             face_centroid,
                             coord_factor,
                             neighbor ? neighbor->subdomain_id()
                                      : libMesh::Elem::invalid_subdomain_id);

      if (neighbor)
        mooseAssert(
            getContiguousElementID(neighbor) != libMesh::DofObject::invalid_id,
            "We should have determined a contiguous element ID for our local element neighbors");

      const auto elem_centroid_to_side_centroid = face_centroid - elem_centroid;
      const auto elem_centroid_to_neighbor_centroid =
          neighbor ? neighbor->vertex_average() - elem_centroid : Real3();

      _side_area(side, eid) = face.volume() * coord_factor;
      _side_centroid(side, eid) = face_centroid;
      _side_normal(side, eid) = elem->side_vertex_average_normal(side);
      _elem_centroid_to_side_centroid(side, eid) = elem_centroid_to_side_centroid;
      _elem_centroid_to_side_centroid_distance(side, eid) = elem_centroid_to_side_centroid.norm();
      _elem_centroid_to_neighbor_centroid(side, eid) = elem_centroid_to_neighbor_centroid;
      _elem_centroid_to_neighbor_centroid_distance(side, eid) =
          neighbor ? elem_centroid_to_neighbor_centroid.norm() : Real(0);
    }
  }

  _side_area.moveToDevice();
  _side_centroid.moveToDevice();
  _side_normal.moveToDevice();
  _elem_centroid_to_side_centroid.moveToDevice();
  _elem_centroid_to_side_centroid_distance.moveToDevice();
  _elem_centroid_to_neighbor_centroid.moveToDevice();
  _elem_centroid_to_neighbor_centroid_distance.moveToDevice();

  _element_side_geometry_initialized = true;
}

} // namespace Moose::Kokkos
