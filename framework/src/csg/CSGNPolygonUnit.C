//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGNPolygonUnit.h"
#include "CSGSphere.h"

namespace CSG
{

CSGNPolygonUnit::CSGNPolygonUnit(const std::string & name, unsigned int n_sides, Real apothem)
  : CSGSurfaceEngUnit(name, MooseUtils::prettyCppType<CSGNPolygonUnit>()),
    _n_sides(n_sides),
    _apothem(apothem)
{
  if (_n_sides < 3)
    mooseError("N-sided polygon engineering unit must have 3 or more sides.");
  if (_apothem <= 0.0)
    mooseError("N-sided polygon engineering unit apothem must be positive.");
}

Real
CSGNPolygonUnit::evaluateSurfaceEquationAtPoint(const Point & p) const
{
  // PLACEHOLDER
  return 1.0;
}

std::unordered_map<std::string, AttributeVariant>
CSGNPolygonUnit::getAttributes() const
{
  return {{"num_sides", static_cast<unsigned int>(_n_sides)}, {"apothem", _apothem}};
}

void
CSGNPolygonUnit::expandUnit(CSGBase & base)
{
  // PLACEHOLDER
  std::unique_ptr<CSGSurface> s1_ptr = std::make_unique<CSGSphere>("s1", 3.0);
  auto & _s1 = base.addSurface(std::move(s1_ptr));
  _expanded_region = -_s1;
}

} // namespace CSG
