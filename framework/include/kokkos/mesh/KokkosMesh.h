//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosTypes.h"

#ifdef MOOSE_KOKKOS_SCOPE
#include "KokkosUtils.h"
#endif

using ContiguousSubdomainID = SubdomainID;
using ContiguousBoundaryID = BoundaryID;
using ContiguousElementID = dof_id_type;
using ContiguousNodeID = dof_id_type;

class MooseMesh;

namespace Moose::Kokkos
{

/**
 * The Kokkos object that contains the information of an element
 * The IDs used in Kokkos are different from the MOOSE or libMesh IDs
 * The Kokkos IDs start from zero in each process and are contiguous
 */
struct ElementInfo
{
  /**
   * Element type ID
   */
  unsigned int type = libMesh::invalid_uint;
  /**
   * Contiguous element ID
   */
  ContiguousElementID id = libMesh::DofObject::invalid_id;
  /**
   * Contiguous subdomain ID
   */
  ContiguousSubdomainID subdomain = std::numeric_limits<ContiguousSubdomainID>::max();
};

/**
 * The Kokkos mesh object
 */
class Mesh
{
public:
  /**
   * Constructor
   * @param mesh The MOOSE mesh
   */
  Mesh(const MooseMesh & mesh) : _mesh(mesh) {}
  /**
   * Get the underyling MOOSE mesh
   * @returns The MOOSE mesh
   */
  const MooseMesh & getMesh() { return _mesh; }
  /**
   * Get whether the mesh was initialized
   */
  [[nodiscard]] bool initialized() const { return _initialized; }
  /**
   * Update the mesh
   */
  void update();

  /**
   * Get the number of subdomains
   * @returns The number of subdomains
   */
  auto getNumSubdomains() const { return _maps->_subdomain_id_mapping.size(); }
  /**
   * Get the number of local elements types
   * @returns The number of local element types
   */
  auto getNumLocalElementTypes() const { return _maps->_elem_type_id_mapping.size(); }
  /**
   * Get the number of local elements
   * @returns The number of local elements
   */
  auto getNumLocalElements() const { return _maps->_local_elem_id_mapping.size(); }
  /**
   * Get the number of local elements in a MOOSE subdomain
   * @param subdomain The MOOSE subdomain ID
   * @returns The local number of elements in the subdomain
   */
  auto getNumSubdomainLocalElements(const SubdomainID subdomain) const
  {
    auto range = _maps->_subdomain_elem_id_ranges.at(subdomain);
    return range.second - range.first;
  }
  /**
   * Get the contiguous subdomain ID of a MOOSE subdomain
   * @param subdomain The MOOSE subdomain ID
   * @returns The contiguous subdomain ID
   */
  ContiguousSubdomainID getContiguousSubdomainID(const SubdomainID subdomain) const;
  /**
   * Get the contiguous boundary ID of a boundary
   * @param boundary The MOOSE boundary ID
   * @returns The contiguous boundary ID
   */
  ContiguousBoundaryID getContiguousBoundaryID(const BoundaryID boundary) const;
  /**
   * Get the element type ID of an element
   * @param elem The libMesh element
   * @returns The element type ID
   */
  unsigned int getElementTypeID(const Elem * elem) const;
  /**
   * Get the contiguous element ID of an element
   * @param elem The libMesh element
   * @returns The contiguous element ID
   */
  ContiguousElementID getContiguousElementID(const Elem * elem) const;
  /**
   * Get the element type ID map
   * @returns The element type ID map
   */
  const auto & getElementTypeMap() const { return _maps->_elem_type_id_mapping; }
  /**
   * Get the contiguous element ID map
   * @returns The contiguous element ID map
   */
  const auto & getContiguousElementMap() const { return _maps->_local_elem_id_mapping; }
  /**
   * Get the range of contiguous element IDs for a subdomain
   * @param subdomain The MOOSE subdomain ID
   * @returns The range of contiguous element IDs in the subdomain
   */
  auto getSubdomainContiguousElementIDRange(const SubdomainID subdomain) const
  {
    const auto & range = libmesh_map_find(_maps->_subdomain_elem_id_ranges, subdomain);
    return libMesh::make_range(range.first, range.second);
  }
  /**
   * Get the contiguous node ID of a node
   * @param node The libMesh node
   * @returns The contiguous node ID that starts from zero in each process
   */
  ContiguousNodeID getContiguousNodeID(const Node * node) const;
  /**
   * Get the contiguous node ID map
   * NOTE: This list contains the nodes of local elements, so some nodes may belong to other
   * processes
   * @returns The contiguous node ID map
   */
  const auto & getContiguousNodeMap() const { return _maps->_local_node_id_mapping; }
  /**
   * Get the list of contiguous node IDs for a subdomain
   * NOTE: This list strictly contains the nodes local to the current process
   * @param subdomain The MOOSE subdomain ID
   * @returns The list of contiguous node IDs in the subdomain
   */
  const auto & getSubdomainContiguousNodeIDs(const SubdomainID subdomain) const
  {
    return _maps->_subdomain_node_ids.at(subdomain);
  }

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get the element information object
   * @param elem The contiguous element ID
   * @returns The element information object
   */
  KOKKOS_FUNCTION const auto & getElementInfo(ContiguousElementID elem) const
  {
    return _elem_info[elem];
  }
  /**
   * Get the neighbor contiguous element ID
   * @param elem The contiguous element ID
   * @param side The side index
   * @returns The neighbor contiguous element ID
   */
  KOKKOS_FUNCTION ContiguousElementID getNeighbor(ContiguousElementID elem, unsigned int side) const
  {
    return _elem_neighbor(side, elem);
  }
  /**
   * Get the number of sides of an element type
   * @param elem_type The element type ID
   * @returns The number of sides of the element type
   */
  KOKKOS_FUNCTION unsigned int getNumSides(unsigned int elem_type) const
  {
    return _num_sides[elem_type];
  }
  /**
   * Get the number of nodes of an element type
   * @param elem_type The element type ID
   * @returns The number of nodes of the element type
   */
  KOKKOS_FUNCTION unsigned int getNumNodes(unsigned int elem_type) const
  {
    return _num_nodes[elem_type];
  }
  /**
   * Get the number of nodes on a side of an element type
   * @param elem_type The element type ID
   * @param side The side index
   * @returns The number of nodes on the side of the element type
   */
  KOKKOS_FUNCTION unsigned int getNumNodes(unsigned int elem_type, unsigned int side) const
  {
    return _num_side_nodes[elem_type][side];
  }
  /**
   * Get the starting contiguous element ID of a subdomain
   * @param subdomain The contiguous subdomain ID
   * @returns The starting contiguous element ID
   */
  KOKKOS_FUNCTION dof_id_type getStartingContiguousElementID(ContiguousSubdomainID subdomain) const
  {
    return _starting_elem_id[subdomain];
  }
  /**
   * Get the contiguous node ID for an element
   * @param elem The contiguous element ID
   * @param node The node index
   * @returns The contiguous node ID
   */
  KOKKOS_FUNCTION ContiguousNodeID getContiguousNodeID(ContiguousElementID elem,
                                                       unsigned int node) const
  {
    return _nodes(node, elem);
  }
  /**
   * Get the contiguous node ID for a side
   * @param elem The contiguous element ID
   * @param side The side index
   * @param node The node index
   * @returns The contiguous node ID
   */
  KOKKOS_FUNCTION ContiguousNodeID getContiguousNodeID(ContiguousElementID elem,
                                                       unsigned int side,
                                                       unsigned int node) const
  {
    return _nodes_face(node, side, elem);
  }
  /**
   * Get the coordinate of a node
   * @param node The contiguous node ID
   * @returns The node coordinate
   */
  KOKKOS_FUNCTION Real3 getNodePoint(ContiguousNodeID node) const { return _points[node]; }
  /**
   * Get whether a node is on a boundary
   * @param node The contiguous node ID
   * @param boundary The contiguous boundary ID
   * @returns Whether the node is on the boundary
   */
  KOKKOS_FUNCTION bool isBoundaryNode(ContiguousNodeID node, ContiguousBoundaryID boundary) const;
#endif

private:
  /**
   * Initialize host maps
   */
  void initMap();
  /**
   * Initialize device element data
   */
  void initElement();

  /**
   * Reference of the MOOSE mesh
   */
  const MooseMesh & _mesh;
  /**
   * Flag whether the mesh was initialized
   */
  bool _initialized = false;

  /**
   * The wrapper of host maps
   */
  struct MeshMap
  {
    /**
     * Map from the MOOSE subdomain ID to the contiguous subdomain ID
     */
    std::unordered_map<SubdomainID, ContiguousSubdomainID> _subdomain_id_mapping;
    /**
     * Map from the MOOSE boundary ID to the contiguous boundary ID
     */
    std::unordered_map<BoundaryID, ContiguousBoundaryID> _boundary_id_mapping;
    /**
     * Map from the MOOSE element type to the element type ID
     */
    std::unordered_map<ElemType, unsigned int> _elem_type_id_mapping;
    /**
     * Map from the libMesh element to the contiguous element ID
     */
    std::unordered_map<const Elem *, ContiguousElementID> _local_elem_id_mapping;
    /**
     * Map from the libMesh node to the contiguous node ID
     * This list contains the nodes of local elements, so some nodes may belong to other processes
     */
    std::unordered_map<const Node *, ContiguousNodeID> _local_node_id_mapping;
    /**
     * Range of the contiguous element IDs in each subdomain
     */
    std::unordered_map<SubdomainID, std::pair<ContiguousElementID, ContiguousElementID>>
        _subdomain_elem_id_ranges;
    /**
     * List of the contiguous node IDs in each subdomain
     * This list strictly contains the nodes local to the current process
     */
    std::unordered_map<SubdomainID, std::unordered_set<ContiguousNodeID>> _subdomain_node_ids;
  };
  /**
   * A shared pointer holding all the host maps to avoid deep copy
   */
  std::shared_ptr<MeshMap> _maps;

  /**
   * Element information
   */
  Array<ElementInfo> _elem_info;
  /**
   * Neighbor contiguous element IDs of each element
   */
  Array2D<ContiguousElementID> _elem_neighbor;
  /**
   * Starting contiguous element ID of each subdomain
   */
  Array<ContiguousElementID> _starting_elem_id;
  /**
   * Number of sides of each element type
   */
  Array<unsigned int> _num_sides;
  /**
   * Number of nodes of each element type
   */
  Array<unsigned int> _num_nodes;
  /**
   * number of nodes per side of each element side
   */
  Array<Array<unsigned int>> _num_side_nodes;
  /**
   * Node coordinates
   */
  Array<Real3> _points;
  /**
   * Contiguous node IDs of each element and side
   */
  ///@{
  Array2D<ContiguousNodeID> _nodes;
  Array3D<ContiguousNodeID> _nodes_face;
  ///@}
  /**
   * Contiguous node IDs on each boundary
   */
  Array<Array<ContiguousNodeID>> _boundary_nodes;
};

#ifdef MOOSE_KOKKOS_SCOPE
KOKKOS_FUNCTION inline bool
Mesh::isBoundaryNode(ContiguousNodeID node, ContiguousBoundaryID boundary) const
{
  if (!_boundary_nodes[boundary].size())
    return false;

  auto begin = &_boundary_nodes[boundary].begin();
  auto end = &_boundary_nodes[boundary].end();
  auto target = Utils::find(node, begin, end);

  return target != end;
}
#endif

/**
 * The Kokkos interface that holds the host reference of the Kokkos mesh and copies it to device
 * during parallel dispatch
 * Maintains synchronization between host and device Kokkos mesh and provides access to the
 * appropriate Kokkos mesh depending on the architecture
 */
class MeshHolder
{
public:
  /**
   * Constructor
   * @param mesh The Kokkos mesh
   */
  MeshHolder(const Mesh & mesh) : _mesh_host(mesh), _mesh_device(mesh) {}
  /**
   * Copy constructor
   */
  MeshHolder(const MeshHolder & holder)
    : _mesh_host(holder._mesh_host), _mesh_device(holder._mesh_host)
  {
  }

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get the const reference of the Kokkos mesh
   * @returns The const reference of the Kokkos mesh depending on the architecture this function
   * is being called on
   */
  KOKKOS_FUNCTION const Mesh & kokkosMesh() const
  {
    KOKKOS_IF_ON_HOST(
        if (!_mesh_host.initialized()) mooseError(
            "kokkosMesh() was called too early. Kokkos mesh is available after problem "
            "initialization. Override initialSetup() if you need to setup your object data "
            "using the Kokkos mesh.");

        return _mesh_host;)

    return _mesh_device;
  }
#endif

private:
  /**
   * Host reference of the Kokkos mesh
   */
  const Mesh & _mesh_host;
  /**
   * Device copy of the Kokkos mesh
   */
  const Mesh _mesh_device;
};

} // namespace Moose::Kokkos
