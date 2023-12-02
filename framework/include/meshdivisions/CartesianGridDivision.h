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
 * Divides the mesh based on a Cartesian grid
 */
class CartesianGridDivision : public MeshDivision
{
public:
  static InputParameters validParams();

  CartesianGridDivision(const InputParameters & parameters);

  virtual void initialize() override;
  virtual unsigned int divisionIndex(const Point & pt) const override;
  virtual unsigned int divisionIndex(const Elem & elem) const override;

protected:
  /// Bottom left point of the grid
  Point _bottom_left;
  /// Top right point of the grid
  Point _top_right;
  /// Center of the grid, if user-specified
  const Point * const _center;
  /// Positions object holding the centers of the grids, if user-specified
  const Positions * const _center_positions;
  /// Width of the grid in all 3 axes
  const Point _widths;
  /// Number of divisions in the X direction
  const unsigned int _nx;
  /// Number of divisions in the Y direction
  const unsigned int _ny;
  /// Number of divisions in the Z direction
  const unsigned int _nz;
  /// Whether to map outside the grid onto the corner
  const bool _outside_grid_counts_as_border;
};
