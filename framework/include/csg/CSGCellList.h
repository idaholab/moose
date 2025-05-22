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
 * CSGCellList creates a container for CSGCell objects to pass to CSGMesh
 */
class CSGCellList
{
public:
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
   * @return std::shared_ptr<CSGCell> pointer to CSGCell with material fill that was created and
   * added to this CSGCellList
   */
  std::shared_ptr<CSGCell>
  addMaterialCell(const std::string name, const std::string mat_name, const CSGRegion & region);

  /**
   * @brief Add a Void Cell object cell list
   *
   * @param name unique cell name
   * @param region cell region
   * @return std::shared_ptr<CSGCell>pointer to CSGCell with void fill that was created and
   * added to this CSGCellList
   */
  std::shared_ptr<CSGCell> addVoidCell(const std::string name, const CSGRegion & region);

  /**
   * @brief Add a Universe Cell object to cell list
   *
   * @param name unique cell name
   * @param univ universe
   * @param region cell region
   * @return std::shared_ptr<CSGCell> pointer to CSGCell with universe fill that was created and
   * added to this CSGCellList
   */
  std::shared_ptr<CSGCell> addUniverseCell(const std::string name,
                                           const std::shared_ptr<CSGUniverse> univ,
                                           const CSGRegion & region);

  /**
   * @brief Get the all cells in CSGBase instance
   *
   * @return const std::map<std::string, std::shared_ptr<CSGCell>>& map of all cells to CSGCell
   * objects
   */
  const std::map<std::string, std::shared_ptr<CSGCell>> & getAllCells() const { return _cells; }

  /**
   * @brief Get the CSGCell by name
   *
   * @param name
   * @return const std::shared_ptr<CSGCell>& pointer CSGCell of the specified name
   */
  const std::shared_ptr<CSGCell> & getCell(const std::string name);

  /**
   * @brief add a cell to the CellList
   *
   * @param cell cell to add to the CellList
   */
  void addCell(const std::shared_ptr<CSGCell> cell);

  /**
   * @brief rename the specified cell
   *
   * @param name new name
   */
  void renameCell(const std::shared_ptr<CSGCell> cell, const std::string name);

protected:
  /// Checks whether cell name already exists within CSGCellList object
  void checkCellName(const std::string name);

  /// Mapping of cell names to pointers of stored cell objects
  std::map<std::string, std::shared_ptr<CSGCell>> _cells;
};
} // namespace CSG
