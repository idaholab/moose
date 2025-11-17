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

#ifdef MOOSE_UNIT_TEST
#include "gtest/gtest.h"
#endif

namespace CSG
{

/**
 * CSGCartesianLattice is the class for constructing regular Cartesian lattices of CSGUniverses.
 */
class CSGCartesianLattice : public CSGLattice
{
public:
  /**
   * @brief Construct a new CSGCartesianLattice object from the map of universes provided
   *
   * @param name unique identifying name of lattice
   * @param pitch pitch of lattice elements
   * @param universes list of list of universes to set as the lattice map
   */
  CSGCartesianLattice(
      const std::string & name,
      const Real pitch,
      std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes);

  /**
   * @brief Construct a new empty CSGCartesianLattice object with specified pitch.
   * NOTE: must call setLatticeUniverses to populate the universe map.
   *
   * @param name unique identifying name of lattice
   * @param pitch pitch of lattice elements
   */
  CSGCartesianLattice(const std::string & name, const Real pitch);

  /**
   * Destructor
   */
  virtual ~CSGCartesianLattice() = default;

  /**
   * @brief clone this Cartesian lattice
   *
   * @return std::unique_ptr<CSGLattice> unique pointer to cloned Cartesian lattice
   */
  std::unique_ptr<CSGLattice> clone() const override
  {
    return std::make_unique<CSGCartesianLattice>(*this);
  }

  /**
   * @brief Get attributes that define the lattice (excluding the universe map).
   *  - nrow: number of rows (int)
   *  - ncol: number of columns (int)
   *  - pitch: pitch of the lattice element (Real)
   *
   * @return map of string dimension name to value of that dimension
   */
  virtual std::unordered_map<std::string, std::any> getAttributes() const override
  {
    return {{"nrow", _nrow}, {"ncol", _ncol}, {"pitch", _pitch}};
  }

  /**
   * @brief Checks if the given index location (row, column) is a valid index for the
   * lattice. Allowable indices are: 0 <= row < _nrow and 0 <= column < _ncol.
   *
   * @param index location in (row, column) form
   * @return true if index is valid for the lattice
   */
  virtual bool isValidIndex(const std::pair<unsigned int, unsigned int> index) const override;

  /**
   * @brief check that any provided list of list of CSGUniverses are the correct dimensions. Must
   * have number of lists within universes equal to _nrow. And each sublist must be size _ncol.
   *
   * @param universes list of list of universes to be used to define the lattice structure
   * @return true if universe dimensions are valid
   */
  virtual bool isValidUniverseMap(
      std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes) const override;

  /**
   * @brief get the number of rows
   *
   * @return number of rows
   */
  int getNRows() const { return _nrow; }

  /**
   * @brief get number of columns
   *
   * @return number of columns
   */
  int getNCols() const { return _ncol; }

  /**
   * @brief get lattice pitch
   *
   * @return pitch
   */
  Real getPitch() const { return _pitch; }

  /**
   * @brief set the pitch of the lattice
   *
   * @param pitch new pitch value
   */
  void setPitch(Real pitch);

protected:
  /// compare the attributes returned in getAttributes of this lattice to another lattice
  virtual bool compareAttributes(const CSGLattice & other) const override;

  /**
   * @brief set the universes that define the lattice layout
   *
   * @param universes list of list of universes to set as the lattice map
   */
  virtual void setUniverses(
      std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes) override;

  /// pitch
  Real _pitch;

  /// number of elements in the first dimension (rows)
  int _nrow;

  /// number of elements in the second direction (columns)
  int _ncol;

  friend class CSGBase;

#ifdef MOOSE_UNIT_TEST
  /// Friends for unit testing
  ///@{
  FRIEND_TEST(CSGLatticeTest, testCartSetUniverses);
  FRIEND_TEST(CSGLatticeTest, testCartSetUniverseAtIndex);
  ///@}
#endif
};
}
