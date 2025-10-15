//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGSurface.h"

namespace CSG
{

CSGSurface::CSGSurface(const std::string & name, const std::string & surf_type)
  : _name(name), _surface_type(surf_type)
{
}

CSGSurface::Halfspace
CSGSurface::getHalfspaceFromPoint(const Point & p) const
{
  auto eval = evaluateSurfaceEquationAtPoint(p);
  if (MooseUtils::absoluteFuzzyGreaterThan(eval, 0))
    return Halfspace::POSITIVE;
  else if (MooseUtils::absoluteFuzzyLessThan(eval, 0))
    return Halfspace::NEGATIVE;
  else
    mooseError("Point ",
               p,
               " used to determine halfspace evaluation lies on the surface ",
               _name,
               ", leading to an ambiguously defined halfspace.");
}

bool
CSGSurface::operator==(const CSGSurface & other) const
{
  return (this->getName() == other.getName()) &&
         (this->getSurfaceType() == other.getSurfaceType()) &&
         (this->getCoeffs() == other.getCoeffs());
}

bool
CSGSurface::operator!=(const CSGSurface & other) const
{
  return !(*this == other);
}

} // namespace CSG
