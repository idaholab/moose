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
   * Copy constructor
   */
  CSGSurfaceList(const CSGSurfaceList & other_surface_list);

  /**
   * Destructor
   */
  virtual ~CSGSurfaceList() = default;

  /**
   * @brief Get non-const map of all names to surfaces in surface list
   *
   * @return map of all names to CSGSurface pointers
   */
  std::unordered_map<std::string, std::unique_ptr<CSGSurface>> & getSurfaceListMap()
  {
    return _surfaces;
  }

  /**
   * @brief Get const map of all names to surfaces in surface list
   *
   * @return map of all names to CSGSurface pointers
   */
  const std::unordered_map<std::string, std::unique_ptr<CSGSurface>> & getSurfaceListMap() const
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
   * @brief return whether surface with given name exists in surface list
   *
   * @param name name of surface
   * @return true if surface name exists, false otherwise
   */
  bool hasSurface(const std::string & name) const
  {
    return _surfaces.find(name) != _surfaces.end();
  }

  /**
   * @brief Get a surface by name
   *
   * @param name name of surface
   *
   * @return reference to CSGSurface of the specified name
   */
  CSGSurface & getSurface(const std::string & name) const;

  /**
   * @brief add a surface object to existing SurfaceList. Ownership of surface will be transferred
   * to surface list object that calls this function
   *
   * @param surf CSGSurface to add
   *
   * @return reference to CSGSurface
   */
  CSGSurface & addSurface(std::unique_ptr<CSGSurface> surf);

  /**
   * @brief rename the specified surface
   *
   * @param name new name of surface
   */
  void renameSurface(const CSGSurface & surface, const std::string & name);

  /// Operator overload for checking if two CSGSurfaceList objects are equal
  bool operator==(const CSGSurfaceList & other) const;

  /// Operator overload for checking if two CSGSurfaceList objects are not equal
  bool operator!=(const CSGSurfaceList & other) const;

  /// Mapping of surface names to pointers of stored surface objects
  std::unordered_map<std::string, std::unique_ptr<CSGSurface>> _surfaces;

  // Only CSGBase should be calling the methods in CSGSurfaceList
  friend class CSGBase;
};
} // namespace CSG
