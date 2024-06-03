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
 * Builds mapping between multiple 2D boundaries
 */
class MeshAlignment2D2D : public MeshAlignmentOneToMany
{
public:
  /**
   * Constructor
   *
   * @param mesh[in] mesh   Mesh
   */
  MeshAlignment2D2D(const MooseMesh & mesh);

  /**
   * Extracts mesh information and builds the mapping
   *
   * @param[in] boundary_infos  List of tuples (elem_id, side_id) for each side
   * @param[in] axis_point  Any point on the axis of the 1D or 2D boundary
   * @param[in] axis_direction  Direction of the axis for the 1D or 2D boundary
   */
  void initialize(
      const std::vector<std::vector<std::tuple<dof_id_type, unsigned short int>>> & boundary_infos,
      const Point & axis_point,
      const RealVectorValue & axis_direction);
};
