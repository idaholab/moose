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

CSGSurfaceList::CSGSurfaceList() {}

std::shared_ptr<CSGSurface>
CSGSurfaceList::addPlaneFromPoints(const std::string name,
                                   const Point p1,
                                   const Point p2,
                                   const Point p3)
{
  if (_surfaces.find(name) != _surfaces.end())
    mooseError("Surface with name " + name + " already exists in geoemetry");
  _surfaces.insert(std::make_pair(name, std::make_shared<CSGPlane>(name, p1, p2, p3)));
  return _surfaces[name];
}

std::shared_ptr<CSGSurface>
CSGSurfaceList::addPlaneFromCoefficients(const std::string name,
                                         const Real a,
                                         const Real b,
                                         const Real c,
                                         const Real d)
{
  if (_surfaces.find(name) != _surfaces.end())
    mooseError("Surface with name " + name + " already exists in geoemetry");
  _surfaces.insert(std::make_pair(name, std::make_shared<CSGPlane>(name, a, b, c, d)));
  return _surfaces[name];
}
} // namespace CSG
