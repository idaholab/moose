//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CSGCellList.h"
#include "CSGTransformation.h"

#ifdef MOOSE_UNIT_TEST
#include "gtest/gtest.h"
#endif

namespace CSG
{

/**
 * CSGUniverse creates an internal representation of a Constructive Solid Geometry (CSG)
 * universe, which represents a collection of cells that can be defined repeatedly within a separate
 * container of cells
 */
class CSGUniverse
{
public:
  /**
   * @brief Construct a new CSGUniverse object
   *
   * @param name unique name of universe
   * @param is_root true to set universe as the root universe (default false)
   */
  CSGUniverse(const std::string & name, bool is_root = false);

  /**
   * @brief Construct a new CSGUniverse object from list of cells
   *
   * @param name unique name of universe
   * @param cells list of cells to add to universe
   * @param is_root true to set universe as the root universe (default false)
   */
  CSGUniverse(const std::string & name, std::vector<CSGCell *> & cells, bool is_root = false);

  /**
   * Destructor
   */
  virtual ~CSGUniverse() = default;

  /**
   * @brief Get the CSGCell object by name
   *
   * @param name name of cell
   * @return reference to the cell of the specified name in this universe
   */
  const CSGCell & getCell(const std::string & name);

  /**
   * @brief check if cell of provided name is present in universe
   *
   * @param name name of cell
   * @return true if cell of name is in universe, otherwise false
   */
  bool hasCell(const std::string & name) const;

  /**
   * @brief Get list of the all cells in the universe
   *
   * @return list of pointers to cells in universe
   */
  const std::vector<std::reference_wrapper<const CSGCell>> & getAllCells() const { return _cells; }

  /**
   * @brief Get the name of the universe
   *
   * @return name of universe
   */
  const std::string & getName() const { return _name; }

  /**
   * @brief return true if the universe is the root universe
   *
   * @return true / false
   */
  bool isRoot() const { return _is_root; }

  /**
   * @brief Get the list of transformations applied to this universe
   *
   * @return const reference to the list of transformations
   */
  const std::vector<std::pair<TransformationType, std::vector<Real>>> & getTransformations() const
  {
    return _transformations;
  }

  /// Operator overload for checking if two CSGUniverse objects are equal
  bool operator==(const CSGUniverse & other) const;

  /// Operator overload for checking if two CSGUniverse objects are not equal
  bool operator!=(const CSGUniverse & other) const;

protected:
  /**
   * @brief add cell to universe
   *
   * @param reference to cell to add
   */
  void addCell(const CSGCell & cell);

  /**
   * @brief remove a cell of the specified name from the universe
   *
   * @param name name of cell to remove
   */
  void removeCell(const std::string & name);

  /**
   * @brief remove all cells from the universe
   */
  void removeAllCells() { _cells.clear(); }

  // set the name of the universe - intentionally not public because
  // name needs to be managed at the CSGUniverseList level
  void setName(const std::string & name) { _name = name; }

  /// Apply a transformation to the universe (accessed through CSGBase)
  void applyTransformation(TransformationType type, const std::vector<Real> & values);

  /// Name of universe
  std::string _name;

  /// list of references to cells in universe
  std::vector<std::reference_wrapper<const CSGCell>> _cells;

  /// whether or not this universe is the root universe
  bool _is_root;

  /// list of transformations applied to the universe (type, value) in the order they are applied
  std::vector<std::pair<TransformationType, std::vector<Real>>> _transformations;

  // CSGUniverseList needs to be friend to access setName()
  friend class CSGUniverseList;

  // CSGUniverseList needs to be friend to access addCell()
  friend class CSGBase;

#ifdef MOOSE_UNIT_TEST
  /// Friends for unit testing
  ///@{
  FRIEND_TEST(CSGUniverseTest, testGetCell);
  FRIEND_TEST(CSGUniverseTest, testAddCell);
  FRIEND_TEST(CSGUniverseTest, testRemoveCell);
  FRIEND_TEST(CSGUniverseTest, testRemoveAllCells);
  FRIEND_TEST(CSGUniverseTest, testSetName);
  ///@}
#endif
};
} // namespace CSG
