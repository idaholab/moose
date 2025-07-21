//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGSurfaceList.h"
#include "CSGPlane.h"
#include "CSGSphere.h"
#include "CSGXCylinder.h"
#include "CSGYCylinder.h"
#include "CSGZCylinder.h"

namespace CSG
{

CSGSurfaceList::CSGSurfaceList() {}

void
CSGSurfaceList::checkSurfaceName(const std::string & name) const
{
  if (_surfaces.find(name) != _surfaces.end())
    mooseError("Surface with name " + name + " already exists in geoemetry.");
}

CSGSurface &
CSGSurfaceList::getSurface(const std::string & name) const
{
  auto surf = _surfaces.find(name);
  if (surf == _surfaces.end())
    mooseError("No surface by name " + name + " exists in the geometry.");
  else
    return *(surf->second);
}

std::vector<std::reference_wrapper<const CSGSurface>>
CSGSurfaceList::getAllSurfaces() const
{
  std::vector<std::reference_wrapper<const CSGSurface>> surfaces;
  for (auto it = _surfaces.begin(); it != _surfaces.end(); ++it)
    surfaces.push_back(*(it->second));
  return surfaces;
}

CSGSurface &
CSGSurfaceList::addSurface(std::unique_ptr<CSGSurface> & surf)
{
  auto name = surf->getName();
  checkSurfaceName(name);
  _surfaces.insert(std::make_pair(name, std::move(surf)));
  return *_surfaces[name];
}

void
CSGSurfaceList::renameSurface(const CSGSurface & surface, const std::string & name)
{
  // check that this surface passed in is actually in the same surface that is in the surface
  // list
  auto prev_name = surface.getName();
  auto existing_surface = std::move(_surfaces.find(prev_name)->second);
  if ((*existing_surface) != surface)
    mooseError("Surface " + prev_name + " cannot be renamed to " + name +
               " as it does not exist in this CSGBase instance.");

  checkSurfaceName(name);
  existing_surface->setName(name);
  _surfaces.erase(prev_name);
  _surfaces.insert(std::make_pair(name, std::move(existing_surface)));
}

} // namespace CSG
