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
 * Builds mapping between a 1D subdomain and a 3D boundary
 *
 * The "primary" mesh is the 1D subdomain, and the "secondary" mesh is the 3D boundary.
 */
class MeshAlignment1D3D : public MeshAlignmentBase
{
public:
  /**
   * Constructor
   *
   * @param mesh[in] mesh   Mesh
   */
  MeshAlignment1D3D(const MooseMesh & mesh);

  /**
   * Extracts mesh information and builds the mapping
   *
   * @param primary_elem_ids[in] List of primary element IDs
   * @param secondary_boundary_info[in] List of tuples (elem_id, side_id) of the secondary side
   */
  void initialize(
      const std::vector<dof_id_type> & primary_elem_ids,
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
   * Returns true if the given primary element ID has coupled secondary elements
   *
   * @param[in] primary_elem_id   Primary element ID for which to find the coupled secondary element
   * IDs
   */
  bool hasCoupledSecondaryElemIDs(const dof_id_type & primary_elem_id) const;

  /**
   * Gets the coupled secondary element IDs for a given primary element ID
   *
   * @param[in] primary_elem_id   Primary element ID for which to find the coupled secondary element
   * IDs
   */
  const std::vector<dof_id_type> &
  getCoupledSecondaryElemIDs(const dof_id_type & primary_elem_id) const;

  /**
   * Gets the number of quadrature points for the given secondary element
   *
   * @param[in] secondary_elem_id   Secondary element ID
   */
  unsigned int getSecondaryNumberOfQuadraturePoints(const dof_id_type & secondary_elem_id) const;

  /**
   * Gets the quadrature point index on the primary element corresponding to the
   * quadrature point index on the provided secondary element
   *
   * Only secondary elements corresponding to local primary elements may be queried.
   *
   * @param[in] secondary_elem_id   Secondary element ID for which to find the coupled quadrature
   * point
   * @param[in] secondary_qp        Quadrature point index on the given secondary element
   */
  unsigned int getCoupledPrimaryElemQpIndex(const dof_id_type & secondary_elem_id,
                                            const unsigned int & secondary_qp) const;

protected:
  /**
   * Builds the mapping using the extracted mesh information
   */
  void buildMapping();

  /// Map of primary element ID to coupled secondary element IDs
  std::map<dof_id_type, std::vector<dof_id_type>> _primary_elem_id_to_secondary_elem_ids;
  /// Map of secondary element ID to vector of coupled quadrature points
  std::map<dof_id_type, std::vector<unsigned int>> _secondary_elem_id_to_qp_indices;
};
