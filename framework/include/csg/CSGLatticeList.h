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

  /**
   * @brief add an existing lattice to list. Ownership of lattice will be transferred to
   * CSGLatticeList object that calls this function
   *
   * @param lattice pointer to lattice to add
   * @return reference to CSGLattice that is passed in
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
   * @brief Get all the lattices in CSGBase instance
   *
   * @return list of references to all CSGLattice objects
   */
  std::vector<std::reference_wrapper<const CSGLattice>> getAllLattices() const;

  /**
   * @brief Get a Lattice from the list by its name
   *
   * @param name name of lattice
   * @return reference to CSGLattice of the specified name
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

  /// Mapping of lattice names to pointers of stored lattice objects
  std::unordered_map<std::string, std::unique_ptr<CSGLattice>> _lattices;

  // Only CSGBase should be calling the methods in CSGLatticeList
  friend class CSGBase;
};
} // namespace CSG
