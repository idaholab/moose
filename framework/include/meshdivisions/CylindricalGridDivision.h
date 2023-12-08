//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshDivision.h"

class Positions;

/**
 * Divides the mesh based on a cylindrical grid
 */
class CylindricalGridDivision : public MeshDivision
{
public:
  static InputParameters validParams();

  CylindricalGridDivision(const InputParameters & parameters);

  virtual void initialize() override;
  virtual unsigned int divisionIndex(const Point & pt) const override;
  virtual unsigned int divisionIndex(const Elem & elem) const override;

protected:
  /// Axis direction of the cylinder
  const Point _direction;
  /// Point at the center of the cylinder, serving as the coordinate frame center
  const Point * const _center;
  /// Positions giving all the centers of the cylinders, serving as the coordinate frame center
  const Positions * const _center_positions;
  /// Azimuthal axis direction (angle = 0)
  const Point _azim_dir;

  /// Minimal radial extent of the cylinder
  const Real _min_r;
  /// Maximal radial extent of the cylinder
  const Real _max_r;
  /// Minimal axial extent of the cylinder
  const Real _min_z;
  /// Maximal axial extent of the cylinder
  const Real _max_z;

  /// Number of divisions in the radial direction
  const unsigned int _n_radial;
  /// Number of divisions in the azimuthal direction
  const unsigned int _n_azim;
  /// Number of divisions in the cylinder axial direction
  const unsigned int _n_axial;

  /// Whether to map outside the grid onto the inner/outer crowns (radially) or top/bottom bins (axially)
  const bool _outside_grid_counts_as_border;
};
