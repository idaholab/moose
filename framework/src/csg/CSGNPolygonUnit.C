//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGNPolygonUnit.h"
#include "CSGPlane.h"

namespace CSG
{

CSGNPolygonUnit::CSGNPolygonUnit(const std::string & name, unsigned int n_sides, Real apothem)
  : CSGSurfaceEngUnit(name, MooseUtils::prettyCppType<CSGNPolygonUnit>()),
    _n_sides(n_sides),
    _apothem(apothem)
{
  if (_n_sides < 3)
    mooseError("N-sided polygon engineering unit must have 3 or more sides.");
  if (_apothem <= 0.0)
    mooseError("N-sided polygon engineering unit apothem must be positive.");
}

Real
CSGNPolygonUnit::evaluateSurfaceEquationAtPoint(const Point & p) const
{
  // Condition for interior (negative value) vs exterior (positive value) for the polygon is:
  // for point (x, y), if the following is true for all values of k, then the point is "interior".
  // If any one value is positive, then the point is outside the polygon. Therefore, we calculate
  // the maximum value. If that max value remains negative, then getHalfspaceFromPoint will
  // correctly determine the point to be interior.
  //
  //    x*cos(2*pi*k/N) + y*sin(2*pi*k/N) <= apothem, for all k = 0..N-1

  Real max_val = -1e200; // initialize to extremely large negative (to be updated)
  for (unsigned int k = 0; k < _n_sides - 1; ++k)
  {
    auto val =
        p(0) * std::cos(2.0 * M_PI * k / _n_sides) + p(0) * std::sin(2.0 * M_PI * k / _n_sides);
    if (val > max_val)
      max_val = val;
  }
  return max_val;
}

std::unordered_map<std::string, AttributeVariant>
CSGNPolygonUnit::getAttributes() const
{
  return {{"num_sides", static_cast<unsigned int>(_n_sides)}, {"apothem", _apothem}};
}

void
CSGNPolygonUnit::expandUnit(CSGBase & base)
{
  // Polygon orientation assumes an infinite prism oriented with the z-axis.
  // The right-most face is parallel to the y-axis and is centered at the origin.
  // Equation for the kth face, where the 0th face is the right-most and A is the apothem, follows
  // this equation:
  //    x*cos(2*pi*k/N) + y*sin(2*pi*k/N) + z*0 = A
  // Coefficients for the plane ax + by + cz = d:
  //    a = cos(2*pi*k/N)
  //    b = sin(2*pi*k/N)
  //    c = 0.0
  //    d = A (apothem)
  //
  // Surface naming scheme: [UnitName]_exp_[k]

  Real a, b; // to be calculated based on side
  Real c = 0.0;
  Real d = _apothem;

  // Initialize region to be added to:
  Point p(0, 0, 0); // origin used for determining half-space

  // base name for surfaces
  std::string base_name = getName() + "_exp_";

  for (unsigned int k = 0; k < _n_sides - 1; ++k)
  {
    auto sname = base_name + std::to_string(k);
    a = std::cos(2.0 * M_PI * k / _n_sides);
    b = std::sin(2.0 * M_PI * k / _n_sides);
    std::unique_ptr<CSG::CSGPlane> s_ptr = std::make_unique<CSG::CSGPlane>(sname, a, b, c, d);
    auto & surf = base.addSurface(std::move(s_ptr));

    // determine the halfspace that contains the origin for this surface
    auto hp_type = surf.getHalfspaceFromPoint(p);
    CSGRegion hp; // halfspace region for this surface only (to be intersected below)
    if (hp_type == CSGSurface::Halfspace::POSITIVE)
      hp = +surf;
    else
      hp = -surf;

    // start the region with first half-space, otherwise intersect with existing region
    if (_expanded_region.getRegionType() == CSGRegion::RegionType::EMPTY)
      _expanded_region = hp;
    else
      _expanded_region &= hp; // intersect with existing region
  }
}

} // namespace CSG
