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

class HexagonalLatticeUtils;
class Positions;

/**
 * Divides the mesh based on a hexagonal grid
 */
class HexagonalGridDivision : public MeshDivision
{
public:
  static InputParameters validParams();

  HexagonalGridDivision(const InputParameters & parameters);

  virtual void initialize() override;
  virtual unsigned int divisionIndex(const Point & pt) const override;
  virtual unsigned int divisionIndex(const Elem & elem) const override;

protected:
  /// Center of the lattice (single lattice)
  const Point _center;
  /// Centers of the lattices (lattices centered around positions)
  const Positions * _center_positions;

  // Extent of the hexagonal lattice
  /// Distance from one side to the one facing it of the lattice
  const Real _lattice_flat_to_flat;
  /// Pitch between fuel pins
  const Real _pin_pitch;

  /// Axial component for the Z axis
  const MooseEnum _z_axis_index;

  /// Minimal axial coordinate
  const Real _min_z;
  /// Maximal axial coordinate
  const Real _max_z;

  /// Number of rings in the radial direction
  const unsigned int _nr;
  /// Number of divisions in the Z direction
  const unsigned int _nz;
  /// Whether to map outside the grid onto the corner
  const bool _outside_grid_counts_as_border;

  /// Hexagonal lattice utility object
  std::unique_ptr<HexagonalLatticeUtils> _hex_latt;
};
