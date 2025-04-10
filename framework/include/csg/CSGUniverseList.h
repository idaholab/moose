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
   * @param name
   * @return std::shared_ptr<CSGUniverse>
   */
  std::shared_ptr<CSGUniverse> addUniverse(const std::string name);

  /**
   * @brief create a universe from list of cells
   *
   * @param name
   * @param cells
   * @return std::shared_ptr<CSGUniverse>
   */
  std::shared_ptr<CSGUniverse> addUniverse(const std::string name,
                                           std::vector<std::shared_ptr<CSGCell>> cells);

  /**
   * @brief Get the all universes
   *
   * @return const std::map<std::string, std::shared_ptr<CSGUniverse>>&
   */
  const std::map<std::string, std::shared_ptr<CSGUniverse>> & getAllUniverses() const
  {
    return _universes;
  }

  /**
   * @brief Get the Universe by name
   *
   * @param name
   * @return const std::shared_ptr<CSGUniverse>&
   */
  const std::shared_ptr<CSGUniverse> & getUniverse(const std::string name);

  /**
   * @brief Get the root universe
   *
   * @return const std::shared_ptr<CSGUniverse>&
   */
  const std::shared_ptr<CSGUniverse> & getRoot() const { return _root_universe; };

  /**
   * @brief add an existing universe to list
   *
   * @param universe
   */
  void addUniverse(const std::pair<std::string, std::shared_ptr<CSGUniverse>> universe);

  void renameRoot(const std::string name);

protected:
  /// Mapping of universe names to pointers of stored universe objects
  std::map<std::string, std::shared_ptr<CSGUniverse>> _universes;

  std::shared_ptr<CSGUniverse> _root_universe;

  /// Checks whether universe name already exists within CSGUniverseList object
  void checkUniverseName(const std::string name);
};
} // namespace CSG
