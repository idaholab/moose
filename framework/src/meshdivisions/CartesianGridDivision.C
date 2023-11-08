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

#include "libmesh/elem.h"

registerMooseObject("MooseApp", CartesianGridDivision);

InputParameters
CartesianGridDivision::validParams()
{
  InputParameters params = MeshDivision::validParams();
  params.addClassDescription("Divide the mesh along a Cartesian grid. Numbering increases from "
                             "bottom to top and from left to right and from back to front. The "
                             "inner ordering is X, then Y, then Z");
  params.addRequiredParam<Point>("bottom_left", "Bottom-back-left corner of the grid");
  params.addRequiredParam<Point>("top_right", "Top-front-right corner of the grid");
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
    _bottom_left(getParam<Point>("bottom_left")),
    _top_right(getParam<Point>("top_right")),
    _widths(_top_right - _bottom_left),
    _nx(getParam<unsigned int>("nx")),
    _ny(getParam<unsigned int>("ny")),
    _nz(getParam<unsigned int>("nz")),
    _outside_grid_counts_as_border(getParam<bool>("assign_domain_outside_grid_to_border"))
{
  CartesianGridDivision::initialize();
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
  setNumDivisions(_nx * _ny * _nz);
}

unsigned int
CartesianGridDivision::divisionIndex(const Elem & elem) const
{
  return divisionIndex(elem.vertex_average());
}

unsigned int
CartesianGridDivision::divisionIndex(const Point & pt) const
{
  if (!_outside_grid_counts_as_border)
  {
    if (MooseUtils::absoluteFuzzyLessThan(pt(0), _bottom_left(0)) ||
        MooseUtils::absoluteFuzzyGreaterThan(pt(0), _top_right(0)))
      return MooseMeshDivision::INVALID_DIVISION_INDEX;
    if (MooseUtils::absoluteFuzzyLessThan(pt(1), _bottom_left(1)) ||
        MooseUtils::absoluteFuzzyGreaterThan(pt(1), _top_right(1)))
      return MooseMeshDivision::INVALID_DIVISION_INDEX;
    if (MooseUtils::absoluteFuzzyLessThan(pt(2), _bottom_left(2)) ||
        MooseUtils::absoluteFuzzyGreaterThan(pt(2), _top_right(2)))
      return MooseMeshDivision::INVALID_DIVISION_INDEX;
  }

  const auto not_found = MooseMeshDivision::INVALID_DIVISION_INDEX;
  unsigned int ix = not_found, iy = not_found, iz = not_found;

  // Look inside the grid and on the left / back / bottom
  for (const auto jx : make_range(_nx + 1))
  {
    const auto border_x = _bottom_left(0) + _widths(0) * jx / _nx;
    if (jx > 0 && jx < _nx && MooseUtils::absoluteFuzzyEqual(border_x, pt(0)))
      mooseWarning(
          "Querying the division index for a point of a boundary between two regions in X: " +
          Moose::stringify(pt));
    if (border_x >= pt(0))
    {
      ix = (jx > 0) ? jx - 1 : 0;
      break;
    }
  }
  for (const auto jy : make_range(_ny + 1))
  {
    const auto border_y = _bottom_left(1) + _widths(1) * jy / _ny;
    if (jy > 0 && jy < _ny && MooseUtils::absoluteFuzzyEqual(border_y, pt(1)))
      mooseWarning(
          "Querying the division index for a point of a boundary between two regions in Y: " +
          Moose::stringify(pt));
    if (border_y >= pt(1))
    {
      iy = (jy > 0) ? jy - 1 : 0;
      break;
    }
  }
  for (const auto jz : make_range(_nz + 1))
  {
    const auto border_z = _bottom_left(2) + _widths(2) * jz / _nz;
    if (jz > 0 && jz < _nz && MooseUtils::absoluteFuzzyEqual(border_z, pt(2)))
      mooseWarning(
          "Querying the division index for a point of a boundary between two regions in Z: " +
          Moose::stringify(pt));
    if (border_z >= pt(2))
    {
      iz = (jz > 0) ? jz - 1 : 0;
      break;
    }
  }

  // Look on the right / front / top of the grid
  if (MooseUtils::absoluteFuzzyGreaterEqual(pt(0), _top_right(0)))
    ix = _nx - 1;
  if (MooseUtils::absoluteFuzzyGreaterEqual(pt(1), _top_right(1)))
    iy = _ny - 1;
  if (MooseUtils::absoluteFuzzyGreaterEqual(pt(2), _top_right(2)))
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

  return ix + _nx * iy + iz * _nx * _ny;
}
