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
#include "CSGVoidCell.h"

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
   * Destructor
   */
  virtual ~CSGUniverse() = default;

  std::shared_ptr<CSGCell> addMaterialCell(const std::string name, const std::string fill_name);

  std::shared_ptr<CSGCell> addVoidCell(const std::string name);

  std::shared_ptr<CSGCell> getCell(const std::string name);

  bool hasCell(const std::string name) const;

  const std::map<unsigned int, std::shared_ptr<CSGCell>> getAllCells() const { return _cells; }

protected:
  /// Name of surface
  std::string _name;

  /// Mapping of cell ids to pointers of cell objects that belong to universe
  std::map<unsigned int, std::shared_ptr<CSGCell>> _cells;

  /// Mapping of cell name to cell id
  std::map<std::string, unsigned int> _cell_name_id_mapping;

  /// Next available cell id
  unsigned int _next_cell_id;
};
} // namespace CSG
