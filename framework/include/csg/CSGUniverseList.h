//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CSGUniverse.h"

namespace CSG
{

/**
 * CSGUniverseList creates a container for CSGUniverse objects to pass to CSGBase
 */
class CSGUniverseList
{
protected:
  /**
   * Default constructor
   */
  CSGUniverseList();

  /**
   * Destructor
   */
  virtual ~CSGUniverseList() = default;

  /**
   * @brief create an empty universe
   *
   * @param name unique name of universe
   * @return reference to empty universe that is created
   */
  CSGUniverse & addUniverse(const std::string & name);

  /**
   * @brief Get map of all names to universes in universe list
   *
   * @return map of all names to CSGUniverse pointers
   */
  std::unordered_map<std::string, std::unique_ptr<CSGUniverse>> & getUniverseListMap()
  {
    return _universes;
  }

  /**
   * @brief Get all the universes in CSGBase instance
   *
   * @return list of references to all CSGUniverse objects
   */
  std::vector<std::reference_wrapper<const CSGUniverse>> getAllUniverses() const;

  /**
   * @brief Get a Universe from the list by its name
   *
   * @param name name of universe
   * @return reference to CSGUniverse of the specified name
   */
  CSGUniverse & getUniverse(const std::string & name);

  /**
   * @brief Get the root universe
   *
   * @return reference to the root universe
   */
  const CSGUniverse & getRoot() const { return *_root_universe; };

  /**
   * @brief add an existing universe to list. Ownership of universe will be transferred to universe
   * list object that calls this function
   *
   * @param universe pointer to universe to add
   */
  void addUniverse(std::unique_ptr<CSGUniverse> universe);

  /**
   * @brief rename the specified universe
   *
   * @param universe reference to universe whose name should be renamed
   * @param name new name
   */
  void renameUniverse(const CSGUniverse & universe, const std::string & name);

  /// Checks whether universe name already exists within CSGUniverseList object
  void checkUniverseName(const std::string & name) const;

  /// Mapping of universe names to pointers of stored universe objects
  std::unordered_map<std::string, std::unique_ptr<CSGUniverse>> _universes;

  /// root universe for the CSGBase instance
  const CSGUniverse * _root_universe;

  // Only CSGBase should be calling the methods in CSGUniverseList
  friend class CSGBase;
};
} // namespace CSG
