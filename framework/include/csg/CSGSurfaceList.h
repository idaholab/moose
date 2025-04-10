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
   * @brief create a plane defined by 3 points
   *
   * @param name
   * @param p1
   * @param p2
   * @param p3
   * @return std::shared_ptr<CSGSurface>
   */
  std::shared_ptr<CSGSurface>
  addPlaneFromPoints(const std::string name, const Point p1, const Point p2, const Point p3);

  /**
   * @brief create a plane from coefficients of the equation (aX + bY + cZ = d)
   *
   * @param name
   * @param a
   * @param b
   * @param c
   * @param d
   * @return std::shared_ptr<CSGSurface>
   */
  std::shared_ptr<CSGSurface> addPlaneFromCoefficients(
      const std::string name, const Real a, const Real b, const Real c, const Real d);

  /**
   * @brief create a sphere surface
   *
   * @param name
   * @param center
   * @param r
   * @return std::shared_ptr<CSGSurface>
   */
  std::shared_ptr<CSGSurface> addSphere(const std::string name, const Point center, const Real r);

  /**
   * @brief create a cylinder aligned with axis
   *
   * @param name
   * @param x0
   * @param x1
   * @param r
   * @param axis
   * @return std::shared_ptr<CSGSurface>
   */
  std::shared_ptr<CSGSurface> addCylinder(
      const std::string name, const Real x0, const Real x1, const Real r, const std::string axis);

  /**
   * @brief Get the all aurfaces
   *
   * @return const std::map<std::string, std::shared_ptr<CSGSurface>>&
   */
  const std::map<std::string, std::shared_ptr<CSGSurface>> & getAllSurfaces() const
  {
    return _surfaces;
  }

  /**
   * @brief Get a surface by name
   *
   * @param name
   * @return const std::shared_ptr<CSGSurface>&
   */
  const std::shared_ptr<CSGSurface> & getSurface(const std::string name);

  /**
   * @brief add a surface object to existing SurfaceList
   *
   * @param surf
   */
  void addSurface(const std::pair<std::string, std::shared_ptr<CSGSurface>> surf);

protected:
  /// Mapping of surface names to pointers of stored surface objects
  std::map<std::string, std::shared_ptr<CSGSurface>> _surfaces;

  /// Checks whether surface name already exists within CSGSurfaceList object
  void checkSurfaceName(const std::string name);
};
} // namespace CSG
