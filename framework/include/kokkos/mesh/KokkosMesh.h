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

class MooseMesh;

namespace Moose
{
namespace Kokkos
{

/**
 * The Kokkos object that contains the information of an element
 * The IDs used in Kokkos objects are different from the MOOSE or libMesh IDs and are serialized to
 * start from zero to allow index-based access
 * The serialized IDs are local to each process
 */
struct ElementInfo
{
  /**
   * Element type ID
   */
  unsigned int type;
  /**
   * Element ID
   */
  dof_id_type id;
  /**
   * Subdomain ID
   */
  SubdomainID subdomain;
  /**
   * Number of sides
   */
  unsigned int n_sides;
  /**
   * Number of nodes
   */
  unsigned int n_nodes;
  /**
   * Number of nodes on each side
   */
  unsigned int n_nodes_side[6];
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
   * Update the mesh
   */
  void update();

  /**
   * Get the serialized subdomain ID of a subdomain
   * @param subdomain The MOOSE subdomain ID
   * @returns The subdomain ID
   */
  SubdomainID getSubdomainID(const SubdomainID subdomain) const;
  /**
   * Get the serialized boundary ID of a boundary
   * @param boundary The MOOSE boundary ID
   * @returns The boundary ID
   */
  BoundaryID getBoundaryID(const BoundaryID boundary) const;
  /**
   * Get the serialized element type ID of an element
   * @param elem The libMesh element
   * @returns The element type ID
   */
  unsigned int getElementTypeID(const Elem * elem) const;
  /**
   * Get the serialized element ID of an element
   * @param elem The libMesh element
   * @returns The element ID
   */
  dof_id_type getElementID(const Elem * elem) const;
  /**
   * Get the serialized element type ID map
   * @returns The element type ID map
   */
  const auto & getElementTypeMap() const { return _maps->_elem_type_id_mapping; }
  /**
   * Get the serialized element ID map
   * @returns The element ID map
   */
  const auto & getLocalElementMap() const { return _maps->_local_elem_id_mapping; }
  /**
   * Get the serialized element ID map for a subdomain
   * @param subdomain The MOOSE subdomain ID
   * @returns The element ID map for the subdomain
   */
  const auto & getSubdomainElementMap(const SubdomainID subdomain) const
  {
    return _maps->_subdomain_elem_id_mapping.at(subdomain);
  }
  /**
   * Get the list of serialized element IDs for a subdomain
   * @param subdomain The MOOSE subdomain ID
   * @returns The list of element IDs in the subdomain
   */
  const auto & getSubdomainElementIDs(const SubdomainID subdomain) const
  {
    return _maps->_subdomain_elem_ids.at(subdomain);
  }
  /**
   * Get the serialized node ID of a node
   * @param node The libMesh node
   * @returns The serialized node ID that starts from zero in each process
   */
  dof_id_type getNodeID(const Node * node) const;
  /**
   * Get the serialized node ID map
   * This list contains the nodes of local elements, so some nodes may belong to other processes
   * @returns The node ID map
   */
  const auto & getLocalNodeMap() const { return _maps->_local_node_id_mapping; }
  /**
   * Get the list of serialized node IDs for a subdomain
   * This list strictly contains the nodes local to the current process
   * @param subdomain The MOOSE subdomain ID
   * @returns The list of node IDs in the subdomain
   */
  const auto & getSubdomainNodeIDs(const SubdomainID subdomain) const
  {
    return _maps->_subdomain_node_ids.at(subdomain);
  }

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get the element information object
   * @param elem The element ID
   * @returns The element information object
   */
  KOKKOS_FUNCTION const auto & getElementInfo(dof_id_type elem) const { return _elem_info[elem]; }
  /**
   * Get the neighbor element ID
   * @param elem The element ID
   * @param side The side index
   * @returns The neighbor element ID
   */
  KOKKOS_FUNCTION dof_id_type getNeighbor(dof_id_type elem, unsigned int side) const
  {
    return _elem_neighbor(side, elem);
  }
  /**
   * Get the node ID for an element
   * @param elem The element ID
   * @param node The node index
   * @returns The node ID
   */
  KOKKOS_FUNCTION dof_id_type getNodeID(dof_id_type elem, unsigned int node) const
  {
    return _nodes(node, elem);
  }
  /**
   * Get the node ID for a side
   * @param elem The element ID
   * @param side The side index
   * @param node The node index
   * @returns The node ID
   */
  KOKKOS_FUNCTION dof_id_type getNodeID(dof_id_type elem,
                                        unsigned int side,
                                        unsigned int node) const
  {
    return _nodes_face(node, side, elem);
  }
  /**
   * Get the coordinate of a node
   * @param node The node ID
   * @returns The node coordinate
   */
  KOKKOS_FUNCTION Real3 getNodePoint(dof_id_type node) const { return _points[node]; }
  /**
   * Get whether a node is on a boundary
   * @param node The node ID
   * @param boundary The boundary ID
   * @returns Whether the node is on the boundary
   */
  KOKKOS_FUNCTION bool isBoundaryNode(dof_id_type node, BoundaryID boundary) const;
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
   * The wrapper of host maps
   */
  struct MeshMap
  {
    /**
     * Map from the MOOSE subdomain ID to the serialized subdomain ID
     */
    std::unordered_map<SubdomainID, SubdomainID> _subdomain_id_mapping;
    /**
     * Map from the MOOSE boundary ID to the serialized boundary ID
     */
    std::unordered_map<BoundaryID, BoundaryID> _boundary_id_mapping;
    /**
     * Map from the MOOSE element type to the serialized element type ID
     */
    std::unordered_map<ElemType, unsigned int> _elem_type_id_mapping;
    /**
     * Map from the libMesh element to the serialized element ID
     */
    std::unordered_map<const Elem *, dof_id_type> _local_elem_id_mapping;
    /**
     * Map from the libMesh element to the serialized element ID for each subdomain
     */
    std::unordered_map<SubdomainID, std::unordered_map<const Elem *, dof_id_type>>
        _subdomain_elem_id_mapping;
    /**
     * Map from the libMesh node to the serialized node ID
     * This list contains the nodes of local elements, so some nodes may belong to other processes
     */
    std::unordered_map<const Node *, dof_id_type> _local_node_id_mapping;
    /**
     * List of the serialized element IDs in each subdomain
     */
    std::unordered_map<SubdomainID, std::unordered_set<dof_id_type>> _subdomain_elem_ids;
    /**
     * List of the serialized node IDs in each subdomain
     * This list strictly contains the nodes local to the current process
     */
    std::unordered_map<SubdomainID, std::unordered_set<dof_id_type>> _subdomain_node_ids;
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
   * Neighbor element IDs of each element
   */
  Array2D<dof_id_type> _elem_neighbor;
  /**
   * Node coordinates
   */
  Array<Real3> _points;
  /**
   * Node IDs of each element and side
   */
  ///@{
  Array2D<dof_id_type> _nodes;
  Array3D<dof_id_type> _nodes_face;
  ///@}
  /**
   * Node IDs on each boundary
   */
  Array<Array<dof_id_type>> _boundary_nodes;
};

#ifdef MOOSE_KOKKOS_SCOPE
KOKKOS_FUNCTION inline bool
Mesh::isBoundaryNode(dof_id_type node, BoundaryID boundary) const
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
   * @param assembly The Kokkos mesh
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
    KOKKOS_IF_ON_HOST(return _mesh_host;)

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

} // namespace Kokkos
} // namespace Moose
