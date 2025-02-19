//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGXCylinder.h"

namespace CSG
{

CSGXCylinder::CSGXCylinder(const std::string name, const Real y0, const Real z0, const Real r)
  : CSGSurface(name, SurfaceType::xcylinder), _y0(y0), _z0(z0), _r(r)
{
}

std::map<std::string, Real>
CSGXCylinder::getCoeffs()
{
  std::map<std::string, Real> coeffs = {{"y0", _y0}, {"z0", _z0}, {"r", _r}};
  return coeffs;
}

CSGSurface::Direction
CSGXCylinder::directionFromPoint(const Point p)
{
  // Compute distance from the cylinder center to determine if inside (< r^2)
  // or outside (> r^2) the cylinder
  const Real dist_sq = pow((p(1) - _y0), 2) + pow((p(2) - _z0), 2);

  return (dist_sq > pow(_r, 2)) ? CSGSurface::Direction::positive : CSGSurface::Direction::negative;
}
} // namespace CSG
