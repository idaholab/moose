//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGYCylinder.h"

namespace CSG
{

CSGYCylinder::CSGYCylinder(const std::string name, const Real x0, const Real z0, const Real r)
  : CSGSurface(name, SurfaceType::YCYLINDER), _x0(x0), _z0(z0), _r(r)
{
}

std::map<std::string, Real>
CSGYCylinder::getCoeffs()
{
  std::map<std::string, Real> coeffs = {{"x0", _x0}, {"z0", _z0}, {"r", _r}};
  return coeffs;
}

CSGSurface::Direction
CSGYCylinder::directionFromPoint(const Point p)
{
  // Compute distance from the cylinder center to determine if inside (< r^2)
  // or outside (> r^2) the cylinder
  const Real dist_sq = pow((p(0) - _x0), 2) + pow((p(2) - _z0), 2);

  return (dist_sq > pow(_r, 2)) ? CSGSurface::Direction::positive : CSGSurface::Direction::negative;
}
} // namespace CSG
