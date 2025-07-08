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
   * @return CSGUniverse & reference to empty universe that is created
   */
  CSGUniverse & addUniverse(const std::string name);

  /**
   * @brief create a universe from list of cells
   *
   * @param name unique name of universe
   * @param cells list of cell pointers to add to the universe upon creation
   * @return CSGUniverse & pointer to universe that is created
   */
  CSGUniverse & addUniverse(const std::string name, std::vector<CSGCell *> & cells);

  /**
   * @brief Get map of all names to universes in universe list
   *
   * @return std::map<std::string, std::shared_ptr<CSGUniverse>>& map of all names to
   * universes
   */
  std::map<std::string, std::shared_ptr<CSGUniverse>> & getUniverseListMap() { return _universes; }

  /**
   * @brief Get the all universes in CSGBase instance
   *
   * @return std::vector<CSGUniverse *> list of pointers to all CSGUniverse objects
   */
  std::vector<CSGUniverse *> getAllUniverses() const;

  /**
   * @brief Get the Universe by name
   *
   * @param name name of universe
   * @return CSGUniverse & reference to CSGUniverse of the specified name
   */
  CSGUniverse & getUniverse(const std::string name);

  /**
   * @brief Get the root universe
   *
   * @return CSGUniverse & reference to the root universe
   */
  CSGUniverse & getRoot() { return *_root_universe; };

  /**
   * @brief add an existing universe to list
   *
   * @param universe shared_ptr to universe to add
   */
  void addUniverse(std::shared_ptr<CSGUniverse> & universe);

  /**
   * @brief rename the specified universe
   *
   * @param universe reference to universe whose name should be renamed
   * @param name new name
   */
  void renameUniverse(CSGUniverse & universe, const std::string name);

  /// Checks whether universe name already exists within CSGUniverseList object
  void checkUniverseName(const std::string name) const;

  /// Mapping of universe names to pointers of stored universe objects
  std::map<std::string, std::shared_ptr<CSGUniverse>> _universes;

  /// root universe for the CSGBase instance
  std::shared_ptr<CSGUniverse> _root_universe;

  // Only CSGBase should be calling the methods in CSGUniverseList
  friend class CSGBase;
};
} // namespace CSG
