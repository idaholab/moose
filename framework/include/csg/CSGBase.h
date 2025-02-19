//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CSGSurfaceList.h"
#include "CSGPlane.h"
#include "CSGSphere.h"
#include "CSGXCylinder.h"
#include "CSGYCylinder.h"
#include "CSGZCylinder.h"
#include "CSGUniverse.h"
#include "CSGMaterialCell.h"
#include "CSGVoidCell.h"
#include "CSGIntersection.h"
#include "CSGHalfspace.h"

namespace CSG
{

/**
 * CSGBase creates an internal representation of a Constructive Solid Geometry (CSG)
 * mesh based on an existing MooseMesh instance.
 */
class CSGBase
{
public:
  /**
   * Default constructor
   */
  CSGBase();

  /**
   * Destructor
   */
  ~CSGBase();

  std::shared_ptr<CSGSurface>
  createPlaneFromPoints(const std::string name, const Point p1, const Point p2, const Point p3)
  {
    return _surface_list.addPlaneFromPoints(name, p1, p2, p3);
  }

  std::shared_ptr<CSGSurface>
  createPlaneFromCoefficients(const std::string name, const Real a, const Real b, const Real c, const Real d)
  {
    return _surface_list.addPlaneFromCoefficients(name, a, b, c, d);
  }

  std::shared_ptr<CSGSurface>
  createSphereAtOrigin(const std::string name, const Real r)
  {
    return _surface_list.addSphere(name, 0.0, 0.0, 0.0, r);
  }

  std::shared_ptr<CSGSurface>
  createSphereAtPoint(const std::string name, const Point center, const Real r)
  {
    return _surface_list.addSphere(name, center(0), center(1), center(2), r);
  }

  std::shared_ptr<CSGSurface>
  createSphereAtXYZ(const std::string name, const Real x, const Real y, const Real z, const Real r)
  {
    return _surface_list.addSphere(name, x, y, z, r);
  }

  std::shared_ptr<CSGSurface>
  createCylinder(const std::string name, const Real x0, const Real x1, const Real r, const std::string axis)
  {
    return _surface_list.addCylinder(name, x0, x1, r, axis);
  }

  const std::map<std::string, std::shared_ptr<CSGSurface>> & getAllSurfaces() const
  {
    return _surface_list.getAllSurfaces();
  }

  const std::shared_ptr<CSGSurface> & getSurfaceByName(const std::string name)
  {
    return _surface_list.getSurface(name);
  }

  std::shared_ptr<CSGUniverse> createRootUniverse(const std::string name);

  std::shared_ptr<CSGUniverse> getRootUniverse();

  void generateOutput() const;

private:
  /// List of surfaces associated with CSG mesh
  CSGSurfaceList _surface_list;

  /// Pointer to root universe
  std::shared_ptr<CSGUniverse> _root_universe;
};
} // namespace CSG
