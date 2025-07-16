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

CSGSurface &
CSGSurfaceList::addPlaneFromPoints(const std::string & name,
                                   const Point & p1,
                                   const Point & p2,
                                   const Point & p3,
                                   std::string boundary)
{
  checkSurfaceName(name);
  _surfaces.insert(std::make_pair(name, std::make_unique<CSGPlane>(name, p1, p2, p3, boundary)));
  return *_surfaces[name];
}

CSGSurface &
CSGSurfaceList::addPlaneFromCoefficients(const std::string & name,
                                         const Real a,
                                         const Real b,
                                         const Real c,
                                         const Real d,
                                         std::string boundary)
{
  checkSurfaceName(name);
  _surfaces.insert(std::make_pair(name, std::make_unique<CSGPlane>(name, a, b, c, d, boundary)));
  return *_surfaces[name];
}

CSGSurface &
CSGSurfaceList::addSphere(const std::string & name,
                          const Point & center,
                          const Real r,
                          std::string boundary)
{
  checkSurfaceName(name);
  _surfaces.insert(std::make_pair(name, std::make_unique<CSGSphere>(name, center, r, boundary)));
  return *_surfaces[name];
}

CSGSurface &
CSGSurfaceList::addCylinder(const std::string & name,
                            const Real x0,
                            const Real x1,
                            const Real r,
                            const std::string & axis,
                            std::string boundary)
{
  checkSurfaceName(name);
  std::unique_ptr<CSGSurface> surf;

  if (axis == "x" || axis == "X")
    surf = std::make_unique<CSGXCylinder>(name, x0, x1, r, boundary);
  else if (axis == "y" || axis == "Y")
    surf = std::make_unique<CSGYCylinder>(name, x0, x1, r, boundary);
  else if (axis == "z" || axis == "Z")
    surf = std::make_unique<CSGZCylinder>(name, x0, x1, r, boundary);
  else
    mooseError("Axis " + axis + " not recognized for CSG cylinder. Options are x, y, or z.");

  _surfaces.insert(std::make_pair(name, std::move(surf)));
  return *_surfaces[name];
}

std::vector<std::reference_wrapper<const CSGSurface>>
CSGSurfaceList::getAllSurfaces() const
{
  std::vector<std::reference_wrapper<const CSGSurface>> surfaces;
  for (auto it = _surfaces.begin(); it != _surfaces.end(); ++it)
    surfaces.push_back(*(it->second));
  return surfaces;
}

void
CSGSurfaceList::addSurface(std::unique_ptr<CSGSurface> & surf)
{
  auto name = surf->getName();
  checkSurfaceName(name);
  _surfaces.insert(std::make_pair(name, std::move(surf)));
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
