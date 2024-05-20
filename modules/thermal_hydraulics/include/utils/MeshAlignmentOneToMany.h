//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshAlignmentBase.h"

/**
 * Builds mapping between a 1D/2D boundary and a 3D boundary
 *
 * The "primary" mesh is the 1D/2D boundary, and the "secondary" mesh is the 3D boundary.
 */
class MeshAlignmentOneToMany : public MeshAlignmentBase
{
public:
  /**
   * Constructor
   *
   * @param mesh[in] mesh   Mesh
   */
  MeshAlignmentOneToMany(const MooseMesh & mesh);

  /**
   * Returns true if the given secondary element ID has a coupled primary element
   *
   * @param[in] secondary_elem_id   Secondary element ID for which to find the coupled primary
   * element ID
   */
  bool hasCoupledPrimaryElemID(const dof_id_type & secondary_elem_id) const;

  /**
   * Gets the coupled primary element ID for a given secondary element ID
   *
   * @param[in] secondary_elem_id   Secondary element ID for which to find the coupled primary
   * element ID
   */
  dof_id_type getCoupledPrimaryElemID(const dof_id_type & secondary_elem_id) const;

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
   * Gets the number of quadrature points for faces on the primary boundary
   */
  unsigned int getPrimaryNumberOfQuadraturePoints() const { return _n_qp_primary; }

  /**
   * Gets the number of quadrature points for faces on the secondary boundary
   */
  unsigned int getSecondaryNumberOfQuadraturePoints() const { return _n_qp_secondary; }

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

  /**
   * Gets the maximum number of secondary elements coupled to any primary element
   */
  unsigned int getMaxCouplingSize() const { return _max_coupling_size; }

protected:
  /**
   * Builds the mapping using the extracted mesh information
   */
  void buildMapping();

  /**
   * Checks the alignment and sets \c _mesh_alignment accordingly
   *
   * @param[in] axis_point  Any point on the axis of the 1D or 2D boundary
   * @param[in] axis_direction  Direction of the axis for the 1D or 2D boundary
   */
  void checkAlignment(const Point & axis_point, const RealVectorValue & axis_direction);

  /// Map of primary element ID to coupled secondary element IDs
  std::map<dof_id_type, std::vector<dof_id_type>> _primary_elem_id_to_secondary_elem_ids;
  /// Map of secondary element ID to coupled primary element ID
  std::map<dof_id_type, dof_id_type> _secondary_elem_id_to_primary_elem_id;
  /// Map of secondary element ID to vector of coupled quadrature points
  std::map<dof_id_type, std::vector<unsigned int>> _secondary_elem_id_to_qp_indices;

  /// Number of quadrature points for faces on the primary boundary
  unsigned int _n_qp_primary;
  /// Number of quadrature points for faces on the secondary boundary
  unsigned int _n_qp_secondary;

  /// The maximum number of secondary elements coupled to any primary element
  unsigned long _max_coupling_size;
};
