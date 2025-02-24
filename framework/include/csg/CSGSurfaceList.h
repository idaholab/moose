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

  std::shared_ptr<CSGSurface>
  addPlaneFromPoints(const std::string name, const Point p1, const Point p2, const Point p3);

  std::shared_ptr<CSGSurface>
  addPlaneFromCoefficients(const std::string name, const Real a, const Real b, const Real c, const Real d);

  std::shared_ptr<CSGSurface>
  addSphere(const std::string name, const Real x, const Real y, const Real z, const Real r);

  std::shared_ptr<CSGSurface>
  addCylinder(const std::string name, const Real x0, const Real x1, const Real r, const std::string axis);

  const std::map<std::string, std::shared_ptr<CSGSurface>> & getAllSurfaces() const
  {
    return _surfaces;
  }

  const std::shared_ptr<CSGSurface> & getSurface(const std::string name);

protected:
  /// Mapping of surface names to pointers of stored surface objects
  std::map<std::string, std::shared_ptr<CSGSurface>> _surfaces;

  /// Checks whether surface name already exists within CSGSurfaceList object
  void checkSurfaceName(const std::string name);

};
} // namespace CSG
