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

  const std::map<unsigned int, std::shared_ptr<CSGSurface>> & getAllSurfaces() const
  {
    return _surfaces;
  }

protected:
  /// Mapping of surface ids to pointers of stored surface objects
  std::map<unsigned int, std::shared_ptr<CSGSurface>> _surfaces;

  /// Next available surface id
  unsigned int _next_surface_id;

  /// Mapping of surface name to surface id
  std::map<std::string, unsigned int> _surface_name_id_mapping;
};
} // namespace CSG
