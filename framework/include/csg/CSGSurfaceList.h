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
#include "CSGPlane.h"

namespace CSG
{

/**
 * CSGSurfaceList creates a container for CSGSurface objects to pass to CSGBase object
 */
class CSGSurfaceList
{
protected:
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
   * @param boundary CSGSurface::BoundaryType boundary type for the surface
   * @return CSGSurface & reference to plane surface created
   */
  CSGSurface & addPlaneFromPoints(const std::string name,
                                  const Point & p1,
                                  const Point & p2,
                                  const Point & p3,
                                  CSGSurface::BoundaryType boundary);

  /**
   * @brief Create a new CSGPlane surface from coefficients (a, b, c, d) for the
   * equation of a plane: aX + bY + cZ = d
   *
   * @param name unique name of plane
   * @param a coefficient a
   * @param b coefficient b
   * @param c coefficient c
   * @param d coefficient d
   * @param boundary CSGSurface::BoundaryType boundary type for the surface
   * @return const CSGSurface & reference to plane surface created
   */
  CSGSurface & addPlaneFromCoefficients(const std::string name,
                                        const Real a,
                                        const Real b,
                                        const Real c,
                                        const Real d,
                                        CSGSurface::BoundaryType boundary);

  /**
   * @brief create a new CSGSphere surface
   *
   * @param name unique name for the sphere surface
   * @param center center point of sphere
   * @param r radius of sphere
   * @param boundary CSGSurface::BoundaryType boundary type for the surface
   * @return CSGSurface & reference to sphere surface created
   */
  CSGSurface & addSphere(const std::string name,
                         const Point center,
                         const Real r,
                         CSGSurface::BoundaryType boundary);

  /**
   * @brief create a cylinder aligned with the specified axis
   *
   * @param name unique name for the cylinder surface
   * @param x0 first coordinate of origin
   * @param x1 second coordinate of origin
   * @param r radius
   * @param axis axis alignment (x, y, or z)
   * @param boundary CSGSurface::BoundaryType boundary type for the surface
   * @return reference to CSGSurface object that is created
   */
  CSGSurface & addCylinder(const std::string name,
                           const Real x0,
                           const Real x1,
                           const Real r,
                           const std::string axis,
                           CSGSurface::BoundaryType boundary);

  /**
   * @brief Get map of all names to surfaces in surface list
   *
   * @return std::map<std::string, std::unique_ptr<CSGSurface>>& map of all names to surfaces
   */
  std::map<std::string, std::unique_ptr<CSGSurface>> & getSurfaceListMap() { return _surfaces; }

  /**
   * @brief Get list of pointers to all surfaces in surface list
   *
   * @return std::vector<CSGSurface *> list of pointers to surfaces
   */
  std::vector<CSGSurface *> getAllSurfaces() const;

  /**
   * @brief Get a surface by name
   *
   * @param name name of surface
   * @return CSGSurface & reference to CSGSurface of the specified name
   */
  CSGSurface & getSurface(const std::string name) const;

  /**
   * @brief add a surface object to existing SurfaceList. Ownership of surface will be transferred
   * to surface list object that calls this function
   *
   * @param surf CSGSurface to add
   */
  void addSurface(std::unique_ptr<CSGSurface> & surf);

  /**
   * @brief rename the specified surface
   *
   * @param name new name of surface
   */
  void renameSurface(const CSGSurface & surface, const std::string name);

  /// Checks whether surface name already exists within CSGSurfaceList object
  void checkSurfaceName(const std::string name) const;

  /// Mapping of surface names to pointers of stored surface objects
  std::map<std::string, std::unique_ptr<CSGSurface>> _surfaces;

  // Only CSGBase should be calling the methods in CSGSurfaceList
  friend class CSGBase;
};
} // namespace CSG
