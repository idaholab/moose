//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CSGSurface.h"

namespace CSG
{

/**
 * CSGSurfaceList creates a container for CSGSurface objects to pass to CSGMesh
 */
class CSGSurfaceList
{
public:
  /**
   * Default constructor
   */
  CSGSurfaceList();

  /**
   * Destructor
   */
  virtual ~CSGSurfaceList() = default;

  /**
   * @brief Create a new CSGPlane surface from three non co-linear points
   *
   * @param name unique name of plane
   * @param p1 point 1
   * @param p2 point 2
   * @param p3 point 3
   *
   * @return std::shared_ptr<CSGSurface> pointer to plane surface created
   */
  std::shared_ptr<CSGSurface>
  addPlaneFromPoints(const std::string name, const Point p1, const Point p2, const Point p3);

  /**
   * @brief Create a new CSGPlane surface from coefficients (a, b, c, d) for the
   * equation of a plane: aX + bY + cZ = d
   *
   * @param name unique name of plane
   * @param a coefficient a
   * @param b coefficient b
   * @param c coefficient c
   * @param d coefficient d
   *
   * @return std::shared_ptr<CSGSurface> pointer to plane surface created
   */
  std::shared_ptr<CSGSurface> addPlaneFromCoefficients(
      const std::string name, const Real a, const Real b, const Real c, const Real d);

  /**
   * @brief create a new CSGSphere surface
   *
   * @param name unique name for the sphere surface
   * @param center center point of sphere
   * @param r radius of sphere
   *
   * @return std::shared_ptr<CSGSurface> pointer to sphere surface created
   */
  std::shared_ptr<CSGSurface> addSphere(const std::string name, const Point center, const Real r);

  /**
   * @brief create a cylinder aligned with the specified axis
   *
   * @param name unique name for the cylinder surface
   * @param x0 first coordinate of origin
   * @param x1 second coordinate of origin
   * @param r radius
   * @param axis axis alignment (x, y, or z)
   * @return std::shared_ptr<CSGSurface>
   */
  std::shared_ptr<CSGSurface> addCylinder(
      const std::string name, const Real x0, const Real x1, const Real r, const std::string axis);

  /**
   * @brief Get the all aurfaces
   *
   * @return const std::map<std::string, std::shared_ptr<CSGSurface>>& map of all names to surfaces
   */
  const std::map<std::string, std::shared_ptr<CSGSurface>> & getAllSurfaces() const
  {
    return _surfaces;
  }

  /**
   * @brief Get a surface by name
   *
   * @param name name of surface
   * @return const std::shared_ptr<CSGSurface>& pointer to CSGSurface of the specified name
   */
  const std::shared_ptr<CSGSurface> & getSurface(const std::string name);

  /**
   * @brief add a surface object to existing SurfaceList
   *
   * @param surf CSGSurface to add
   */
  void addSurface(const std::shared_ptr<CSGSurface> surf);

  /**
   * @brief rename the specified cell
   *
   * @param name new name of surface
   */
  void renameSurface(const std::shared_ptr<CSGSurface> surface, const std::string name);

protected:
  /// Mapping of surface names to pointers of stored surface objects
  std::map<std::string, std::shared_ptr<CSGSurface>> _surfaces;

  /// Checks whether surface name already exists within CSGSurfaceList object
  void checkSurfaceName(const std::string name);
};
} // namespace CSG
