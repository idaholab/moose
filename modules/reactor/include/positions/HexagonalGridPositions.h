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

class HexagonalLatticeUtils;

/**
 * Creates positions (points) following an hexagonal grid
 */
class HexagonalGridPositions : public Positions
{
public:
  static InputParameters validParams();

  HexagonalGridPositions(const InputParameters & parameters);

  void initialize() override;

protected:
  /// Center of the lattice
  const Point _center;

  // Extent of the hexagonal lattice
  /// Distance from one side to the one facing it of the lattice
  const Real _lattice_flat_to_flat;
  /// Pitch between fuel pins
  const Real _pin_pitch;

  /// Axial component for the Z axis
  const MooseEnum _z_axis_index;

  /// Number of rings in the radial direction
  const unsigned int _nr;

  /// 2D pattern of the pins to select (if specified)
  std::vector<std::vector<unsigned int>> _pattern;

  /// List of the pattern locations to include. Include all if empty
  std::set<unsigned int> _include_in_pattern;

  /// Hexagonal lattice utility object
  std::unique_ptr<HexagonalLatticeUtils> _hex_latt;
};
