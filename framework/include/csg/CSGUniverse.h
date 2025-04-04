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

  CSGUniverse(const std::string name, std::vector<std::shared_ptr<CSGCell>> cells);

  /**
   * Destructor
   */
  virtual ~CSGUniverse() = default;

  void addCell(const std::shared_ptr<CSGCell> cell);

  std::shared_ptr<CSGCell> getCell(const std::string name);

  bool hasCell(const std::string name) const;

  void removeCell(const std::string name);

  void removeCell(const std::shared_ptr<CSGCell> cell);

  std::vector<std::shared_ptr<CSGCell>> getAllCells() const { return _cells; }

  const std::string getName() const { return _name; }

protected:
  /// Name of surface
  std::string _name;

  /// list of cells in universe
  std::vector<std::shared_ptr<CSGCell>> _cells;
};
} // namespace CSG
