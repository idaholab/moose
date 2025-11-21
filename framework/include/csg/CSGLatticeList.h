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
#include "CSGCartesianLattice.h"
#include "CSGHexagonalLattice.h"

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

  /**
   * @brief add a Cartesian lattice whose layout is defined by the set of universes
   *
   * @param name unique name identifier of the lattice
   * @param pitch flat-to-flat size of the lattice elements
   * @param universes list of list of universes that define the lattice layout
   * @return reference to new Cartesian lattice
   */
  CSGLattice & addCartesianLattice(
      const std::string & name,
      const Real pitch,
      std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes);

  /**
   * @brief add a Cartesian lattice of the specified dimension but does not have a defined
   * universe layout yet.
   *
   * @param name unique name identifier of the lattice
   * @param pitch flat-to-flat size of the lattice elements
   * @return reference to new Cartesian lattice
   */
  CSGLattice & addCartesianLattice(const std::string & name, const Real pitch);

  /**
   * @brief Add an empty hexagonal lattice
   *
   * @param name unique lattice name
   * @param pitch  flat‐to‐flat distance between adjacent centers
   * @return reference to new hexagonal lattice
   */
  CSGLattice & addHexagonalLattice(const std::string & name, Real pitch);

  /**
   * @brief add a hexagonal lattice whose layout is defined by the set of universes. Universes
   * should be arranged by rows and correspond to a hexagonal lattice with x-orientation.
   *
   * @param name unique name identifier of the lattice
   * @param pitch flat-to-flat size of the lattice elements
   * @param universes list of list of universes that define the lattice layout
   * @return reference to new hexagonal lattice
   */
  CSGLattice & addHexagonalLattice(
      const std::string & name,
      const Real pitch,
      std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes);

  /**
   * @brief add an existing universe to list. Ownership of universe will be transferred to universe
   * list object that calls this function
   *
   * @param universe pointer to universe to add
   * @return reference to universe that is passed in
   */
  CSGLattice & addLattice(std::unique_ptr<CSGLattice> lattice);

  /**
   * @brief return whether lattice with given name exists in lattice list
   *
   * @param name name of the lattice
   * @return true if lattice name exists, otherwise false
   */
  bool hasLattice(const std::string & name) const
  {
    return _lattices.find(name) != _lattices.end();
  }

  /**
   * @brief Get map of all names to lattices in lattice list
   *
   * @return map of all names to CSGLattice pointers
   */
  std::unordered_map<std::string, std::unique_ptr<CSGLattice>> & getLatticeListMap()
  {
    return _lattices;
  }

  /**
   * @brief Get const map of all names to lattices in lattice list
   *
   * @return map of all names to CSGLattice pointers
   */
  const std::unordered_map<std::string, std::unique_ptr<CSGLattice>> & getLatticeListMap() const
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
   * @brief Get a Lattice from the list by its name
   *
   * @param name name of universe
   * @return reference to CSGUniverse of the specified name
   */
  CSGLattice & getLattice(const std::string & name) const;

  /**
   * @brief rename the specified lattice
   *
   * @param lattice reference to lattice whose name should be renamed
   * @param name new name
   */
  void renameLattice(const CSGLattice & lattice, const std::string & name);

  /// Operator overload for checking if two CSGLatticeList objects are equal
  bool operator==(const CSGLatticeList & other) const;

  /// Operator overload for checking if two CSGLatticeList objects are not equal
  bool operator!=(const CSGLatticeList & other) const;

  /// Mapping of universe names to pointers of stored universe objects
  std::unordered_map<std::string, std::unique_ptr<CSGLattice>> _lattices;

  // Only CSGBase should be calling the methods in CSGUniverseList
  friend class CSGBase;
};
} // namespace CSG
