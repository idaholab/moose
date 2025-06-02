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

CSGSurface::CSGSurface(const std::string name, SurfaceType surf_type)
  : _name(name), _surface_type(surf_type), _boundary_type(BoundaryType::TRANSMISSION)
{
}

const std::string
CSGSurface::getSurfaceTypeString() const
{
  switch (_surface_type)
  {
    case SurfaceType::PLANE:
      return "plane";
    case SurfaceType::SPHERE:
      return "sphere";
    case SurfaceType::XCYLINDER:
      return "xcylinder";
    case SurfaceType::YCYLINDER:
      return "ycylinder";
    case SurfaceType::ZCYLINDER:
      return "zcylinder";
    default:
      mooseError("Detected invalid surface type");
  }
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
    default:
      mooseError("Detected invalid surface boundary type");
  }
}

} // namespace CSG
