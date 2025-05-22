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
public:
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
   * @return std::shared_ptr<CSGUniverse> pointer to empty universe that is created
   */
  std::shared_ptr<CSGUniverse> addUniverse(const std::string name);

  /**
   * @brief create a universe from list of cells
   *
   * @param name unique name of universe
   * @param cells list of cell pointers to add to the universe upon creation
   * @return std::shared_ptr<CSGUniverse> pointer to universe that is created
   */
  std::shared_ptr<CSGUniverse> addUniverse(const std::string name,
                                           std::vector<std::shared_ptr<CSGCell>> cells);

  /**
   * @brief Get the all universes
   *
   * @return const std::map<std::string, std::shared_ptr<CSGUniverse>>& map of all names to
   * universes
   */
  const std::map<std::string, std::shared_ptr<CSGUniverse>> & getAllUniverses() const
  {
    return _universes;
  }

  /**
   * @brief Get the Universe by name
   *
   * @param name name of universe
   * @return const std::shared_ptr<CSGUniverse>& pointer to CSGUniverse of the specified name
   */
  const std::shared_ptr<CSGUniverse> & getUniverse(const std::string name);

  /**
   * @brief Get the root universe
   *
   * @return const std::shared_ptr<CSGUniverse>& pointer to the root universe
   */
  const std::shared_ptr<CSGUniverse> & getRoot() const { return _root_universe; };

  /**
   * @brief add an existing universe to list
   *
   * @param universe
   */
  void addUniverse(const std::shared_ptr<CSGUniverse> universe);

  /**
   * @brief rename the specified universe
   *
   * @param name new name
   */
  void renameUniverse(const std::shared_ptr<CSGUniverse> universe, const std::string name);

protected:
  /// Mapping of universe names to pointers of stored universe objects
  std::map<std::string, std::shared_ptr<CSGUniverse>> _universes;

  /// root universe for the CSGBase instance
  std::shared_ptr<CSGUniverse> _root_universe;

  /// Checks whether universe name already exists within CSGUniverseList object
  void checkUniverseName(const std::string name);
};
} // namespace CSG
