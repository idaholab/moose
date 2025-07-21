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
 * CSGSurfaceList is a container for storing CSGSurface objects in the CSGBase object
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
   * @brief Get map of all names to surfaces in surface list
   *
   * @return map of all names to CSGSurface pointers
   */
  std::unordered_map<std::string, std::unique_ptr<CSGSurface>> & getSurfaceListMap()
  {
    return _surfaces;
  }

  /**
   * @brief Get list of references to all surfaces in surface list
   *
   * @return list of references to surfaces
   */
  std::vector<std::reference_wrapper<const CSGSurface>> getAllSurfaces() const;

  /**
   * @brief Get a surface by name
   *
   * @param name name of surface
   * @return reference to CSGSurface of the specified name
   */
  CSGSurface & getSurface(const std::string & name) const;

  /**
   * @brief add a surface object to existing SurfaceList. Ownership of surface will be transferred
   * to surface list object that calls this function
   *
   * @param surf CSGSurface to add
   */
  CSGSurface & addSurface(std::unique_ptr<CSGSurface> & surf);

  /**
   * @brief rename the specified surface
   *
   * @param name new name of surface
   */
  void renameSurface(const CSGSurface & surface, const std::string & name);

  /// Checks whether surface name already exists within CSGSurfaceList object
  void checkSurfaceName(const std::string & name) const;

  /// Mapping of surface names to pointers of stored surface objects
  std::unordered_map<std::string, std::unique_ptr<CSGSurface>> _surfaces;

  // Only CSGBase should be calling the methods in CSGSurfaceList
  friend class CSGBase;
};
} // namespace CSG
