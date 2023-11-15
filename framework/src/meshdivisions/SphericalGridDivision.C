//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SphericalGridDivision.h"
#include "MooseMesh.h"

#include "libmesh/elem.h"
#include <math.h>

registerMooseObject("MooseApp", SphericalGridDivision);

InputParameters
SphericalGridDivision::validParams()
{
  InputParameters params = MeshDivision::validParams();
  params.addClassDescription("Divide the mesh along a spherical grid.");

  // Definition of the sphere
  params.addRequiredParam<Point>("center", "Center of the sphere");

  // Spatial bounds of the sphere
  params.addRangeCheckedParam<Real>(
      "r_min", 0, "r_min>0", "Minimum radial coordinate (for a hollow sphere)");
  params.addRequiredRangeCheckedParam<Real>("r_max", "r_max>0", "Maximum radial coordinate");

  // Number of bins
  params.addRequiredRangeCheckedParam<unsigned int>(
      "n_radial", "n_radial>0", "Number of divisions in the sphere radial direction");

  params.addParam<bool>("assign_domain_outside_grid_to_border",
                        false,
                        "Whether to map the domain outside the grid back to the border of the grid "
                        "(radially or axially)");

  return params;
}

SphericalGridDivision::SphericalGridDivision(const InputParameters & parameters)
  : MeshDivision(parameters),
    _center(getParam<Point>("center")),
    _min_r(getParam<Real>("r_min")),
    _max_r(getParam<Real>("r_max")),
    _n_radial(getParam<unsigned int>("n_radial")),
    _outside_grid_counts_as_border(getParam<bool>("assign_domain_outside_grid_to_border"))
{
  SphericalGridDivision::initialize();

  // Check non-negative size
  if (_max_r < _min_r)
    paramError("r_min", "Maximum radius must be larger than minimum radius");

  // Check non-zero size if subdivided
  if (_n_radial > 1 && MooseUtils::absoluteFuzzyEqual(_min_r, _max_r))
    paramError("n_radial", "Zero-thickness sphere cannot be subdivided radially");
}

void
SphericalGridDivision::initialize()
{
  setNumDivisions(_n_radial);
}

unsigned int
SphericalGridDivision::divisionIndex(const Elem & elem) const
{
  return divisionIndex(elem.vertex_average());
}

unsigned int
SphericalGridDivision::divisionIndex(const Point & pt) const
{
  // Compute coordinates of the point in the spherical coordinates
  Point pc;
  pc(0) = (pt - _center).norm();

  if (!_outside_grid_counts_as_border)
  {
    if (MooseUtils::absoluteFuzzyLessThan(pc(0), _min_r) ||
        MooseUtils::absoluteFuzzyGreaterThan(pc(0), _max_r))
      return MooseMeshDivision::INVALID_DIVISION_INDEX;
  }

  const auto not_found = MooseMeshDivision::INVALID_DIVISION_INDEX;
  auto ir = not_found;
  const Real width = _max_r - _min_r;

  // Look inside the grid and on the left / back / bottom
  for (const auto jr : make_range(_n_radial + 1))
  {
    const auto border_r = _min_r + width * jr / _n_radial;
    if (jr > 0 && jr < _n_radial && MooseUtils::absoluteFuzzyEqual(border_r, pc(0)))
      mooseWarning(
          "Querying the division index for a point of a boundary between two regions radially: " +
          Moose::stringify(pt));
    if (border_r >= pc(0))
    {
      ir = jr > 0 ? jr - 1 : 0;
      break;
    }
  }

  // Look beyond the extent of the radial grid
  if (MooseUtils::absoluteFuzzyGreaterEqual(pc(0), _max_r))
    ir = _n_radial - 1;

  // Handle edge case on widths
  if (ir == not_found && MooseUtils::absoluteFuzzyEqual(width, 0))
    ir = 0;

  mooseAssert(ir != not_found, "We should have found a mesh division bin radially");
  return ir;
}
