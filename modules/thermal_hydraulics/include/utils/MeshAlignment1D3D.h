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
 * Builds mapping between a 1D subdomain and a 3D boundary
 *
 * The "primary" mesh is the 1D subdomain, and the "secondary" mesh is the 3D boundary.
 */
class MeshAlignment1D3D : public MeshAlignmentOneToMany
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
};
