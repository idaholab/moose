//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Positions.h"

/**
 * Creates positions (points) following an Cartesian grid
 */
class CartesianGridPositions : public Positions
{
public:
  static InputParameters validParams();

  CartesianGridPositions(const InputParameters & parameters);

  void initialize() override;

protected:
  /// Center of the lattice
  const Point _center;

  // Extent of the Cartesian lattice
  /// Extent of the lattice in the X direction
  const Real _dx;
  /// Extent of the lattice in the Y direction
  const Real _dy;
  /// Extent of the lattice in the Z direction
  const Real _dz;

  /// Number of points along the X direction
  const unsigned int _nx;
  /// Number of points along the Y direction
  const unsigned int _ny;
  /// Number of points along the Z direction
  const unsigned int _nz;

  /// 2D pattern of the pins to select (if specified)
  std::vector<std::vector<std::vector<unsigned int>>> _pattern;

  /// List of the pattern locations to include. Include all if empty
  std::set<unsigned int> _include_in_pattern;
};
