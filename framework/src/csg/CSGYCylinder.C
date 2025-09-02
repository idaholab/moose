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

CSGYCylinder::CSGYCylinder(const std::string & name, const Real x0, const Real z0, const Real r)
  : CSGSurface(name, MooseUtils::prettyCppType<CSGYCylinder>()), _x0(x0), _z0(z0), _r(r)
{
  checkRadius();
}

std::unordered_map<std::string, Real>
CSGYCylinder::getCoeffs() const
{
  std::unordered_map<std::string, Real> coeffs = {{"x0", _x0}, {"z0", _z0}, {"r", _r}};
  return coeffs;
}

Real
CSGYCylinder::evaluateSurfaceEquationAtPoint(const Point & p) const
{
  // Compute distance from the cylinder center to determine if inside (< r^2)
  // or outside (> r^2) the cylinder
  const Real dist_sq = Utility::pow<2>((p(0) - _x0)) + Utility::pow<2>((p(2) - _z0));

  return dist_sq - Utility::pow<2>(_r);
}

void
CSGYCylinder::checkRadius() const
{
  if (_r <= 0.0)
    mooseError("Radius of y-cylinder must be positive.");
}

} // namespace CSG
