//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CylindricalGridDivision.h"
#include "MooseMesh.h"

#include "libmesh/elem.h"
#include <math.h>

registerMooseObject("MooseApp", CylindricalGridDivision);

InputParameters
CylindricalGridDivision::validParams()
{
  InputParameters params = MeshDivision::validParams();
  params.addClassDescription(
      "Divide the mesh along a cylindrical grid. The innermost numbering of divisions is the "
      "radial bins, then comes the azimuthal bins, then the axial bins");

  // Definition of the cylinder
  params.addRequiredParam<Point>("axis_direction", "Direction of the cylinder's axis");
  params.addRequiredParam<Point>(
      "azimuthal_start",
      "Direction of the 0-azimuthal-angle vector, normal to the cylinder's axis");
  params.addRequiredParam<Point>("center",
                                 "Point on the cylinder's axis, acting as the center of this local "
                                 "R-theta-Z coordinate based division");

  // Spatial bounds of the cylinder
  params.addRangeCheckedParam<Real>(
      "r_min", 0, "r_min>=0", "Minimum radial coordinate (for a hollow cylinder)");
  params.addRequiredRangeCheckedParam<Real>("r_max", "r_max>0", "Maximum radial coordinate");
  params.addRangeCheckedParam<Real>("cylinder_axial_min",
                                    std::numeric_limits<Real>::min(),
                                    "cylinder_axial_min>0",
                                    "Minimum axial coordinate");
  params.addRangeCheckedParam<Real>("cylinder_axial_max",
                                    std::numeric_limits<Real>::max(),
                                    "cylinder_axial_max>0",
                                    "Maximum axial coordinate");

  // Number of bins
  params.addRequiredRangeCheckedParam<unsigned int>(
      "n_radial", "n_radial>0", "Number of divisions in the cylinder radial direction");
  params.addRangeCheckedParam<unsigned int>(
      "n_azimuthal", 1, "n_azimuthal>0", "Number of divisions in the azimuthal direction");
  params.addRangeCheckedParam<unsigned int>(
      "n_axial", 1, "n_axial>0", "Number of divisions in the cylinder axial direction");

  params.addParam<bool>("assign_domain_outside_grid_to_border",
                        false,
                        "Whether to map the domain outside the grid back to the border of the grid "
                        "(radially or axially)");

  return params;
}

CylindricalGridDivision::CylindricalGridDivision(const InputParameters & parameters)
  : MeshDivision(parameters),
    _direction(getParam<Point>("axis_direction")),
    _center(getParam<Point>("center")),
    _azim_dir(getParam<Point>("azimuthal_start")),
    _min_r(getParam<Real>("r_min")),
    _max_r(getParam<Real>("r_max")),
    _min_z(getParam<Real>("cylinder_axial_min")),
    _max_z(getParam<Real>("cylinder_axial_max")),
    _n_radial(getParam<unsigned int>("n_radial")),
    _n_azim(getParam<unsigned int>("n_azimuthal")),
    _n_axial(getParam<unsigned int>("n_axial")),
    _outside_grid_counts_as_border(getParam<bool>("assign_domain_outside_grid_to_border"))
{
  CylindricalGridDivision::initialize();

  // Check axis
  if (!MooseUtils::absoluteFuzzyEqual(_direction.norm_sq(), 1))
    paramError("axis_direction", "Axis must have a norm of 1");
  if (!MooseUtils::absoluteFuzzyEqual(_azim_dir.norm_sq(), 1))
    paramError("azimuthal_start", "Azimuthal axis must have a norm of 1");

  // Check non-negative size
  if (_max_r < _min_r)
    paramError("r_min", "Maximum radius must be larger than minimum radius");
  if (_max_z < _min_z)
    paramError("cylinder_axial_min", "Maximum axial extent must be larger than minimum");

  // Check non-zero size if subdivided
  if (_n_radial > 1 && MooseUtils::absoluteFuzzyEqual(_min_r, _max_r))
    paramError("n_radial", "Zero-thickness cylinder cannot be subdivided radially");
  if (_n_axial > 1 && MooseUtils::absoluteFuzzyEqual(_min_z, _max_z))
    paramError("n_axial", "Zero-height cylinder cannot be subdivided axially");

  // Check non-infinite size if subdivided
  if ((_min_z == std::numeric_limits<Real>::min() || _max_z == std::numeric_limits<Real>::min()) &&
      _n_axial > 1)
    paramError("n_axial",
               "Infinite-height cylinder cannot be subdivided axially. Please specify both a "
               "cylinder axial minimum and maximum extent");
}

void
CylindricalGridDivision::initialize()
{
  setNumDivisions(_n_radial * _n_azim * _n_axial);
}

unsigned int
CylindricalGridDivision::divisionIndex(const Elem & elem) const
{
  return divisionIndex(elem.vertex_average());
}

unsigned int
CylindricalGridDivision::divisionIndex(const Point & pt) const
{
  // Compute coordinates of the point in the cylindrical coordinates
  Point pc;
  pc(2) = (pt - _center) * _direction;
  const Point in_plane = (pt - _center) - pc(2) * _direction;
  pc(0) = in_plane.norm();
  const Point pi2_dir = _azim_dir.cross(_direction);
  pc(1) = std::atan2(in_plane * pi2_dir, in_plane * _azim_dir) + libMesh::pi;

  if (!_outside_grid_counts_as_border)
  {
    if (MooseUtils::absoluteFuzzyLessThan(pc(0), _min_r) ||
        MooseUtils::absoluteFuzzyGreaterThan(pc(0), _max_r))
      return MooseMeshDivision::INVALID_DIVISION_INDEX;
    if (MooseUtils::absoluteFuzzyLessThan(pc(2), _min_z) ||
        MooseUtils::absoluteFuzzyGreaterThan(pc(2), _max_z))
      return MooseMeshDivision::INVALID_DIVISION_INDEX;
  }

  const auto not_found = MooseMeshDivision::INVALID_DIVISION_INDEX;
  auto ir = not_found, ia = not_found, iz = not_found;
  const Point widths(_max_r - _min_r, 2 * libMesh::pi, _max_z - _min_z);

  // Look inside the grid and on the left / back / bottom
  for (const auto jr : make_range(_n_radial + 1))
  {
    const auto border_r = _min_r + widths(0) * jr / _n_radial;
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
  for (const auto ja : make_range(_n_azim + 1))
  {
    const auto border_a = widths(1) * ja / _n_azim;
    if (ja > 0 && ja < _n_azim && MooseUtils::absoluteFuzzyEqual(border_a, pc(1)))
      mooseWarning("Querying the division index for a point of a boundary between two regions "
                   "azimuthally : " +
                   Moose::stringify(pt));
    if (border_a >= pc(1))
    {
      ia = ja > 0 ? ja - 1 : 0;
      break;
    }
  }
  for (const auto jz : make_range(_n_axial + 1))
  {
    const auto border_z = _min_z + widths(2) * jz / _n_axial;
    if (jz > 0 && jz < _n_axial && MooseUtils::absoluteFuzzyEqual(border_z, pc(2)))
      mooseWarning("Querying the division index for a point of a boundary between two axial "
                   "regions along the cylinder axis: " +
                   Moose::stringify(pt));
    if (border_z >= pc(2))
    {
      iz = jz > 0 ? jz - 1 : 0;
      break;
    }
  }

  // Look beyond the edges of the grid
  if (MooseUtils::absoluteFuzzyGreaterEqual(pc(0), _max_r))
    ir = _n_radial - 1;
  if (MooseUtils::absoluteFuzzyGreaterEqual(pc(2), _max_z))
    iz = _n_axial - 1;

  // Handle edge case on widths
  if (ir == not_found && MooseUtils::absoluteFuzzyEqual(widths(0), 0))
    ir = 0;
  if (iz == not_found && MooseUtils::absoluteFuzzyEqual(widths(2), 0))
    iz = 0;
  mooseAssert(ir != not_found, "We should have found a mesh division bin radially");
  mooseAssert(ia != not_found, "We should have found a mesh division bin azimuthally");
  mooseAssert(iz != not_found, "We should have found a mesh division bin axially");

  return ir + _n_radial * ia + iz * _n_radial * _n_azim;
}
