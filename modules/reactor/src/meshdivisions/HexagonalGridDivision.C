//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HexagonalGridDivision.h"
#include "MooseMesh.h"
#include "HexagonalLatticeUtils.h"
#include "Positions.h"

#include "libmesh/elem.h"

registerMooseObject("ReactorApp", HexagonalGridDivision);

InputParameters
HexagonalGridDivision::validParams()
{
  InputParameters params = MeshDivision::validParams();
  params.addClassDescription(
      "Divide the mesh along a hexagonal grid. Numbering of pin divisions increases first "
      "counterclockwise, then expanding outwards from the inner ring, then axially. "
      "Inner-numbering is within a radial ring, outer-numbering is axial divisions");

  params.addParam<Point>("center", "Center of the hexagonal grid");
  params.addParam<PositionsName>("center_positions", "Centers of the hexagonal grids");

  params.addRequiredRangeCheckedParam<Real>(
      "lattice_flat_to_flat",
      "lattice_flat_to_flat>0",
      "Distance between two (inner) opposite sides of a lattice. Also known as bundle pitch or "
      "inner flat-to-flat distance");
  params.addRequiredRangeCheckedParam<Real>("pin_pitch", "pin_pitch>0", "Distance between pins");

  params.addRequiredParam<Real>("z_min", "Minimal axial extent of the lattice");
  params.addRequiredParam<Real>("z_max", "Maximum axial extent of the lattice");
  params.addRequiredRangeCheckedParam<unsigned int>("nr", "nr>0", "Number of hexagonal rings");
  params.addRequiredRangeCheckedParam<unsigned int>("nz", "nz>0", "Number of divisions in Z");
  params.addParam<bool>(
      "assign_domain_outside_grid_to_border",
      false,
      "Whether to map the domain outside the grid back to the border of the grid");

  return params;
}

HexagonalGridDivision::HexagonalGridDivision(const InputParameters & parameters)
  : MeshDivision(parameters),
    _center(isParamValid("center") ? getParam<Point>("center") : Point(0, 0, 0)),
    _center_positions(
        isParamValid("center_positions")
            ? &_fe_problem->getPositionsObject(getParam<PositionsName>("center_positions"))
            : nullptr),
    _lattice_flat_to_flat(getParam<Real>("lattice_flat_to_flat")),
    _pin_pitch(getParam<Real>("pin_pitch")),
    _z_axis_index(MooseEnum("X Y Z", "Z")),
    _min_z(getParam<Real>("z_min")),
    _max_z(getParam<Real>("z_max")),
    _nr(getParam<unsigned int>("nr")),
    _nz(getParam<unsigned int>("nz")),
    _outside_grid_counts_as_border(getParam<bool>("assign_domain_outside_grid_to_border"))
{
  HexagonalGridDivision::initialize();

  if (!isParamValid("center") && !_center_positions)
    paramError("center", "A center must be provided, or a Positions object for the centers");
  if (_pin_pitch > _lattice_flat_to_flat)
    mooseError("lattice_flat_to_flat", "Pin pitch should be smaller than bundle pitch");
  if ((_nz > 1) && MooseUtils::absoluteFuzzyEqual(_max_z, _min_z))
    paramError("nz", "Subdivision number must be 1 if width is 0 in Z direction");
}

void
HexagonalGridDivision::initialize()
{
  // We make very large pins so they cover the entire position
  _hex_latt = std::make_unique<HexagonalLatticeUtils>(
      _lattice_flat_to_flat, _pin_pitch, _pin_pitch, 0., 1., _nr, _z_axis_index);

  if (!_center_positions)
    setNumDivisions(_hex_latt->totalPins(_nr) * _nz);
  else
    setNumDivisions(_center_positions->getNumPositions() * _hex_latt->totalPins(_nr) * _nz);

  // Check that the grid is well-defined
  if (_center_positions)
  {
    const Real min_center_dist = _center_positions->getMinDistanceBetweenPositions();
    // Note that if the positions are not aligned on a hexagonal lattice themselves,
    // this bound is not sufficiently strict. The simplest example would be non-coplanar
    // points, which can be a great distance away axially but be on the same axis
    if (MooseUtils::absoluteFuzzyGreaterThan(_lattice_flat_to_flat, min_center_dist))
      mooseWarning(
          "Hexagonal grids centered on the positions are too close to each other (min distance: ",
          min_center_dist,
          "), closer than the extent of each grid (",
          _lattice_flat_to_flat,
          "). Mesh division is ill-defined ");
  }
}

unsigned int
HexagonalGridDivision::divisionIndex(const Elem & elem) const
{
  return divisionIndex(elem.vertex_average());
}

unsigned int
HexagonalGridDivision::divisionIndex(const Point & pt) const
{
  unsigned int offset = 0;

  // Get point in the coordinates of the lattice. This can involve a projection due to
  // the axis of the lattice, or simply a translation if there are lattices distributed
  // using positions
  Point pc;
  if (_center_positions)
  {
    // If dividing using positions, find the closest position and
    // look at the relative position of the point compared to that position
    const bool initial = _fe_problem->getCurrentExecuteOnFlag() == EXEC_INITIAL;
    const auto nearest_grid_center_index = _center_positions->getNearestPositionIndex(pt, initial);
    offset = nearest_grid_center_index * _hex_latt->totalPins(_nr) * _nz;
    const auto nearest_grid_center =
        _center_positions->getPosition(nearest_grid_center_index, initial);

    // Project in local hexagonal grid
    pc = pt - nearest_grid_center;
  }
  else
    pc = pt - _center;

  // Get radial division index, using the channel as the pins are 0-radius
  // The logic in get pin index requires getting the point in the plane of the pin centers
  auto ir = _hex_latt->pinIndex(pc);
  const auto n_pins = _hex_latt->nPins();

  if (!_outside_grid_counts_as_border)
  {
    if (ir == n_pins)
      return MooseMeshDivision::INVALID_DIVISION_INDEX;
    if (MooseUtils::absoluteFuzzyLessThan(pc(_z_axis_index), _min_z) ||
        MooseUtils::absoluteFuzzyGreaterThan(pc(_z_axis_index), _max_z))
      return MooseMeshDivision::INVALID_DIVISION_INDEX;
  }

  // If too far from the grid to have a valid radial index, use the closest pin
  if (ir == n_pins)
    ir = _hex_latt->closestPinIndex(pc);

  // Find axial index
  const auto not_found = MooseMeshDivision::INVALID_DIVISION_INDEX;
  auto iz = not_found;
  for (const auto jz : make_range(_nz + 1))
  {
    const auto border_z = _min_z + (_max_z - _min_z) * jz / _nz;
    if (jz > 0 && jz < _nz && MooseUtils::absoluteFuzzyEqual(border_z, pc(_z_axis_index)))
      mooseWarning(
          "Querying the division index for a point of a boundary between two regions in Z: " +
              Moose::stringify(pt),
          ", in local hex grid frame: ",
          Moose::stringify(pc));
    if (border_z >= pc(_z_axis_index))
    {
      iz = (jz > 0) ? jz - 1 : 0;
      break;
    }
  }

  // Look on the top of the grid
  if (MooseUtils::absoluteFuzzyGreaterEqual(pc(_z_axis_index), _max_z))
    iz = _nz - 1;

  // Handle edge case on widths
  if (iz == not_found && MooseUtils::absoluteFuzzyEqual(_max_z - _min_z, 0))
    iz = 0;
  mooseAssert(ir != not_found, "We should have found a mesh division bin radially");
  mooseAssert(iz != not_found, "We should have found a mesh division bin in Z");

  const auto n_radial = _hex_latt->totalPins(_nr);
  return offset + ir + iz * n_radial;
}
