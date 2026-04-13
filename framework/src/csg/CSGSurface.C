//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGSurface.h"
#include "CSGEngUnit.h"
#include "CSGUtils.h"

namespace CSG
{

CSGSurface::CSGSurface(const std::string & name, const std::string & surf_type)
  : _name(name), _surface_type(surf_type)
{
  CSGUtils::checkValidCSGName(name);
}

CSGSurface::Halfspace
CSGSurface::getHalfspaceFromPoint(const Point & p) const
{
  // if transformations are present on the surface, apply them in reverse to the point and pass that
  // new point to the evaluation to ensure transformations are considered.

  // Create a local transformed copy - the original p in the calling scope is NOT modified
  const Point p_trans = getTransformations().size() ? applyReverseTransformsToPoint(p) : p;

  auto eval = evaluateSurfaceEquationAtPoint(p_trans);
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
  // If both objects are engineering units, delegate to CSGEngUnit::operator== to avoid
  // calling getCoeffs(), which is not supported on engineering unit types.
  if (const auto * this_eng = dynamic_cast<const CSGEngUnit *>(this))
  {
    const auto * other_eng = dynamic_cast<const CSGEngUnit *>(&other);
    if (other_eng)
      return *this_eng == *other_eng;
    return false; // an engineering unit cannot equal a plain surface
  }

  return (this->getName() == other.getName()) &&
         (this->getSurfaceType() == other.getSurfaceType()) &&
         (this->getCoeffs() == other.getCoeffs()) &&
         (this->getTransformations() == other.getTransformations());
}

bool
CSGSurface::operator!=(const CSGSurface & other) const
{
  return !(*this == other);
}

} // namespace CSG
