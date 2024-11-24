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
#include "CSGUniverse.h"

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

  const std::map<unsigned int, std::shared_ptr<CSGSurface>> & getAllSurfaces() const
  {
    return _surface_list.getAllSurfaces();
  }

  std::shared_ptr<CSGUniverse> createRootUniverse(const std::string name);

  void generateOutput() const;

private:
  /// List of surfaces associated with CSG mesh
  CSGSurfaceList _surface_list;

  /// Pointer to root universe
  std::shared_ptr<CSGUniverse> _root_universe;
};
} // namespace CSG
