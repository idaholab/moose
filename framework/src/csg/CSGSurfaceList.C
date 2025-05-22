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
CSGSurfaceList::checkSurfaceName(const std::string name)
{
  if (_surfaces.find(name) != _surfaces.end())
    mooseError("Surface with name " + name + " already exists in geoemetry.");
}

const std::shared_ptr<CSGSurface> &
CSGSurfaceList::getSurface(const std::string name)
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
                                   const Point p3)
{
  checkSurfaceName(name);
  _surfaces.insert(std::make_pair(name, std::make_shared<CSGPlane>(name, p1, p2, p3)));
  return _surfaces[name];
}

std::shared_ptr<CSGSurface>
CSGSurfaceList::addPlaneFromCoefficients(
    const std::string name, const Real a, const Real b, const Real c, const Real d)
{
  checkSurfaceName(name);
  _surfaces.insert(std::make_pair(name, std::make_shared<CSGPlane>(name, a, b, c, d)));
  return _surfaces[name];
}

std::shared_ptr<CSGSurface>
CSGSurfaceList::addSphere(const std::string name, const Point center, const Real r)
{
  checkSurfaceName(name);
  _surfaces.insert(std::make_pair(name, std::make_shared<CSGSphere>(name, center, r)));
  return _surfaces[name];
}

std::shared_ptr<CSGSurface>
CSGSurfaceList::addCylinder(
    const std::string name, const Real x0, const Real x1, const Real r, const std::string axis)
{
  checkSurfaceName(name);
  std::shared_ptr<CSGSurface> surf;

  if (axis == "x" || axis == "X")
    surf = std::make_shared<CSGXCylinder>(name, x0, x1, r);
  else if (axis == "y" || axis == "Y")
    surf = std::make_shared<CSGYCylinder>(name, x0, x1, r);
  else if (axis == "z" || axis == "Z")
    surf = std::make_shared<CSGZCylinder>(name, x0, x1, r);
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
  checkSurfaceName(name);

  auto prev_name = surface->getName();
  surface->setName(name);
  _surfaces.erase(prev_name);
  _surfaces.insert(std::make_pair(name, surface));
}

} // namespace CSG
