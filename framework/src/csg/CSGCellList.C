//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGCellList.h"

namespace CSG
{

CSGCellList::CSGCellList() {}

void
CSGCellList::checkCellName(const std::string name) const
{
  if (_cells.find(name) != _cells.end())
    mooseError("Cell with name " + name + " already exists in geoemetry.");
}

CSGCell &
CSGCellList::getCell(const std::string name) const
{
  if (_cells.find(name) == _cells.end())
    mooseError("No cell by name " + name + " exists in the geometry.");
  else
    return *(_cells.find(name)->second);
}

CSGCell &
CSGCellList::addVoidCell(const std::string name, const CSGRegion & region)
{
  checkCellName(name);
  _cells.insert(std::make_pair(name, std::make_unique<CSGCell>(name, region)));
  return *_cells[name];
}

CSGCell &
CSGCellList::addMaterialCell(const std::string name,
                             const std::string mat_name,
                             const CSGRegion & region)
{
  checkCellName(name);
  _cells.insert(std::make_pair(name, std::make_unique<CSGCell>(name, mat_name, region)));
  return *_cells[name];
}

CSGCell &
CSGCellList::addUniverseCell(const std::string name, CSGUniverse & univ, const CSGRegion & region)
{
  checkCellName(name);
  _cells.insert(std::make_pair(name, std::make_unique<CSGCell>(name, &univ, region)));
  return *_cells[name];
}

std::vector<CSGCell *>
CSGCellList::getAllCells() const
{
  std::vector<CSGCell *> cells;
  for (auto it = _cells.begin(); it != _cells.end(); ++it)
    cells.push_back(it->second.get());
  return cells;
}

void
CSGCellList::addCell(std::unique_ptr<CSGCell> & cell)
{
  auto name = cell->getName();
  checkCellName(name);
  _cells.insert(std::make_pair(name, std::move(cell)));
}

void
CSGCellList::renameCell(CSGCell & cell, const std::string name)
{
  // check that this cell passed in is actually in the same cell that is in the cell list
  auto prev_name = cell.getName();
  auto existing_cell = std::move(_cells.find(prev_name)->second);
  if (*existing_cell != cell)
    mooseError("Cell " + prev_name + " cannot be renamed to " + name +
               " as it does not exist in this CSGBase instance.");

  checkCellName(name);
  existing_cell->setName(name);
  _cells.erase(prev_name);
  _cells.insert(std::make_pair(name, std::move(existing_cell)));
}

} // namespace CSG
