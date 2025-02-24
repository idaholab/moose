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
#include "nlohmann/json.h"

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

  /**
   * @brief Create a plane from three points
   *
   * @param name surface name
   * @param p1 point 1
   * @param p2 point 2
   * @param p3 point 3
   * @return shared pointer to CSGSurface object
   */
  std::shared_ptr<CSGSurface>
  createPlaneFromPoints(const std::string name, const Point p1, const Point p2, const Point p3)
  {
    return _surface_list.addPlaneFromPoints(name, p1, p2, p3);
  }

  /**
   * @brief Create a plane from coefficients for the equation: ax + by + cz = d
   *
   * @param name surface name
   * @param a coefficient a
   * @param b coefficient b
   * @param c coefficient c
   * @param d coefficient d
   * @return shared pointer to CSGSurface object
   */
  std::shared_ptr<CSGSurface>
  createPlaneFromCoefficients(const std::string name, const Real a, const Real b, const Real c, const Real d)
  {
    return _surface_list.addPlaneFromCoefficients(name, a, b, c, d);
  }

  /**
   * @brief Create a Sphere At Origin
   *
   * @param name surface name
   * @param r radius
   * @return shared pointer to CSGSurface object
   */
  std::shared_ptr<CSGSurface>
  createSphereAtOrigin(const std::string name, const Real r)
  {
    return _surface_list.addSphere(name, 0.0, 0.0, 0.0, r);
  }

  /**
   * @brief Create a Sphere centered at a point
   *
   * @param name surface name
   * @param center a point defining the center
   * @param r radius
   * @return shared pointer to CSGSurface object
   */
  std::shared_ptr<CSGSurface>
  createSphereAtPoint(const std::string name, const Point center, const Real r)
  {
    return _surface_list.addSphere(name, center(0), center(1), center(2), r);
  }

  /**
   * @brief Create a Sphere at a point (x, y, z)
   *
   * @param name surface name
   * @param x x coordinate for center
   * @param y y coordinate for center
   * @param z z coordinate for center
   * @param r radius
   * @return shared pointer to CSGSurface object
   */
  std::shared_ptr<CSGSurface>
  createSphereAtXYZ(const std::string name, const Real x, const Real y, const Real z, const Real r)
  {
    return _surface_list.addSphere(name, x, y, z, r);
  }

  /**
   * @brief Create a Cylinder aligned with an axis (x, y or z) at the point
   * (x0, x1), where x0 and x1 correspond to:
   * x aligned: (y, z)
   * y aligned: (x, z)
   * z aligned: (x, y)
   *
   * @param name surface name
   * @param x0 first coordinate for center
   * @param x1 second coordinate for center
   * @param r radius
   * @return shared pointer to CSGSurface object
   */
  std::shared_ptr<CSGSurface>
  createCylinder(const std::string name, const Real x0, const Real x1, const Real r, const std::string axis)
  {
    return _surface_list.addCylinder(name, x0, x1, r, axis);
  }

  /**
   * @brief Get all surface objects
   *
   * @return map of names to CSGSurface objects
   */
  const std::map<std::string, std::shared_ptr<CSGSurface>> & getAllSurfaces() const
  {
    return _surface_list.getAllSurfaces();
  }

  /**
   * @brief Get a Surface object by name
   *
   * @param name surface name
   * @return shared pointer to CSGSurface object
   */
  const std::shared_ptr<CSGSurface> & getSurfaceByName(const std::string name)
  {
    return _surface_list.getSurface(name);
  }

  /**
   * @brief Create a Root Universe object
   *
   * @param name root universe name
   * @return shared pointer to CSGUniverse
   */
  std::shared_ptr<CSGUniverse> createRootUniverse(const std::string name);

  /**
   * @brief Get the Root Universe object
   *
   * @return  shared pointer to CSGUniverse
   */
  std::shared_ptr<CSGUniverse> getRootUniverse();

  /**
   * @brief generate the JSON representation output for the CSG object
   *
   */
  nlohmann::json generateOutput() const;

private:
  /// List of surfaces associated with CSG mesh
  CSGSurfaceList _surface_list;

  /// Pointer to root universe
  std::shared_ptr<CSGUniverse> _root_universe;
};
} // namespace CSG
