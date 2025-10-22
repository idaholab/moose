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

#ifdef MOOSE_UNIT_TEST
#include "gtest/gtest.h"
#endif

namespace CSG
{

/**
 * CSGLattice is the abstract class for defining lattices.
 */
class CSGLattice
{
public:
  /**
   * @brief Construct a new CSGLattice of specific type without universes defined
   *
   * @param name unique name of lattice
   * @param lattice_type type of lattice
   */
  CSGLattice(const std::string & name, const std::string & lattice_type);

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
   * @param name of universe to search for
   *
   * @return true if universe of that name exists in lattice
   */
  bool hasUniverse(const std::string & name) const;

  /**
   * @brief check if the speficied dimensions of the lattice are valid, raise error if not
   *
   */
  void checkDimensions() const;

  /**
   * @brief get the map of data that defines the geometric dimensions of the lattice
   *
   * @return map of string dimension name to value of that dimension
   */
  virtual std::unordered_map<std::string, std::any>
  getDimensions() const = 0; // pure virtual function

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
   * @param index pair of ints that specify the location in lattice
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
   * @brief check that any provided list of list of CSGUniverses are the correct dimensions for the
   * type of lattice
   *
   * @param universes list of list of universes to be used to define the lattice structure
   * @return true if universe dimensions are valid
   */
  virtual bool
  isValidUniverseMap(std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes)
      const = 0; // pure virtual function

  /// Operator overload for checking if two CSGLattice objects are equal
  // bool operator==(const CSGLattice & other) const;

  /// Operator overload for checking if two CSGLattice objects are not equal
  // bool operator!=(const CSGLattice & other) const;

protected:
  // set the name of the lattices - intentionally not public because
  // name needs to be managed at the CSGLatticeList level
  void setName(const std::string & name) { _name = name; }

  /// @brief assign the vectors of universes as the lattice elements
  /// @param universes
  void setUniverses(std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes);

  /**
   * @brief replace the element at specified index in the lattice with the provided CSGUniverse.
   * This will check that the _universe_map has been initialized and that the index is valid.
   *
   * @param universe universe to add to the lattice at the location index
   * @param index location in lattice replace with provided universe
   */
  void setUniverseAtIndex(std::reference_wrapper<const CSGUniverse> universe,
                          const std::pair<int, int> index);

  /**
   * @brief For the lattice type, check that any values in the _dimensions variable are valid
   *
   * @return true if valid, otherwise false
   */
  virtual bool hasValidDimensions() const = 0; // pure virtual function

  /**
   * @brief change the value of a dimension parameter for the lattice. Each lattice type shall
   * determine which dimensions (dim_name) are able to be changed and check for validity of the new
   * value.
   *
   * @param dim_name
   * @param dim_value
   */
  virtual void updateDimension(const std::string & dim_name, std::any dim_value); // pure virtual

  /// Name of lattice
  std::string _name;

  /// Type of lattice
  const std::string _lattice_type;

  /// Universes in the arrangement of how they appear in the lattice; dimensions depends on lattice type
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> _universe_map;

  // CSGLatticeList needs to be friend to access setName()
  friend class CSGLatticeList;
  // CSGBase needed for access updateDimension
  friend class CSGBase;

#ifdef MOOSE_UNIT_TEST
  /// Friends for unit testing
  ///@{
  FRIEND_TEST(CSGLatticeTest, testCartSetUniverses);
  FRIEND_TEST(CSGLatticeTest, testCartSetUniverseAtIndex);
  FRIEND_TEST(CSGLatticeTest, testSetName);
  ///@}
#endif
};
}
