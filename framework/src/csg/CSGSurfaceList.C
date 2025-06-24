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

void
CSGSurfaceList::checkSurfaceName(const std::string name) const
{
  if (_surfaces.find(name) != _surfaces.end())
    mooseError("Surface with name " + name + " already exists in geoemetry.");
}

const std::shared_ptr<CSGSurface> &
CSGSurfaceList::getSurface(const std::string name) const
{
  auto surf = _surfaces.find(name);
  if (surf == _surfaces.end())
    mooseError("No surface by name " + name + " exists in the geometry.");
  else
    return surf->second;
}

std::shared_ptr<CSGSurface>
CSGSurfaceList::addPlaneFromPoints(const std::string name,
                                   const Point p1,
                                   const Point p2,
                                   const Point p3,
                                   CSGSurface::BoundaryType boundary)
{
  checkSurfaceName(name);
  _surfaces.insert(std::make_pair(name, std::make_shared<CSGPlane>(name, p1, p2, p3, boundary)));
  return _surfaces[name];
}

std::shared_ptr<CSGSurface>
CSGSurfaceList::addPlaneFromCoefficients(const std::string name,
                                         const Real a,
                                         const Real b,
                                         const Real c,
                                         const Real d,
                                         CSGSurface::BoundaryType boundary)
{
  checkSurfaceName(name);
  _surfaces.insert(std::make_pair(name, std::make_shared<CSGPlane>(name, a, b, c, d, boundary)));
  return _surfaces[name];
}

std::shared_ptr<CSGSurface>
CSGSurfaceList::addSphere(const std::string name,
                          const Point center,
                          const Real r,
                          CSGSurface::BoundaryType boundary)
{
  checkSurfaceName(name);
  _surfaces.insert(std::make_pair(name, std::make_shared<CSGSphere>(name, center, r, boundary)));
  return _surfaces[name];
}

std::shared_ptr<CSGSurface>
CSGSurfaceList::addCylinder(const std::string name,
                            const Real x0,
                            const Real x1,
                            const Real r,
                            const std::string axis,
                            CSGSurface::BoundaryType boundary)
{
  checkSurfaceName(name);
  std::shared_ptr<CSGSurface> surf;

  if (axis == "x" || axis == "X")
    surf = std::make_shared<CSGXCylinder>(name, x0, x1, r, boundary);
  else if (axis == "y" || axis == "Y")
    surf = std::make_shared<CSGYCylinder>(name, x0, x1, r, boundary);
  else if (axis == "z" || axis == "Z")
    surf = std::make_shared<CSGZCylinder>(name, x0, x1, r, boundary);
  else
    mooseError("Axis " + axis + " not recognized for CSG cylinder. Options are x, y, or z.");

  _surfaces.insert(std::make_pair(name, surf));
  return _surfaces[name];
}

void
CSGSurfaceList::addSurface(const std::shared_ptr<CSGSurface> surf)
{
  auto name = surf->getName();
  checkSurfaceName(name);
  _surfaces.insert(std::make_pair(name, surf));
}

void
CSGSurfaceList::renameSurface(const std::shared_ptr<CSGSurface> surface, const std::string name)
{
  // check that this surface passed in is actually in the same surface that is in the surface
  // list
  auto prev_name = surface->getName();
  auto existing_surface = _surfaces.find(prev_name)->second;
  if (existing_surface != surface)
    mooseError("Surface " + prev_name + " cannot be renamed to " + name +
               " as it does not exist in this CSGBase instance.");

  checkSurfaceName(name);
  surface->setName(name);
  _surfaces.erase(prev_name);
  _surfaces.insert(std::make_pair(name, surface));
}

} // namespace CSG
