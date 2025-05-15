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
CSGCellList::checkCellName(const std::string name)
{
  if (_cells.find(name) != _cells.end())
    mooseError("Cell with name " + name + " already exists in geoemetry.");
}

const std::shared_ptr<CSGCell> &
CSGCellList::getCell(const std::string name)
{
  auto cell = _cells.find(name);
  if (cell == _cells.end())
    mooseError("No cell by name " + name + " exists in the geometry.");
  else
    return cell->second;
}

std::shared_ptr<CSGCell>
CSGCellList::addVoidCell(const std::string name, const CSGRegion & region)
{
  checkCellName(name);
  _cells.insert(
      std::make_pair(name, std::make_shared<CSGCell>(name, CSGCell::FillType::VOID, region)));
  return _cells[name];
}

std::shared_ptr<CSGCell>
CSGCellList::addMaterialCell(const std::string name,
                             const std::string mat_name,
                             const CSGRegion & region)
{
  checkCellName(name);
  _cells.insert(std::make_pair(name, std::make_shared<CSGCell>(name, mat_name, region)));
  return _cells[name];
}

std::shared_ptr<CSGCell>
CSGCellList::addUniverseCell(const std::string name,
                             const std::shared_ptr<CSGUniverse> univ,
                             const CSGRegion & region)
{
  checkCellName(name);
  _cells.insert(std::make_pair(name, std::make_shared<CSGCell>(name, univ, region)));
  return _cells[name];
}

void
CSGCellList::addCell(const std::pair<std::string, std::shared_ptr<CSGCell>> cell)
{
  checkCellName(cell.first);
  _cells.insert(cell);
}

void
CSGCellList::renameCell(const std::shared_ptr<CSGCell> cell, const std::string name)
{
  checkCellName(name);
  auto prev_name = cell->getName();
  cell->setName(name);
  _cells.erase(prev_name);
  _cells.insert(std::make_pair(name, cell));
}

} // namespace CSG
