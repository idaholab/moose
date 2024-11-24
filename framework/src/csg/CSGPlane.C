//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGPlane.h"

namespace CSG
{

CSGPlane::CSGPlane(const std::string name, const Point p1, const Point p2, const Point p3)
  : CSGSurface(name, SurfaceType::plane)
{
  coeffsFromPoints(p1, p2, p3);
}

CSGPlane::CSGPlane(const std::string name, const Real a, const Real b, const Real c, const Real d)
  : CSGSurface(name, SurfaceType::plane), _a(a), _b(b), _c(c), _d(d)
{
}

std::map<std::string, Real>
CSGPlane::getCoeffs()
{
  std::map<std::string, Real> coeffs = {{"a", _a}, {"b", _b}, {"c", _c}, {"d", _d}};
  return coeffs;
}

void
CSGPlane::coeffsFromPoints(const Point p1, const Point p2, const Point p3)
{
  // Use three points on plane to solve for the plane equation in form aX + bY +cZ = d,
  // where we are solving for a, b, c, and d.
  RealVectorValue v1 = p2 - p1;
  RealVectorValue v2 = p3 - p1;
  RealVectorValue cross = v2.cross(v1);

  // TODO add MooseAssert that points aren't colinear
  _a = cross(0);
  _b = cross(1);
  _c = cross(2);
  _d = cross * (RealVectorValue)p1;
}

CSGSurface::Direction
CSGPlane::directionFromPoint(const Point p)
{
  // Compute dot product of <a, b, c> and p to determine if p lies
  // in the positive or negative halfspace of the plane
  const Real dot_prod = _a * p(0) + _b * p(1) + _c * p(2);

  // TODO add MooseAssert that point doesn't lie on plane
  return (dot_prod > _d) ? CSGSurface::Direction::positive : CSGSurface::Direction::negative;
}
} // namespace CSG
