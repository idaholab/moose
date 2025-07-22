//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGZCylinder.h"

namespace CSG
{

CSGZCylinder::CSGZCylinder(
    const std::string & name, const Real x0, const Real y0, const Real r, std::string boundary)
  : CSGSurface(name, MooseUtils::prettyCppType<CSGZCylinder>(), boundary), _x0(x0), _y0(y0), _r(r)
{
}

std::unordered_map<std::string, Real>
CSGZCylinder::getCoeffs() const
{
  std::unordered_map<std::string, Real> coeffs = {{"x0", _x0}, {"y0", _y0}, {"r", _r}};
  return coeffs;
}

CSGSurface::Halfspace
CSGZCylinder::getHalfspaceFromPoint(const Point & p) const
{
  // Compute distance from the cylinder center to determine if inside (< r^2)
  // or outside (> r^2) the cylinder
  const Real dist_sq = Utility::pow<2>((p(0) - _x0)) + Utility::pow<2>((p(1) - _y0));

  return (dist_sq > Utility::pow<2>(_r)) ? CSGSurface::Halfspace::POSITIVE
                                         : CSGSurface::Halfspace::NEGATIVE;
}
} // namespace CSG
