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

/**
 * Builds mapping between two aligned subdomains/boundaries
 *
 * This class handles the following cases:
 * - 1D subdomain coupled to 2D boundary
 * - 2D boundary coupled to 2D boundary
 */
class MeshAlignmentBase
{
public:
  /**
   * Constructor
   *
   * @param mesh[in] mesh   Mesh
   */
  MeshAlignmentBase(const MooseMesh & mesh);

  /**
   * Returns the list of element IDs on the primary boundary
   */
  const std::vector<dof_id_type> & getPrimaryElemIDs() const { return _primary_elem_ids; }

protected:
  /**
   * Extracts mesh information from 1D elements
   *
   * @param[in] elem_ids       Vector of element IDs
   * @param[out] elem_points   Vector of element centroids
   * @param[out] node_ids      Vector of node IDs
   * @param[out] node_points   Vector of node points
   */
  void extractFrom1DElements(const std::vector<dof_id_type> & elem_ids,
                             std::vector<Point> & elem_points,
                             std::vector<dof_id_type> & node_ids,
                             std::vector<Point> & node_points) const;

  /**
   * Extracts mesh information from boundary info
   *
   * @param[in] boundary_info  Vector of tuples of element ID and side ID on boundary
   * @param[out] elem_ids      Vector of element IDs
   * @param[out] side_ids      Vector of side IDs
   * @param[out] side_points   Vector of side centroids
   * @param[out] node_ids      Vector of node IDs
   * @param[out] node_points   Vector of node points
   */
  void extractFromBoundaryInfo(
      const std::vector<std::tuple<dof_id_type, unsigned short int>> & boundary_info,
      std::vector<dof_id_type> & elem_ids,
      std::vector<unsigned short int> & side_ids,
      std::vector<Point> & side_points,
      std::vector<dof_id_type> & node_ids,
      std::vector<Point> & node_points) const;

  /// Mesh
  const MooseMesh & _mesh;

  /// List of primary element IDs
  std::vector<dof_id_type> _primary_elem_ids;
  /// List of secondary element IDs
  std::vector<dof_id_type> _secondary_elem_ids;
  /// List of primary element points
  std::vector<Point> _primary_elem_points;
  /// List of secondary element points
  std::vector<Point> _secondary_elem_points;

  /// List of primary side IDs (if any)
  std::vector<unsigned short int> _primary_side_ids;
  /// List of secondary side IDs (if any)
  std::vector<unsigned short int> _secondary_side_ids;

  /// List of primary node IDs
  std::vector<dof_id_type> _primary_node_ids;
  /// List of secondary node IDs
  std::vector<dof_id_type> _secondary_node_ids;
  /// List of primary node points
  std::vector<Point> _primary_node_points;
  /// List of secondary node points
  std::vector<Point> _secondary_node_points;
};
