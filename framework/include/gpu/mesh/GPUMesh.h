//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUTypes.h"

class MooseMesh;

namespace Moose
{
namespace Kokkos
{

struct ElementInfo
{
  // Element type ID
  unsigned int type;
  // Element ID
  dof_id_type id;
  // Subdomain-local element ID
  dof_id_type local_id;
  // Subdomain ID
  SubdomainID subdomain;
  // Number of sides
  unsigned int n_sides;
  // Number of nodes
  unsigned int n_nodes;
  // Number of nodes per side
  unsigned int n_nodes_side[6];
};

class Mesh
{
public:
  // Constructor
  Mesh(const MooseMesh & mesh) : _mesh(mesh) {}
  // Get the underyling MooseMesh
  const MooseMesh & getMesh() { return _mesh; }
  // Update the mesh information
  void update();

private:
  // Initialize maps
  void initMap();
  // Initialize elements
  void initElement();

public:
  /**
   * Get GPU subdomain ID of a subdomain (local to each process)
   */
  SubdomainID getSubdomainID(const SubdomainID subdomain) const;

  /**
   * Get GPU element type ID of an element pointer (local to each process)
   */
  unsigned int getElementTypeID(const Elem * elem) const;

  /**
   * Get GPU element ID of an element pointer (local to each process)
   */
  dof_id_type getElementID(const Elem * elem) const;

  /**
   * Get GPU subdomain-local element ID of an element pointer (local to each process)
   */
  dof_id_type getSubdomainLocalElementID(const Elem * elem) const;

  /**
   * Get element type to GPU element type ID map (local to each process)
   */
  const auto & getElementTypeMap() const { return _maps->_elem_type_id_mapping; }

  /**
   * Get element pointer to GPU local element ID map (local to each process)
   */
  const auto & getLocalElementMap() const { return _maps->_local_elem_id_mapping; }

  /**
   * Get element pointer to GPU local element ID map for a subdomain (local to each process)
   */
  const auto & getSubdomainElementMap(const SubdomainID subdomain) const
  {
    return _maps->_subdomain_elem_id_mapping.at(subdomain);
  }

  /**
   * Get element pointer to GPU subdomain-local element ID map (local to each process)
   */
  const auto & getSubdomainLocalElementMap(const SubdomainID subdomain) const
  {
    return _maps->_subdomain_local_elem_id_mapping.at(subdomain);
  }

  /**
   * Get the list of GPU local element IDs for a subdomain (local to each process)
   */
  const auto & getSubdomainElementIDs(const SubdomainID subdomain) const
  {
    return _maps->_subdomain_elem_ids.at(subdomain);
  }

  /**
   * Get GPU node ID of a node pointer (local to each process)
   */
  dof_id_type getNodeID(const Node * node) const;

  /**
   * Get node pointer to GPU local node ID map (local to each process)
   */
  const auto & getLocalNodeMap() const { return _maps->_local_node_id_mapping; }

  /**
   * Get the list of GPU local node IDs for a subdomain (local to each process)
   */
  const auto & getSubdomainNodeIDs(const SubdomainID subdomain) const
  {
    return _maps->_subdomain_node_ids.at(subdomain);
  }

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get the element information
   */
  KOKKOS_FUNCTION auto getElementInfo(dof_id_type elem) const { return _elem_info[elem]; }
  /**
   * Get the neighbor local element ID
   */
  KOKKOS_FUNCTION auto getNeighbor(dof_id_type elem, unsigned int side) const
  {
    return _elem_neighbor(side, elem);
  }
  /**
   * Get the local node ID
   */
  KOKKOS_FUNCTION auto getNodeID(dof_id_type elem, unsigned int node) const
  {
    return _nodes(node, elem);
  }
  KOKKOS_FUNCTION auto getNodeID(dof_id_type elem, unsigned int side, unsigned int node) const
  {
    return _nodes_face(node, side, elem);
  }
  /**
   * Get the node coordinate
   */
  KOKKOS_FUNCTION auto getNodePoint(dof_id_type node) const { return _points[node]; }
#endif

private:
  // Pointer to the MooseMesh
  const MooseMesh & _mesh;
  // Collection of maps
  struct MeshMap
  {
    // Map from subdomain to serialized GPU subdomain ID (local to each process)
    std::map<SubdomainID, SubdomainID> _subdomain_id_mapping;
    // Map from element type to serialized GPU element type ID (local to each process)
    std::map<ElemType, unsigned int> _elem_type_id_mapping;
    // Map from element pointer to serialized GPU local element ID (local to each process)
    std::map<const Elem *, dof_id_type> _local_elem_id_mapping;
    // Map from element pointer to serialized GPU element ID for each subdomain (local to each
    // process)
    std::map<SubdomainID, std::map<const Elem *, dof_id_type>> _subdomain_elem_id_mapping;
    // Map from element pointer to serialized GPU subdomain-local element ID (local to each process)
    std::map<SubdomainID, std::map<const Elem *, dof_id_type>> _subdomain_local_elem_id_mapping;
    // Map from node pointer to serialized GPU local node ID (local to each process)
    std::map<const Node *, dof_id_type> _local_node_id_mapping;
    // List of serialized GPU local element IDs for each subdomain (local to each process)
    std::map<SubdomainID, std::set<dof_id_type>> _subdomain_elem_ids;
    // List of serialized GPU local node IDs for each subdomain (local to each process)
    std::map<SubdomainID, std::set<dof_id_type>> _subdomain_node_ids;
  };
  // A shared pointer holding all maps to avoid deep copy
  std::shared_ptr<MeshMap> _maps;

private:
  // Element information
  Array<ElementInfo> _elem_info;
  // Neighbor elements of each element
  Array2D<dof_id_type> _elem_neighbor;
  // Physical node points
  Array<Real3> _points;
  // Node indices for each element and side
  Array2D<dof_id_type> _nodes;
  Array3D<dof_id_type> _nodes_face;
};

class MeshHolder
{
private:
  // Copy of Kokkos mesh
  Mesh _mesh_device;
  // Reference to Kokkos mesh
  const Mesh & _mesh_host;

#ifdef MOOSE_KOKKOS_SCOPE
public:
  KOKKOS_FUNCTION const Mesh & kokkosMesh() const
  {
    KOKKOS_IF_ON_HOST(return _mesh_host;)
    KOKKOS_IF_ON_DEVICE(return _mesh_device;)
  }
#endif

public:
  MeshHolder(const Mesh & mesh) : _mesh_device(mesh), _mesh_host(mesh) {}
  MeshHolder(const MeshHolder & holder)
    : _mesh_device(holder._mesh_host), _mesh_host(holder._mesh_host)
  {
  }
};

} // namespace Kokkos
} // namespace Moose
