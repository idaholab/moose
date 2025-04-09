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
   * Default constructor
   */
  CSGUniverse(const std::string name);

  /**
   * @brief Construct a new CSGUniverse object from list of cells
   *
   * @param name unique name of universe
   * @param cells list of cells to add to universe
   */
  CSGUniverse(const std::string name, std::vector<std::shared_ptr<CSGCell>> cells);

  /**
   * Destructor
   */
  virtual ~CSGUniverse() = default;

  /**
   * @brief add cell to universe
   *
   * @param cell
   */
  void addCell(const std::shared_ptr<CSGCell> cell);

  /**
   * @brief Get the Cell object by name
   *
   * @param name name of cell
   * @return std::shared_ptr<CSGCell>
   */
  std::shared_ptr<CSGCell> getCell(const std::string name);

  /**
   * @brief check if cell of provided name is present in universe
   *
   * @param name name of cell
   * @return true / false
   */
  bool hasCell(const std::string name) const;

  /**
   * @brief remove a cell from the universe by its name
   *
   * @param name name of cell to remove
   */
  void removeCell(const std::string name);

  /**
   * @brief remove a cell from the universe
   *
   * @param cell cell to remove
   */
  void removeCell(const std::shared_ptr<CSGCell> cell);

  /**
   * @brief Get list of the all cells in the universe
   *
   * @return std::vector<std::shared_ptr<CSGCell>>
   */
  std::vector<std::shared_ptr<CSGCell>> getAllCells() const { return _cells; }

  /**
   * @brief Get the name of the universe
   *
   * @return const std::string
   */
  const std::string getName() const { return _name; }

protected:
  /// Name of universe
  std::string _name;

  /// list of cells in universe
  std::vector<std::shared_ptr<CSGCell>> _cells;
};
} // namespace CSG
