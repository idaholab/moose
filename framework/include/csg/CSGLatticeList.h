//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CSGLattice.h"

namespace CSG
{

/**
 * CSGLatticeList creates a container for CSGLattice objects to pass to CSGBase
 */
class CSGLatticeList
{
protected:
  /**
   * Default constructor
   */
  CSGLatticeList();

  /**
   * Destructor
   */
  virtual ~CSGLatticeList() = default;

  // /**
  //  * @brief create an empty lattice
  //  *
  //  * @param name unique name of universe
  //  * @return reference to empty universe that is created
  //  */
  // CSGLattice & addLattice(const std::string & name);

  /**
   * @brief add an existing universe to list. Ownership of universe will be transferred to universe
   * list object that calls this function
   *
   * @param universe pointer to universe to add
   * @return reference to universe that is passed in
   */
  CSGLattice & addLattice(std::unique_ptr<CSGLattice> lattice);

  /**
   * @brief Get map of all names to universes in universe list
   *
   * @return map of all names to CSGLattice pointers
   */
  std::unordered_map<std::string, std::unique_ptr<CSGLattice>> & getLatticeListMap()
  {
    return _lattices;
  }

  /**
   * @brief Get all the universes in CSGBase instance
   *
   * @return list of references to all CSGUniverse objects
   */
  std::vector<std::reference_wrapper<const CSGLattice>> getAllLattices() const;

  /**
   * @brief Get a Universe from the list by its name
   *
   * @param name name of universe
   * @return reference to CSGUniverse of the specified name
   */
  CSGLattice & getLattice(const std::string & name) const;

  /**
   * @brief rename the specified universe
   *
   * @param universe reference to universe whose name should be renamed
   * @param name new name
   */
  void renameLattice(const CSGLattice & lattice, const std::string & name);

  /// Mapping of universe names to pointers of stored universe objects
  std::unordered_map<std::string, std::unique_ptr<CSGLattice>> _lattices;

  // Only CSGBase should be calling the methods in CSGUniverseList
  friend class CSGBase;
};
} // namespace CSG
