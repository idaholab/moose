//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseApp.h"

#include "CSGUniverse.h"

namespace CSG
{

/**
 * CSGLattice is the abstract class for defining lattices.
 */
class CSGLattice
{
public:
  /**
   * Default constructor
   *
   * @param name unique name of lattice
   */
  CSGLattice(const std::string & name);

  /**
   * @brief Construct a new CSGLattice
   *
   * @param name unique name of lattice
   * @param lattice_type type of lattice
   */
  CSGLattice(const std::string & name, const std::string & lattice_type);

  CSGLattice(const std::string & name,
             std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes,
             const std::string & lattice_type);

  /**
   * Destructor
   */
  virtual ~CSGLattice() = default;

  /**
   * @brief Get the name of lattice
   *
   * @return std::string name of lattice
   */
  const std::string & getName() const { return _name; }

  /**
   * @brief Get the lattice type
   *
   * @return std::string type of lattice
   */
  const std::string & getType() const { return _lattice_type; }

  /**
   * @brief Get the arrangement of CSGUniverses in the lattice
   *
   * @return list of list of pointers to universes
   */
  const std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> & getUniverses()
  {
    return _universe_map;
  }

  /**
   * @brief whether or not the universe of the specified name exists in the lattice
   *
   * @param name of universe
   *
   * @return true if universe of that name exists in lattice
   */
  bool hasUniverse(const std::string & name) const;

  /**
   * @brief check if dimensions of the universe map are valid, raise error if not
   *
   */
  void checkDimensions() const;

  /**
   * @brief get the map of data that defines the geometric dimensions of the lattice
   *
   * @return map of string to any value
   */
  std::unordered_map<std::string, std::any> getDimensions() { return _dimensions; }

  /**
   * @brief Checks if the given index location is a valid index for the lattice
   *
   * @param index location
   * @return true if index is valid for the lattice
   */
  virtual bool isValidIndex(const std::pair<int, int> index) const = 0; // Pure virtual function

  /**
   * @brief Get the universe located at the given index
   *
   * @param index location in lattice
   * @return universe at the specified location
   */
  std::reference_wrapper<const CSGUniverse> getUniverseAtIndex(const std::pair<int, int> index);

  /**
   * @brief get all locations in lattice where universe of the specified name exists
   *
   * @param univ_name name of universe
   * @return vector of locations (pairs of ints)
   */
  const std::vector<std::pair<int, int>> getUniverseIndices(const std::string & univ_name) const;

  /**
   * @brief given a point, check if it is within the bounds of the specified lattice
   *
   * @param point to check
   * @return true if within lattice bounds
   */
  virtual bool isPointInLattice(Point point) const = 0; // pure virtual function

  /**
   * @brief given a point, get the index of the lattice universe in which it lies; raises an
   * error if point is not within the bounds of the lattice
   *
   * @param point
   * @return index of universe where point is located
   */
  std::pair<int, int> getIndexAtPoint(Point point);

  /// Operator overload for checking if two CSGLattice objects are equal
  // bool operator==(const CSGLattice & other) const;

  /// Operator overload for checking if two CSGLattice objects are not equal
  // bool operator!=(const CSGLattice & other) const;

protected:
  // set the name of the lattices - intentionally not public because
  // name needs to be managed at the CSGLatticeList level
  void setName(const std::string & name) { _name = name; }

  /// given a point, get the index of the lattice element that contains it (assumes point is within bounds)
  virtual std::pair<int, int> getIndex(Point point) const = 0; // pure virtual function

  /// @brief assign the vectors of universes as the lattice elements
  /// @param universes
  void setUniverses(std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes);

  /**
   * @brief For the lattice type, check that the dimension of _universe_map are valid
   *
   * @return true if valid, otherwise false
   */
  virtual bool hasValidDimensions() const = 0; // pure virtual function

  /// Name of lattice
  std::string _name;

  /// Type of lattice
  const std::string _lattice_type;

  /// Universes in the arrangement of how they appear in the lattice; dimensions depends on lattice type
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> _universe_map;

  /// Dimensional data: maps a string to the value to define necessary lattice dimensions
  const std::unordered_map<std::string, std::any> _dimensions;
};
}
