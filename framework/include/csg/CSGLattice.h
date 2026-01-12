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
#include "JsonOutputUtils.h"

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
   * @brief Construct a new CSGLattice of specific type without an outer defined
   *
   * @param name unique name of lattice
   * @param lattice_type type of lattice
   */
  CSGLattice(const std::string & name, const std::string & lattice_type);

  /**
   * @brief Construct a new CSGLattice of specific type with material string outer
   *
   * @param name unique name of lattice
   * @param lattice_type type of lattice
   * @param outer_name name of outer material name that fills space around lattice elements
   */
  CSGLattice(const std::string & name,
             const std::string & lattice_type,
             const std::string outer_name);

  /**
   * @brief Construct a new CSGLattice of specific type with a universe outer
   *
   * @param name unique name of lattice
   * @param lattice_type type of lattice
   * @param outer_universe pointer to outer universe that fills space around lattice elements
   */
  CSGLattice(const std::string & name,
             const std::string & lattice_type,
             const CSGUniverse & outer_universe);

  /**
   * Destructor
   */
  virtual ~CSGLattice() = default;

  // Pure virtual clone method that derived classes must implement
  virtual std::unique_ptr<CSGLattice> clone() const = 0;

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
   * @brief Get the type of outer that fills the space around the lattice elements
   *
   * @return string of outer type: CSG_MATERIAL, UNIVERSE, or VOID
   */
  const std::string getOuterType() const { return _outer_type; }

  /**
   * @brief Get the outer universe if outer type is UNIVERSE
   *
   * @return Reference to CSGUniverse
   */
  const CSGUniverse & getOuterUniverse() const;

  /**
   * @brief Get the outer material name if outer fype is CSG_MATERIAL
   *
   * @return name of the CSG material fill
   */
  const std::string & getOuterMaterial() const;

  /**
   * @brief Get the arrangement of CSGUniverses in the lattice
   *
   * @return list of list of universes in their lattice arrangement
   */
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> getUniverses() const
  {
    return _universe_map;
  }

  /**
   * @brief Get the arrangement of CSGUniverses in the lattice as their names
   *
   * @return list of list of universe names
   */
  const std::vector<std::vector<std::string>> getUniverseNameMap() const;

  /**
   * @brief whether or not the universe of the specified name exists in the lattice
   *
   * @param name of universe to search for
   *
   * @return true if universe of that name exists in lattice
   */
  bool hasUniverse(const std::string & name) const;

  /**
   * @brief Get attributes that define the lattice (excluding the universe map).
   *
   * @return map of string attribute name to value of that attribute
   */
  virtual std::unordered_map<std::string, JsonOutputUtils::AttributeVariant>
  getAttributes() const = 0; // pure virtual function

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
  const CSGUniverse & getUniverseAtIndex(const std::pair<int, int> index);

  /**
   * @brief get all locations in lattice where universe of the specified name exists
   *
   * @param univ_name name of universe
   * @return vector of locations (pairs of ints)
   */
  const std::vector<std::pair<unsigned int, unsigned int>>
  getUniverseIndices(const std::string & univ_name) const;

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

  /**
   * @brief reset the outer fill around the lattice elements to be VOID
   */
  void resetOuter();

  /**
   * @brief Get the list of unique universe objects in the lattice
   *
   * @return list of references to unique CSGUniverse objects
   */
  const std::vector<std::reference_wrapper<const CSGUniverse>> getUniqueUniverses();

  /// Operator overload for checking if two CSGLattice objects are equal
  bool operator==(const CSGLattice & other) const;

  /// Operator overload for checking if two CSGLattice objects are not equal
  bool operator!=(const CSGLattice & other) const;

protected:
  // set the name of the lattices - intentionally not public because
  // name needs to be managed at the CSGLatticeList level
  void setName(const std::string & name) { _name = name; }

  /// @brief assign the vectors of universes as the lattice elements
  /// @param universes
  virtual void
  setUniverses(std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes) = 0;

  /**
   * @brief replace the element at specified index in the lattice with the provided CSGUniverse.
   * This will check that the _universe_map has been initialized and that the index is valid.
   *
   * @param universe universe to add to the lattice at the location index
   * @param index location in lattice replace with provided universe
   */
  void setUniverseAtIndex(const CSGUniverse & universe, const std::pair<int, int> index);

  /// helper function to compare the attributes of the lattice type
  virtual bool compareAttributes(const CSGLattice & other) const = 0; // pure virtual

  /**
   * @brief Update the outer of the lattice to be the provided material name. This will change outer
   * type to CSG_MATERIAL even if it was a different type previously.
   *
   * @param outer_name name of CSG material that will fill space around lattice elements
   */
  void updateOuter(const std::string & outer_name);

  /**
   * @brief Update the outer of the lattice to be the provided universe. This will change outer type
   * to UNIVERSE even if it was a different type previously.
   *
   * @param outer_universe pointer to outer universe that will fill space around lattice elements
   */
  void updateOuter(const CSGUniverse & outer_universe);

  /// Name of lattice
  std::string _name;

  /// Type of lattice
  const std::string _lattice_type;

  /// Universes in the arrangement of how they appear in the lattice; dimensions depends on lattice type
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> _universe_map;

  /// An enum for type of outer fill for lattice
  mutable std::string _outer_type;

  /// name of the outer material
  mutable std::string _outer_material;

  /// outer object if fill is CSGUniverse
  const CSGUniverse * _outer_universe;

  // CSGLatticeList needs to be friend to access setName()
  friend class CSGLatticeList;
  // CSGBase needed for access updateAttributes()
  friend class CSGBase;

#ifdef MOOSE_UNIT_TEST
  /// Friends for unit testing
  ///@{
  FRIEND_TEST(CSGLatticeTest, testSetName);
  FRIEND_TEST(CSGLatticeTest, testUpdateOuter);
  FRIEND_TEST(CSGBaseTest, testAddLattice);
  ///@}
#endif
};
}
