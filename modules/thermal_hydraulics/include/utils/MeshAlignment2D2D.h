//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/primary/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseMesh.h"

class MooseMesh;

/**
 * Builds mapping between two aligned 2D boundaries
 */
class MeshAlignment2D2D
{
public:
  /**
   * Constructor
   *
   * @param subproblem[in] Subproblem
   */
  MeshAlignment2D2D(const MooseMesh & mesh);

  /**
   * Builds the neighborhood information between primary and secondary side
   *
   * @param primary_boundary_info[in] List of tuples (elem_id, side_id) of the primary side
   * @param secondary_boundary_info[in] List of tuples (elem_id, side_id) of the secondary side
   */
  void initialize(
      const std::vector<std::tuple<dof_id_type, unsigned short int>> & primary_boundary_info,
      const std::vector<std::tuple<dof_id_type, unsigned short int>> & secondary_boundary_info);

  /**
   * Returns true if the primary and secondary boundaries are coincident
   */
  bool meshesAreCoincident() const { return _all_points_are_coincident; }

  /**
   * Returns true if the primary and secondary boundaries are aligned
   */
  bool meshesAreAligned() const { return _meshes_are_aligned; }

  /**
   * Returns the list of element IDs on the primary boundary
   */
  const std::vector<dof_id_type> & getPrimaryBoundaryElemIDs() const { return _primary_elem_ids; }

  /**
   * Gets the neighbor element ID for a given element ID
   *
   * @param[in] elem_id Element ID for which to find the neighbor element ID
   */
  dof_id_type getNeighborElemID(const dof_id_type & elem_id) const;

  /**
   * Returns true if the given node ID has a neighboring node
   *
   * @param[in] node_id Node ID for which to find the neighbor node ID
   */
  bool hasNeighborNode(const dof_id_type & node_id) const;

  /**
   * Gets the neighbor node ID for a given node ID
   *
   * @param[in] node_id Node ID for which to find the neighbor node ID
   */
  dof_id_type getNeighborNodeID(const dof_id_type & node_id) const;

protected:
  /**
   * Extracts various boundary information
   *
   * @param[in] boundary_info  Vector of tuples of element ID and side ID on boundary
   * @param[out] elem_ids      Vector of element IDs
   * @param[out] side_points   Vector of side centroids
   * @param[out] node_ids      Vector of node IDs
   * @param[out] node_points   Vector of node points
   */
  void extractBoundaryInfo(
      const std::vector<std::tuple<dof_id_type, unsigned short int>> & boundary_info,
      std::vector<dof_id_type> & elem_ids,
      std::vector<Point> & side_points,
      std::vector<dof_id_type> & node_ids,
      std::vector<Point> & node_points) const;

  /// Mesh
  const MooseMesh & _mesh;

  /// List of primary element IDs
  std::vector<dof_id_type> _primary_elem_ids;
  /// Map of element ID to neighboring element ID
  std::map<dof_id_type, dof_id_type> _elem_id_map;
  /// Map of node ID to neighboring node ID
  std::map<dof_id_type, dof_id_type> _node_id_map;
  /// Flag that all quadrature points are coincident between boundaries
  bool _all_points_are_coincident;
  /// Flag that meshes are aligned
  bool _meshes_are_aligned;
};
