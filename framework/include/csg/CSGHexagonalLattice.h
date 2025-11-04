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
 * CSGHexagonalLattice is the class for constructing hexagonal lattices of CSGUniverses
 * arranged in concentric hexagonal rings
 */
class CSGHexagonalLattice : public CSGLattice
{
public:
  /**
   * Construct a new CSGHexagonalLattice from a map of universes. Universes should be arranged
   * by rows and correspond to a hexagonal lattice with x-orientation.
   *
   * @param name unique identifying name of lattice
   * @param pitch flat-to-flat distance for one hexagonal lattice element
   * @param universes row-wise ragged vector of universes representing hex pattern
   */
  CSGHexagonalLattice(
      const std::string & name,
      Real pitch,
      std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes);

  /**
   * Construct a new empty CSGHexagonalLattice with the specified pitch.
   * NOTE: must call setLatticeUniverses or addUniverseToLattice to populate.
   *
   * @param name unique identifying name of lattice
   * @param pitch flat-to-flat distance for one hexagonal lattice element
   */
  CSGHexagonalLattice(const std::string & name, Real pitch);

  /**
   * Destructor
   */
  virtual ~CSGHexagonalLattice() = default;

  /**
   * @brief Get the map of data that defines the geometric dimensions of the lattice:
   *  - nrow: number of rows in the hex lattice (int)
   *  - pitch: pitch of the lattice element (Real)
   *
   * @return std::unordered_map<std::string, std::any>
   */
  virtual std::unordered_map<std::string, std::any> getDimensions() const override;

  /**
   * @brief update the specified lattice geometric dimension to the specified value.
   * Valid dimensions are: pitch (Real) and nrow (int). If the universe map has already been set on
   * the lattice, nrow cannot be updated.
   *
   * @param dim_name
   * @param dim_value
   */
  virtual void updateDimension(const std::string & dim_name, std::any dim_value) override;

  /**
   * @brief check if provided index in row-element form is valid for the given hexagonal lattice
   *
   * @param index in row-element form
   * @return true if valid, otherwise false
   */
  virtual bool isValidIndex(const std::pair<int, int> index) const override;

  /**
   * @brief check if the arrangement of the provided universes is valid for the hexagonal lattice
   * given the number or rows/rings defined for the lattice. Universes should be listed by row,
   * starting from the top, and assume an x-orientation arrangment.
   *
   * @param universes list of lists of universes that define the arrangement of the lattice
   * @return true if valid, otherwise false
   */
  virtual bool isValidUniverseMap(
      std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes) const override;

  /**
   * @brief Get given an index in row-element form, get the corresponding ring-element index. The
   * row-element form assumes the top row is the 0th row and is indexed from left to right. The
   * ring-element form assumes the outermost ring is the 0th ring and is indexed starting from the
   * rightmost element of the ring and proceeding counter-clockwise around the ring.
   *
   * Example of corresponding indices for a hexagonal lattice with 3 rings (5 rows):
   *
   * Row-Element Form:
   *        (0,0)  (0,1)  (0,2)
   *     (1,0)  (1,1)  (1,2)  (1,3)
   * (2,0)  (2,1)  (2,2)  (2,3)  (2,4)
   *     (3,0)  (3,1)  (3,2)  (3,3)
   *        (4,0)  (4,1)  (4,2)
   *
   * Ring-Element Form:
   *       (0,8)  (0,9)  (0,10)
   *    (0,7)  (1,4)  (1,5)  (0,11)
   * (0,6)  (1,3)  (2,0)  (1,0)  (0,0)
   *    (0,5)  (1,2)  (1,1)  (0,1)
   *       (0,4)  (0,3)  (0,2)
   *
   * @param index in row-element form
   * @return index in ring-element form
   */
  std::pair<int, int> getRingIndexFromRowIndex(std::pair<int, int> index) const;

  /**
   * @brief Get given an index in ring-element form, get the corresponding row-element index. The
   * ring-element form assumes the outermost ring is the 0th ring and is indexed starting from the
   * rightmost element of the ring and proceeding counter-clockwise around the ring. The row-element
   * form assumes the top row is the 0th row and is indexed from left to right.
   *
   * Example of corresponding indices for a hexagonal lattice with 3 rings (5 rows):
   *
   * Row-Element Form:
   *        (0,0)  (0,1)  (0,2)
   *     (1,0)  (1,1)  (1,2)  (1,3)
   * (2,0)  (2,1)  (2,2)  (2,3)  (2,4)
   *     (3,0)  (3,1)  (3,2)  (3,3)
   *        (4,0)  (4,1)  (4,2)
   *
   * Ring-Element Form:
   *       (0,8)  (0,9)  (0,10)
   *    (0,7)  (1,4)  (1,5)  (0,11)
   * (0,6)  (1,3)  (2,0)  (1,0)  (0,0)
   *    (0,5)  (1,2)  (1,1)  (0,1)
   *       (0,4)  (0,3)  (0,2)
   *
   * @param index in ring-element form
   * @return index in row-element form
   */
  std::pair<int, int> getRowIndexFromRingIndex(std::pair<int, int> index) const;

  /**
   * @brief get number of rows in the lattices
   *
   * @return number of rows
   */
  int getNRows() const { return _nrow; }

  /**
   * @brief get number of rings in the lattice
   *
   * @return number of rings
   */
  int getNRings() const;

  /**
   * @brief get the pitch of the lattice
   *
   * @return pitch
   */
  Real getPitch() const { return _pitch; }

protected:
  /// compare the dimensions _nrow and _pitch of this lattice to another lattice
  virtual bool compareDimensions(const CSGLattice & other) const override;

  virtual void setUniverses(
      std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes) override;

  /// lattice pitch (flat-to-flat distance between adjacent hex elements)
  Real _pitch;

  /// number of rows in the hexagonal lattice (must be odd), should be consistent with the number of rings
  int _nrow;

#ifdef MOOSE_UNIT_TEST
  /// Friends for unit testing
  ///@{
  FRIEND_TEST(CSGLatticeTest, testHexSetUniverses);
///@}
#endif
};

/// methods to help convert between number of rows and rings
// get the total number of rings from the number of rows
int nRowToRing(int nrow);
// get the total number of rows from the number of rings
int nRingToRow(int nring);

} // namespace CSG