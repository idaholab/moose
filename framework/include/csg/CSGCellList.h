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
   * @param mat_name material name (TODO: this will eventually be a material object and not just a
   * name)
   * @param region cell region
   * @return reference to CSGCell with material fill that was created and
   * added to this CSGCellList
   */
  CSGCell &
  addMaterialCell(const std::string name, const std::string mat_name, const CSGRegion & region);

  /**
   * @brief Add a Void Cell object cell list
   *
   * @param name unique cell name
   * @param region cell region
   * @return reference to CSGCell with void fill that was created and
   * added to this CSGCellList
   */
  CSGCell & addVoidCell(const std::string name, const CSGRegion & region);

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
  addUniverseCell(const std::string name, const CSGUniverse & univ, const CSGRegion & region);

  /**
   * @brief Get map of all names to cells in cell list
   *
   * @return map of all names to CSGCell pointers
   */
  std::map<std::string, std::unique_ptr<CSGCell>> & getCellListMap() { return _cells; }

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
  CSGCell & getCell(const std::string name) const;

  /**
   * @brief add a cell to the CellList. Ownership of cell will be trasnferred to cell list object
   * that calls this function
   *
   * @param cell cell to add to the CellList.
   */
  void addCell(std::unique_ptr<CSGCell> & cell);

  /**
   * @brief rename the specified cell
   *
   * @param cell reference to CSGCell object that should be renamed
   * @param name new name
   */
  void renameCell(const CSGCell & cell, const std::string name);

  /// Checks whether cell name already exists within CSGCellList object
  void checkCellName(const std::string name) const;

  /// Mapping of cell names to pointers of stored cell objects
  std::map<std::string, std::unique_ptr<CSGCell>> _cells;

  // Only CSGBase should be calling the methods in CSGCellList
  friend class CSGBase;
};
} // namespace CSG
