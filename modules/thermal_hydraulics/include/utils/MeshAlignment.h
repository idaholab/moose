//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/primary/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshAlignmentBase.h"

class Assembly;

/**
 * Builds mapping between two aligned subdomains/boundaries
 *
 * This class handles cases where there is a 1:1 mapping between elements on the
 * primary and secondary meshes:
 * - 1D subdomain coupled to 2D boundary
 * - 2D boundary coupled to 2D boundary
 */
class MeshAlignment : public MeshAlignmentBase
{
public:
  /**
   * Constructor
   *
   * @param mesh[in] mesh   Mesh
   */
  MeshAlignment(const MooseMesh & mesh);

  /**
   * Extracts mesh information and builds the mapping
   *
   * This version is for 1D elements coupled to a 2D boundary.
   *
   * @param primary_elem_ids[in] List of primary element IDs
   * @param secondary_boundary_info[in] List of tuples (elem_id, side_id) of the secondary side
   */
  void initialize(
      const std::vector<dof_id_type> & primary_elem_ids,
      const std::vector<std::tuple<dof_id_type, unsigned short int>> & secondary_boundary_info);

  /**
   * Extracts mesh information and builds the mapping
   *
   * This version is for a 2D boundary coupled to another 2D boundary.
   *
   * @param primary_boundary_info[in] List of tuples (elem_id, side_id) of the primary side
   * @param secondary_boundary_info[in] List of tuples (elem_id, side_id) of the secondary side
   */
  void initialize(
      const std::vector<std::tuple<dof_id_type, unsigned short int>> & primary_boundary_info,
      const std::vector<std::tuple<dof_id_type, unsigned short int>> & secondary_boundary_info);

  /**
   * Builds the map used for getting the coupled quadrature point index
   *
   * This method needs the assembly object, so it needs to be called from an
   * object that has assembly access (components do not).
   *
   * @param assembly   The assembly object
   */
  void buildCoupledElemQpIndexMap(Assembly & assembly);

  /**
   * Returns true if the primary and secondary meshes are coincident
   */
  bool meshesAreCoincident() const { return _meshes_are_coincident; }

  /**
   * Returns true if the primary and secondary meshes are aligned
   */
  bool meshesAreAligned() const { return _meshes_are_aligned; }

  /**
   * Returns true if the element ID has a coupled element ID
   *
   * @param[in] elem_id Element ID for which to find the coupled element ID
   */
  bool hasCoupledElemID(const dof_id_type & elem_id) const;

  /**
   * Gets the coupled element ID for a given element ID
   *
   * @param[in] elem_id Element ID for which to find the coupled element ID
   */
  const dof_id_type & getCoupledElemID(const dof_id_type & elem_id) const;

  /**
   * Returns true if the node ID has a coupled node ID
   *
   * @param[in] node_id Node ID for which to find the coupled node ID
   */
  bool hasCoupledNodeID(const dof_id_type & node_id) const;

  /**
   * Gets the coupled node ID for a given node ID
   *
   * @param[in] node_id Node ID for which to find the coupled node ID
   */
  const dof_id_type & getCoupledNodeID(const dof_id_type & node_id) const;

  /**
   * Gets the quadrature point index on the coupled element corresponding to the
   * quadrature point index on the provided primary element
   *
   * Only local elements on the primary mesh may be queried.
   *
   * @param[in] elem_id   Element ID for which to find the coupled quadrature point
   * @param[in] qp        Quadrature point index on the given element
   */
  unsigned int getCoupledElemQpIndex(const dof_id_type & elem_id, const unsigned int & qp) const;

protected:
  /**
   * Builds the mapping using the extracted mesh information
   */
  void buildMapping();

  /**
   * Gets the local quadrature point map for the primary or secondary side
   *
   * @param[in] assembly   Assembly
   * @param[in] elem_ids   Vector of element IDs
   * @param[in] side_ids   Vector of side IDs (if any)
   */
  std::map<dof_id_type, std::vector<Point>>
  getLocalQuadraturePointMap(Assembly & assembly,
                             const std::vector<dof_id_type> & elem_ids,
                             const std::vector<unsigned short int> & side_ids) const;

  /// Map of element ID to coupled element ID
  std::map<dof_id_type, dof_id_type> _coupled_elem_ids;
  /// Map of node ID to coupled node ID
  std::map<dof_id_type, dof_id_type> _coupled_node_ids;
  /// Map of element ID to vector of coupled quadrature points
  std::map<dof_id_type, std::vector<unsigned int>> _coupled_elem_qp_indices;

  /// Flag that meshes are coincident
  bool _meshes_are_coincident;
  /// Flag that meshes are aligned
  bool _meshes_are_aligned;
};
