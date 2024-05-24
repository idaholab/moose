//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshAlignmentOneToMany.h"

class Assembly;

/**
 * Builds mapping between a 2D boundary and a 3D boundary
 *
 * The "primary" mesh is the 2D boundary, and the "secondary" mesh is the 3D boundary.
 */
class MeshAlignment2D3D : public MeshAlignmentOneToMany
{
public:
  /**
   * Constructor
   *
   * @param mesh[in] mesh   Mesh
   */
  MeshAlignment2D3D(const MooseMesh & mesh);

  /**
   * Extracts mesh information and builds the mapping
   *
   * @param secondary_boundary_info[in] List of tuples (elem_id, side_id) of the primary side
   * @param secondary_boundary_info[in] List of tuples (elem_id, side_id) of the secondary side
   * @param[in] axis_point  Any point on the axis of the 1D or 2D boundary
   * @param[in] axis_direction  Direction of the axis for the 1D or 2D boundary
   */
  void initialize(
      const std::vector<std::tuple<dof_id_type, unsigned short int>> & primary_boundary_info,
      const std::vector<std::tuple<dof_id_type, unsigned short int>> & secondary_boundary_info,
      const Point & axis_point,
      const RealVectorValue & axis_direction);

  /**
   * Builds the map used for getting the coupled quadrature point index
   *
   * This method is used for coupling a 2D boundary to a 3D boundary, where
   * the map will be used within a side user object for the 3D boundary.
   * This method needs the assembly object, so it needs to be called from an
   * object that has assembly access (components do not).
   *
   * @param assembly   The assembly object
   */
  void buildCoupledElemQpIndexMapSecondary(Assembly & assembly);

  /**
   * Gets the area for each quadrature point on a primary element
   *
   * @param[in] primary_elem_id  Primary element ID
   */
  const std::vector<Real> & getPrimaryArea(const dof_id_type primary_elem_id) const;

protected:
  /// Map of primary element ID to area for each quadrature point
  std::map<dof_id_type, std::vector<Real>> _primary_elem_id_to_area;
};
