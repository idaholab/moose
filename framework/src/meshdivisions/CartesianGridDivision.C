//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CartesianGridDivision.h"
#include "MooseMesh.h"
#include "Positions.h"
#include "FEProblemBase.h"

#include "libmesh/elem.h"

registerMooseObject("MooseApp", CartesianGridDivision);

InputParameters
CartesianGridDivision::validParams()
{
  InputParameters params = MeshDivision::validParams();
  params.addClassDescription("Divide the mesh along a Cartesian grid. Numbering increases from "
                             "bottom to top and from left to right and from back to front. The "
                             "inner ordering is X, then Y, then Z");
  params.addParam<Point>("bottom_left", "Bottom-back-left corner of the grid");
  params.addParam<Point>("top_right", "Top-front-right corner of the grid");
  params.addParam<Point>("center", "Center of the Cartesian grid");
  params.addParam<PositionsName>("center_positions",
                                 "Positions of the centers of divided Cartesian grids");
  params.addParam<Point>("widths", "Widths in the X, Y and Z directions");
  params.addRequiredRangeCheckedParam<unsigned int>("nx", "nx>0", "Number of divisions in X");
  params.addRequiredRangeCheckedParam<unsigned int>("ny", "ny>0", "Number of divisions in Y");
  params.addRequiredRangeCheckedParam<unsigned int>("nz", "nz>0", "Number of divisions in Z");
  params.addParam<bool>(
      "assign_domain_outside_grid_to_border",
      false,
      "Whether to map the domain outside the grid back to the border of the grid");

  return params;
}

CartesianGridDivision::CartesianGridDivision(const InputParameters & parameters)
  : MeshDivision(parameters),
    _bottom_left(isParamValid("bottom_left") ? getParam<Point>("bottom_left") : Point(0, 0, 0)),
    _top_right(isParamValid("top_right") ? getParam<Point>("top_right") : Point(0, 0, 0)),
    _center(isParamValid("center") ? &getParam<Point>("center") : nullptr),
    _center_positions(
        isParamValid("center_positions")
            ? &_fe_problem->getPositionsObject(getParam<PositionsName>("center_positions"))
            : nullptr),
    _widths(isParamValid("widths") ? getParam<Point>("widths") : Point(_top_right - _bottom_left)),
    _nx(getParam<unsigned int>("nx")),
    _ny(getParam<unsigned int>("ny")),
    _nz(getParam<unsigned int>("nz")),
    _outside_grid_counts_as_border(getParam<bool>("assign_domain_outside_grid_to_border"))
{
  CartesianGridDivision::initialize();

  // Check non-overlapping inputs for the dimensions of the grid
  if ((_center || _center_positions) && (isParamValid("bottom_left") || isParamValid("top_right")))
    mooseError("Either the center or the edges of the grids must be specified");
  if ((isParamValid("top_right") + isParamValid("bottom_left") == 1) && !isParamValid("widths"))
    paramError("bottom_left",
               "Both bottom_left and top_right must be passed to be able to determine the width");

  // Pre-determine the bounds if we can
  if (!_center_positions && _center)
  {
    _bottom_left = *_center - _widths / 2;
    _top_right = *_center + _widths / 2;
  }

  // Check widths
  if (_widths(0) < 0)
    paramError("top_right",
               "Top-front-right corner must be right (X axis) of bottom-left-back corner");
  if (_widths(1) < 0)
    paramError("top_right",
               "Top-front-right corner must be in front (Y axis) of bottom-left-back corner");
  if (_widths(2) < 0)
    paramError("top_right",
               "Top-front-right corner must be on top (Z axis) of bottom-left-back corner");
  if ((_nx > 1) && MooseUtils::absoluteFuzzyEqual(_widths(0), 0))
    paramError("nx", "Subdivision number must be 1 if width is 0 in X direction");
  if ((_ny > 1) && MooseUtils::absoluteFuzzyEqual(_widths(1), 0))
    paramError("ny", "Subdivision number must be 1 if width is 0 in Y direction");
  if ((_nz > 1) && MooseUtils::absoluteFuzzyEqual(_widths(2), 0))
    paramError("nz", "Subdivision number must be 1 if width is 0 in Z direction");
}

void
CartesianGridDivision::initialize()
{
  if (!_center_positions)
    setNumDivisions(_nx * _ny * _nz);
  else
    setNumDivisions(_center_positions->getNumPositions() * _nx * _ny * _nz);

  // Check that the grid is well-defined
  if (_center_positions)
  {
    Real min_dist = _widths.norm();
    Real min_center_dist = _center_positions->getMinDistanceBetweenPositions();
    // Note that if the positions are not co-planar, the distance reported would be bigger but there
    // could still be an overlap. Looking at min_center_dist is not enough
    if (MooseUtils::absoluteFuzzyGreaterThan(min_dist, min_center_dist))
      mooseWarning(
          "Cartesian grids centered on the positions are too close to each other (min distance: ",
          min_center_dist,
          "), closer than the extent of each grid. Mesh division is ill-defined");
  }

  // Examine mesh bounding box to see if the mesh may not be fully contained
  _mesh_fully_indexed = true;
  if (!_outside_grid_counts_as_border)
    for (auto i : make_range(LIBMESH_DIM))
    {
      if (_center_positions)
      {
        _mesh_fully_indexed = false;
        break;
      }
      else if (_bottom_left(i) > _mesh.getInflatedProcessorBoundingBox(0).first(i) ||
               _top_right(i) < _mesh.getInflatedProcessorBoundingBox(0).second(i))
      {
        _mesh_fully_indexed = false;
        break;
      }
    }
}

unsigned int
CartesianGridDivision::divisionIndex(const Elem & elem) const
{
  return divisionIndex(elem.vertex_average());
}

unsigned int
CartesianGridDivision::divisionIndex(const Point & pt) const
{
  unsigned int offset = 0;
  // Determine the local grid bounds
  Point bottom_left, top_right, p;
  if (_center_positions)
  {
    // If dividing using positions, find the closest position and
    // look at the relative position of the point compared to that position
    const bool initial = _fe_problem->getCurrentExecuteOnFlag() == EXEC_INITIAL;
    const auto nearest_grid_center_index = _center_positions->getNearestPositionIndex(pt, initial);
    offset = nearest_grid_center_index * _nx * _ny * _nz;
    const auto nearest_grid_center =
        _center_positions->getPosition(nearest_grid_center_index, initial);
    bottom_left = -_widths / 2;
    top_right = _widths / 2;
    p = pt - nearest_grid_center;
  }
  else
  {
    bottom_left = _bottom_left;
    top_right = _top_right;
    p = pt;
  }

  if (!_outside_grid_counts_as_border)
  {
    if (MooseUtils::absoluteFuzzyLessThan(p(0), bottom_left(0)) ||
        MooseUtils::absoluteFuzzyGreaterThan(p(0), top_right(0)))
      return MooseMeshDivision::INVALID_DIVISION_INDEX;
    if (MooseUtils::absoluteFuzzyLessThan(p(1), bottom_left(1)) ||
        MooseUtils::absoluteFuzzyGreaterThan(p(1), top_right(1)))
      return MooseMeshDivision::INVALID_DIVISION_INDEX;
    if (MooseUtils::absoluteFuzzyLessThan(p(2), bottom_left(2)) ||
        MooseUtils::absoluteFuzzyGreaterThan(p(2), top_right(2)))
      return MooseMeshDivision::INVALID_DIVISION_INDEX;
  }

  const auto not_found = MooseMeshDivision::INVALID_DIVISION_INDEX;
  auto ix = not_found, iy = not_found, iz = not_found;

  // Look inside the grid and on the left / back / bottom
  for (const auto jx : make_range(_nx + 1))
  {
    const auto border_x = bottom_left(0) + _widths(0) * jx / _nx;
    if (jx > 0 && jx < _nx && MooseUtils::absoluteFuzzyEqual(border_x, p(0)))
      mooseWarning(
          "Querying the division index for a point of a boundary between two regions in X: " +
          Moose::stringify(p));
    if (border_x >= p(0))
    {
      ix = (jx > 0) ? jx - 1 : 0;
      break;
    }
  }
  for (const auto jy : make_range(_ny + 1))
  {
    const auto border_y = bottom_left(1) + _widths(1) * jy / _ny;
    if (jy > 0 && jy < _ny && MooseUtils::absoluteFuzzyEqual(border_y, p(1)))
      mooseWarning(
          "Querying the division index for a point of a boundary between two regions in Y: " +
          Moose::stringify(p));
    if (border_y >= p(1))
    {
      iy = (jy > 0) ? jy - 1 : 0;
      break;
    }
  }
  for (const auto jz : make_range(_nz + 1))
  {
    const auto border_z = bottom_left(2) + _widths(2) * jz / _nz;
    if (jz > 0 && jz < _nz && MooseUtils::absoluteFuzzyEqual(border_z, p(2)))
      mooseWarning(
          "Querying the division index for a point of a boundary between two regions in Z: " +
          Moose::stringify(p));
    if (border_z >= p(2))
    {
      iz = (jz > 0) ? jz - 1 : 0;
      break;
    }
  }

  // Look on the right / front / top of the grid
  if (MooseUtils::absoluteFuzzyGreaterEqual(p(0), top_right(0)))
    ix = _nx - 1;
  if (MooseUtils::absoluteFuzzyGreaterEqual(p(1), top_right(1)))
    iy = _ny - 1;
  if (MooseUtils::absoluteFuzzyGreaterEqual(p(2), top_right(2)))
    iz = _nz - 1;

  // Handle edge case on widths
  if (ix == not_found && MooseUtils::absoluteFuzzyEqual(_widths(0), 0))
    ix = 0;
  if (iy == not_found && MooseUtils::absoluteFuzzyEqual(_widths(1), 0))
    iy = 0;
  if (iz == not_found && MooseUtils::absoluteFuzzyEqual(_widths(2), 0))
    iz = 0;
  mooseAssert(ix != not_found, "We should have found a mesh division bin in X");
  mooseAssert(iy != not_found, "We should have found a mesh division bin in Y");
  mooseAssert(iz != not_found, "We should have found a mesh division bin in Z");

  return offset + ix + _nx * iy + iz * _nx * _ny;
}
