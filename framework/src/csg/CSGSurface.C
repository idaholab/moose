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

CSGSurface::CSGSurface(const std::string name)
  : _name(name), _surface_type(SurfaceType::invalid), _boundary_type(BoundaryType::transmission)
{
}

CSGSurface::CSGSurface(const std::string name, SurfaceType surf_type)
  : _name(name), _surface_type(surf_type), _boundary_type(BoundaryType::transmission)
{
  if (_surface_type == SurfaceType::invalid)
    mooseError("Surface type of surface " + _name + " is being set to invalid");
}
} // namespace CSG
