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

CSGSurfaceList::CSGSurfaceList(const CSGSurfaceList & other_surface_list)
{
  for (const auto & [name, surf] : other_surface_list.getSurfaceListMap())
    _surfaces.emplace(name, surf->clone());
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
CSGSurfaceList::addSurface(std::unique_ptr<CSGSurface> surf)
{
  auto surf_name = surf->getName();
  auto [it, inserted] = _surfaces.emplace(surf_name, std::move(surf));
  if (!inserted)
    mooseError("Surface with name " + surf_name + " already exists in geometry.");
  return *it->second;
}

void
CSGSurfaceList::renameSurface(const CSGSurface & surface, const std::string & name)
{
  // check that this surface passed in is actually in the same surface that is in the surface list
  auto prev_name = surface.getName();
  auto it = _surfaces.find(prev_name);
  if (it == _surfaces.end() || it->second.get() != &surface)
    mooseError("Surface " + prev_name + " cannot be renamed to " + name +
               " as it does not exist in this CSGBase instance.");

  auto existing_surface = std::move(it->second);
  existing_surface->setName(name);
  _surfaces.erase(prev_name);
  addSurface(std::move(existing_surface));
}

bool
CSGSurfaceList::operator==(const CSGSurfaceList & other) const
{
  const auto all_surfs = this->getAllSurfaces();
  const auto other_surfs = other.getAllSurfaces();

  // Check that same number of surfaces are defined in both lists
  if (all_surfs.size() != other_surfs.size())
    return false;

  // Iterate through each CSGSurface in list and check equality of surface
  // with other list
  for (const auto & surf : all_surfs)
  {
    const auto & surf_name = surf.get().getName();
    if (!other.hasSurface(surf_name))
      return false;
    const auto & other_surf = other.getSurface(surf_name);
    if (surf.get() != other_surf)
      return false;
  }
  return true;
}

bool
CSGSurfaceList::operator!=(const CSGSurfaceList & other) const
{
  return !(*this == other);
}

} // namespace CSG
