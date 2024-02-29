//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HexagonalGridPositions.h"
#include "HexagonalLatticeUtils.h"

registerMooseObject("ReactorApp", HexagonalGridPositions);

InputParameters
HexagonalGridPositions::validParams()
{
  InputParameters params = Positions::validParams();
  params.addClassDescription(
      "Create positions along a hexagonal grid. Numbering of positions increases first "
      "counterclockwise, then expanding outwards from the inner ring, then axially. "
      "Inner-numbering is within a radial ring, outer-numbering is axial divisions");

  params.addParam<Point>("center", "Center of the hexagonal grid");
  params.addRequiredRangeCheckedParam<Real>(
      "lattice_flat_to_flat",
      "lattice_flat_to_flat>0",
      "Distance between two (inner) opposite sides of a lattice. Also known as bundle pitch or "
      "inner flat-to-flat distance");
  params.addRequiredRangeCheckedParam<Real>("pin_pitch", "pin_pitch>0", "Distance between pins");
  params.addRequiredRangeCheckedParam<unsigned int>("nr", "nr>0", "Number of hexagonal rings");

  // Use user-provided ordering
  params.set<bool>("auto_sort") = false;
  // All functors defined on all processes for now
  params.set<bool>("auto_broadcast") = false;

  return params;
}

HexagonalGridPositions::HexagonalGridPositions(const InputParameters & parameters)
  : Positions(parameters),
    _center(getParam<Point>("center")),
    _lattice_flat_to_flat(getParam<Real>("lattice_flat_to_flat")),
    _pin_pitch(getParam<Real>("pin_pitch")),
    _z_axis_index(MooseEnum("X Y Z", "Z")),
    _nr(getParam<unsigned int>("nr"))
{
  if (_pin_pitch > _lattice_flat_to_flat)
    mooseError("lattice_flat_to_flat", "Pin pitch should be smaller than bundle pitch");

  // Obtain the positions by evaluating the functors
  initialize();
  // Sort if needed (user-specified)
  finalize();
}

void
HexagonalGridPositions::initialize()
{
  clearPositions();

  // We make very large pins so they cover the entire position
  _hex_latt = std::make_unique<HexagonalLatticeUtils>(
      _lattice_flat_to_flat, _pin_pitch, _pin_pitch, 0., 1., _nr, _z_axis_index);

  const auto n_positions = _hex_latt->totalPins(_nr);
  _positions.resize(n_positions);

  for (const auto pos_i : make_range(n_positions))
    _positions[pos_i] = _hex_latt->pinCenters()[pos_i];

  _initialized = true;
}
