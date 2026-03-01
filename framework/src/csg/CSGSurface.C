//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGSurface.h"
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

void
CSGSurface::applyTransformation(TransformationType type,
                                const std::tuple<Real, Real, Real> & values)
{
  // Assert valid input as a safety measure
  // Main validation is done in CSGBase::applyTransformation
  mooseAssert(isValidTransformationValue(type, values),
              "Invalid transformation values for transformation type " +
                  getTransformationTypeString(type) + " on surface " + getName());
  _transformations.emplace_back(type, values);
}

} // namespace CSG
