//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesApp.h"
#include "NS.h"
#include "libmesh/elem.h"

Real
computeRadialCoordinate(const Point & p, const Point & center, const int & axis)
{
  Real dx = p(0) - center(0);
  Real dy = p(1) - center(1);
  Real dz = p(2) - center(2);
  Real double_count = p(axis) - center(axis);
  return std::sqrt(dx * dx + dy * dy + dz * dz - double_count * double_count);
}

Real
getCoordinate(const Point & p, const int & axis)
{
  return p(axis);
}

Real
computeAxialDistanceAbove(const Point & p, int axis, Real bed_bottom)
{
  return getCoordinate(p, axis) - bed_bottom;
}

Real
computeAxialDistanceBelow(const Point & p, int axis, Real bed_top)
{
  return bed_top - getCoordinate(p, axis);
}

Real
computeMinRadialDistance(const Point & point,
                         const Point & center,
                         const int & axis,
                         const Real & inner_radius,
                         const Real & outer_radius)
{
  Real r = computeRadialCoordinate(point, center, axis);

  // coordinate should be in-between inner_distance and outer_distance
  Real tol = NS_DEFAULT_VALUES::epsilon;
  if (r < inner_radius - tol || r > outer_radius + tol)
    mooseError("Radial coordinate " + std::to_string(r) + " does not lie between inner radius: " +
               std::to_string(inner_radius) + " and outer radius: " + std::to_string(outer_radius) +
               " in 'computeMinRadialDistance' function!");

  // sometimes, the distance is still computed as a very tiny 1e-16 number despite the
  // above check, so to be safe we take the absolute value
  Real inner_distance = std::abs(r - inner_radius);
  Real outer_distance = std::abs(outer_radius - r);
  Real distance = outer_distance;

  // if the bed is annular, we might be closer to the inner than outer radius
  if (inner_radius > 0.0)
    distance = std::min(inner_distance, outer_distance);

  return distance;
}

Real
computeMinAxialDistance(const Point & p, int axis, const Real & bed_bottom, const Real & bed_top)
{
  Real z = getCoordinate(p, axis);

  Real tol = NS_DEFAULT_VALUES::epsilon;
  if ((z < bed_bottom - tol) || (z > bed_top + tol))
    mooseError("Axial coordinate " + std::to_string(z) + " does not lie between bed bottom: " +
               std::to_string(bed_bottom) + " and bed top: " + std::to_string(bed_top) +
               " in 'computeMinAxialDistance' function!");

  return std::min(z - bed_bottom, bed_top - z);
}

Real
elementSize(const Elem * e)
{
  return e->hmin();
}
