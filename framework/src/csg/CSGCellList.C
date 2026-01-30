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

CSGCell &
CSGCellList::addCell(std::unique_ptr<CSGCell> cell)
{
  auto name = cell->getName();
  auto [it, inserted] = _cells.emplace(name, std::move(cell));
  if (!inserted)
    mooseError("Cell with name " + name + " already exists in geometry.");
  return *it->second;
}

CSGCell &
CSGCellList::getCell(const std::string & name) const
{
  if (_cells.find(name) == _cells.end())
    mooseError("No cell by name " + name + " exists in the geometry.");
  else
    return *(_cells.find(name)->second);
}

CSGCell &
CSGCellList::addVoidCell(const std::string & name, const CSGRegion & region)
{
  return addCell(std::make_unique<CSGCell>(name, region));
}

CSGCell &
CSGCellList::addMaterialCell(const std::string & name,
                             const std::string & mat_name,
                             const CSGRegion & region)
{
  return addCell(std::make_unique<CSGCell>(name, mat_name, region));
}

CSGCell &
CSGCellList::addUniverseCell(const std::string & name,
                             const CSGUniverse & univ,
                             const CSGRegion & region)
{
  return addCell(std::make_unique<CSGCell>(name, &univ, region));
}

CSGCell &
CSGCellList::addLatticeCell(const std::string & name,
                            const CSGLattice & lattice,
                            const CSGRegion & region)
{
  return addCell(std::make_unique<CSGCell>(name, &lattice, region));
}

std::vector<std::reference_wrapper<const CSGCell>>
CSGCellList::getAllCells() const
{
  std::vector<std::reference_wrapper<const CSGCell>> cells;
  for (auto it = _cells.begin(); it != _cells.end(); ++it)
    cells.push_back(*(it->second));
  return cells;
}

void
CSGCellList::renameCell(const CSGCell & cell, const std::string & name)
{
  // check that this cell passed in is actually in the same cell that is in the cell list
  auto prev_name = cell.getName();
  auto it = _cells.find(prev_name);
  if (it == _cells.end() || it->second.get() != &cell)
    mooseError("Cell " + prev_name + " cannot be renamed to " + name +
               " as it does not exist in this CSGBase instance.");

  auto existing_cell = std::move(_cells.find(prev_name)->second);
  existing_cell->setName(name);
  _cells.erase(prev_name);
  addCell(std::move(existing_cell));
}

bool
CSGCellList::operator==(const CSGCellList & other) const
{
  const auto all_cells = this->getAllCells();
  const auto other_cells = other.getAllCells();

  // Check that same number of cells are defined in both lists
  if (all_cells.size() != other_cells.size())
    return false;

  // Iterate through each CSGCell in list and check equality of each cell
  // with other list
  for (const auto & cell : all_cells)
  {
    const auto & cell_name = cell.get().getName();
    if (!other.hasCell(cell_name))
      return false;
    const auto & other_cell = other.getCell(cell_name);
    if (cell.get() != other_cell)
      return false;
  }
  return true;
}

bool
CSGCellList::operator!=(const CSGCellList & other) const
{
  return !(*this == other);
}

} // namespace CSG
