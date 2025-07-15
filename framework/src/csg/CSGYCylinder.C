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

CSGYCylinder::CSGYCylinder(const std::string & name,
                           const Real x0,
                           const Real z0,
                           const Real r,
                           CSGSurface::BoundaryType boundary)
  : CSGSurface(name, SurfaceType::YCYLINDER, boundary), _x0(x0), _z0(z0), _r(r)
{
}

std::map<std::string, Real>
CSGYCylinder::getCoeffs() const
{
  std::map<std::string, Real> coeffs = {{"x0", _x0}, {"z0", _z0}, {"r", _r}};
  return coeffs;
}

CSGSurface::Direction
CSGYCylinder::directionFromPoint(const Point & p) const
{
  // Compute distance from the cylinder center to determine if inside (< r^2)
  // or outside (> r^2) the cylinder
  const Real dist_sq = Utility::pow<2>((p(0) - _x0)) + Utility::pow<2>((p(2) - _z0));

  return (dist_sq > Utility::pow<2>(_r)) ? CSGSurface::Direction::POSITIVE
                                         : CSGSurface::Direction::NEGATIVE;
}
} // namespace CSG
