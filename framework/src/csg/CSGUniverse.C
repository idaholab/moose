//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGUniverse.h"

namespace CSG
{

CSGUniverse::CSGUniverse(const std::string name, bool is_root) : _name(name), _is_root(is_root) {
}

CSGUniverse::CSGUniverse(const std::string name,
                         std::vector<std::shared_ptr<CSGCell>> cells,
                         bool is_root)
  : _name(name), _is_root(is_root)
{
  for (auto cell : cells)
    addCell(cell);
}

void
CSGUniverse::addCell(const std::shared_ptr<CSGCell> cell)
{
  auto cell_name = cell->getName();
  if (!hasCell(cell_name))
    _cells.push_back(cell);
  else
    mooseWarning("Universe " + getName() + " already contains a cell by name " + cell_name + ".");
}

std::shared_ptr<CSGCell>
CSGUniverse::getCell(const std::string name)
{
  if (!hasCell(name))
    mooseError("Cell with name " + name + " does not exist in universe " + _name + ".");
  for (auto cell : _cells)
  {
    if (cell->getName() == name)
      return cell;
  }
  return nullptr;
}

bool
CSGUniverse::hasCell(const std::string name) const
{
  for (auto cell : _cells)
  {
    if (cell->getName() == name)
      return true;
  }
  return false;
}

void
CSGUniverse::removeCell(const std::string name)
{
  if (!hasCell(name))
    mooseError("Cannot remove cell. Cell with name " + name + " does not exist in universe " +
               _name + ".");
  for (auto cell : _cells)
  {
    if (cell->getName() == name)
    {
      _cells.erase(std::remove(_cells.begin(), _cells.end(), cell), _cells.end());
      break;
    }
  }
}

void
CSGUniverse::removeCell(const std::shared_ptr<CSGCell> cell)
{
  auto name = cell->getName();
  if (!hasCell(name))
    mooseError("Cannot remove cell. Cell with name " + name + " does not exist in universe " +
               _name + ".");
  _cells.erase(std::remove(_cells.begin(), _cells.end(), cell), _cells.end());
}

} // namespace CSG
