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
 * Divides the mesh based on a spherical grid
 */
class SphericalGridDivision : public MeshDivision
{
public:
  static InputParameters validParams();

  SphericalGridDivision(const InputParameters & parameters);

  virtual void initialize() override;
  virtual unsigned int divisionIndex(const Point & pt) const override;
  virtual unsigned int divisionIndex(const Elem & elem) const override;

protected:
  /// Point at the center of the sphere, serving as the coordinate frame center
  const Point * const _center;
  /// Positions giving all the centers of the spheres, serving as the coordinate frame center
  const Positions * const _center_positions;

  /// Minimal radial extent of the sphere
  const Real _min_r;
  /// Maximal radial extent of the sphere
  const Real _max_r;

  /// Number of divisions in the radial direction
  const unsigned int _n_radial;

  /// Whether to map outside the grid onto the inner/outer shells (radially)
  const bool _outside_grid_counts_as_border;
};
