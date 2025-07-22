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

CSGXCylinder::CSGXCylinder(
    const std::string & name, const Real y0, const Real z0, const Real r, std::string boundary)
  : CSGSurface(name, MooseUtils::prettyCppType<CSGXCylinder>(), boundary), _y0(y0), _z0(z0), _r(r)
{
}

std::unordered_map<std::string, Real>
CSGXCylinder::getCoeffs() const
{
  std::unordered_map<std::string, Real> coeffs = {{"y0", _y0}, {"z0", _z0}, {"r", _r}};
  return coeffs;
}

CSGSurface::Halfspace
CSGXCylinder::getHalfspaceFromPoint(const Point & p) const
{
  // Compute distance from the cylinder center to determine if inside (< r^2)
  // or outside (> r^2) the cylinder
  const Real dist_sq = Utility::pow<2>((p(1) - _y0)) + Utility::pow<2>((p(2) - _z0));

  return (dist_sq > Utility::pow<2>(_r)) ? CSGSurface::Halfspace::POSITIVE
                                         : CSGSurface::Halfspace::NEGATIVE;
}
} // namespace CSG
