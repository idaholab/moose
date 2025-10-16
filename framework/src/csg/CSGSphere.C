//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGSphere.h"

namespace CSG
{

CSGSphere::CSGSphere(const std::string & name, const Point & center, const Real r)
  : CSGSurface(name, MooseUtils::prettyCppType<CSGSphere>()),
    _x0(center(0)),
    _y0(center(1)),
    _z0(center(2)),
    _r(r)
{
  checkRadius();
}

CSGSphere::CSGSphere(const std::string & name, const Real r)
  : CSGSurface(name, MooseUtils::prettyCppType<CSGSphere>()), _x0(0.0), _y0(0.0), _z0(0.0), _r(r)
{
  checkRadius();
}

std::unordered_map<std::string, Real>
CSGSphere::getCoeffs() const
{
  std::unordered_map<std::string, Real> coeffs = {{"x0", _x0}, {"y0", _y0}, {"z0", _z0}, {"r", _r}};
  return coeffs;
}

Real
CSGSphere::evaluateSurfaceEquationAtPoint(const Point & p) const
{
  // Compute distance from the sphere center to determine if inside (< r^2)
  // or outside (> r^2) the sphere
  const Real dist_sq =
      Utility::pow<2>((p(0) - _x0)) + Utility::pow<2>((p(1) - _y0)) + Utility::pow<2>((p(2) - _z0));

  return dist_sq - Utility::pow<2>(_r);
}

void
CSGSphere::checkRadius() const
{
  if (_r <= 0.0)
    mooseError("Radius of sphere must be positive.");
}

} // namespace CSG
