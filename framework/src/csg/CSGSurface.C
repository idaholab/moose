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

CSGSurface::CSGSurface(const std::string & name,
                       const std::string surf_type,
                       CSGSurface::BoundaryType boundary)
  : _name(name),
    _surface_type("PLANE SPHERE XCYLINDER YCYLINDER ZCYLINDER", surf_type),
    _boundary_type(boundary)
{
}

const std::string
CSGSurface::getBoundaryTypeString() const
{
  switch (_boundary_type)
  {
    case BoundaryType::TRANSMISSION:
      return "transmission";
    case BoundaryType::VACUUM:
      return "vacuum";
    case BoundaryType::REFLECTIVE:
      return "reflective";
    default:
      mooseError("Detected invalid surface boundary type");
  }
}

bool
CSGSurface::operator==(const CSGSurface & other) const
{
  const auto name_eq = this->getName() == other.getName();
  const auto boundary_type_eq = this->getBoundaryType() == other.getBoundaryType();
  const auto surface_type_eq = this->getSurfaceType() == other.getSurfaceType();
  const auto coeffs_eq = this->getCoeffs() == other.getCoeffs();
  return (name_eq && boundary_type_eq && surface_type_eq && coeffs_eq);
}

bool
CSGSurface::operator!=(const CSGSurface & other) const
{
  return !(*this == other);
}

} // namespace CSG
