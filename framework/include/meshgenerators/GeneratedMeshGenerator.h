//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"
#include "MooseEnum.h"

/**
 * Generates a line, square, or cube mesh with uniformly spaced or biased elements.
 */
class GeneratedMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  GeneratedMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// The dimension of the mesh
  MooseEnum _dim;

  /// Number of elements in x, y, z direction
  unsigned int &_nx, &_ny, &_nz;

  /// The min/max values for x,y,z component
  Real &_xmin, &_xmax, &_ymin, &_ymax, &_zmin, &_zmax;

  /// Whether or not subdomain_ids parameter is set
  bool _has_subdomain_ids;

  /**
   * All of the libmesh build_line/square/cube routines support an
   * option to grade the mesh into the boundaries according to the
   * spacing of the Gauss-Lobatto quadrature points.  Defaults to
   * false, and cannot be used in conjunction with x, y, and z
   * biasing.
   */
  const bool _gauss_lobatto_grid;

  /**
   * The amount by which to bias the cells in the x,y,z directions.
   * Must be in the range 0.5 <= _bias_x <= 2.0.
   * _bias_x < 1 implies cells are shrinking in the x-direction.
   * _bias_x==1 implies no bias (original mesh unchanged).
   * _bias_x > 1 implies cells are growing in the x-direction.
   */
  const Real _bias_x, _bias_y, _bias_z;

  /// prefix string for the boundary names
  const std::string _boundary_name_prefix;

  /// offset that is added to the boundary IDs
  const boundary_id_type _boundary_id_offset;
};
