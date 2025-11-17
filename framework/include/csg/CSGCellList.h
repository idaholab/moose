//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CSGCell.h"

namespace CSG
{

/**
 * CSGCellList creates a container for CSGCell objects to pass to CSGBase object
 */
class CSGCellList
{
protected:
  /**
   * Default constructor
   */
  CSGCellList();

  /**
   * Destructor
   */
  virtual ~CSGCellList() = default;

  /**
   * @brief Add a Material Cell object to cell list
   *
   * @param name unique cell name
   * @param mat_name material name
   * @param region cell region
   * @return reference to CSGCell with material fill that was created and
   * added to this CSGCellList
   */
  CSGCell &
  addMaterialCell(const std::string & name, const std::string & mat_name, const CSGRegion & region);

  /**
   * @brief Add a Void Cell object cell list
   *
   * @param name unique cell name
   * @param region cell region
   * @return reference to CSGCell with void fill that was created and
   * added to this CSGCellList
   */
  CSGCell & addVoidCell(const std::string & name, const CSGRegion & region);

  /**
   * @brief Add a Universe Cell object to cell list
   *
   * @param name unique cell name
   * @param univ universe
   * @param region cell region
   * @return reference to CSGCell with universe fill that was created and
   * added to this CSGCellList
   */
  CSGCell &
  addUniverseCell(const std::string & name, const CSGUniverse & univ, const CSGRegion & region);

  /**
   * @brief Get non-const map of all names to cells in cell list
   * @brief Add a Lattice Cell object to cell list
   *
   * @param name unique cell name
   * @param lattice lattice
   * @param region cell region
   * @return reference to CSGCell with lattice fill that was created and
   * added to this CSGCellList
   */
  CSGCell &
  addLatticeCell(const std::string & name, const CSGLattice & lattice, const CSGRegion & region);

  /**
   * @brief return whether cell with given name exists in cell list
   *
   * @param name name of cell
   * @return true if cell name exists, false otherwise
   */
  bool hasCell(const std::string & name) const { return _cells.find(name) != _cells.end(); }

  /**
   * @brief Get non-const map of all names to cells in cell list
   *
   * @return map of all names to CSGCell pointers
   */
  std::unordered_map<std::string, std::unique_ptr<CSGCell>> & getCellListMap() { return _cells; }

  /**
   * @brief Get const map of all names to cells in cell list
   *
   * @return map of all names to CSGCell pointers
   */
  const std::unordered_map<std::string, std::unique_ptr<CSGCell>> & getCellListMap() const
  {
    return _cells;
  }

  /**
   * @brief Get all the cells in CSGBase instance
   *
   * @return list of references to all CSGCell objects
   */
  std::vector<std::reference_wrapper<const CSGCell>> getAllCells() const;

  /**
   * @brief Get the CSGCell by name
   *
   * @param name
   * @return reference to CSGCell of the specified name
   */
  CSGCell & getCell(const std::string & name) const;

  /**
   * @brief add a cell to the CellList. Ownership of cell will be transferred to cell list object
   * that calls this function
   *
   * @param cell cell to add to the CellList.
   * @return reference to CSGCell that was added to CellList
   */
  CSGCell & addCell(std::unique_ptr<CSGCell> cell);

  /**
   * @brief rename the specified cell
   *
   * @param cell reference to CSGCell object that should be renamed
   * @param name new name
   */
  void renameCell(const CSGCell & cell, const std::string & name);

  /// Operator overload for checking if two CSGCellList objects are equal
  bool operator==(const CSGCellList & other) const;

  /// Operator overload for checking if two CSGCellList objects are not equal
  bool operator!=(const CSGCellList & other) const;

  /// Mapping of cell names to pointers of stored cell objects
  std::unordered_map<std::string, std::unique_ptr<CSGCell>> _cells;

  // Only CSGBase should be calling the methods in CSGCellList
  friend class CSGBase;
};
} // namespace CSG
