//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGSurfaceList.h"

namespace CSG
{

CSGSurfaceList::CSGSurfaceList() { _next_surface_id = 0; }

std::shared_ptr<CSGSurface>
CSGSurfaceList::addPlaneFromPoints(const std::string name,
                                   const Point p1,
                                   const Point p2,
                                   const Point p3)
{
  if (_surface_name_id_mapping.find(name) != _surface_name_id_mapping.end())
    mooseError("Surface with name " + name + " already exists in geoemetry");
  const auto surface_id = _next_surface_id++;
  _surfaces.insert(std::make_pair(surface_id, std::make_shared<CSGPlane>(name, p1, p2, p3)));
  _surface_name_id_mapping.insert({name, surface_id});
  return _surfaces[surface_id];
}
} // namespace CSG
