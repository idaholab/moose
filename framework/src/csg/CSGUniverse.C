//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGUniverse.h"
#include "CSGUtils.h"
#include "MooseError.h"

namespace CSG
{

CSGUniverse::CSGUniverse(const std::string & name, bool is_root) : _name(name), _is_root(is_root) {}

CSGUniverse::CSGUniverse(const std::string & name, std::vector<CSGCell *> & cells, bool is_root)
  : _name(name), _is_root(is_root)
{
  CSGUtils::checkValidCSGName(name);
  for (auto cell : cells)
    addCell(*cell);
}

void
CSGUniverse::addCell(const CSGCell & cell)
{
  auto cell_name = cell.getName();
  if (!hasCell(cell_name))
    _cells.push_back(cell);
  else
    mooseWarning("Universe " + getName() + " already contains a cell by name " + cell_name + ". " +
                 "Skipping cell insertion for cell with duplicate name.");
}

const CSGCell &
CSGUniverse::getCell(const std::string & name)
{
  if (!hasCell(name))
    mooseError("Cell with name " + name + " does not exist in universe " + _name + ".");
  for (const CSGCell & cell : _cells)
    if (cell.getName() == name)
      return cell;
  mooseError("Should not reach here.");
}

bool
CSGUniverse::hasCell(const std::string & name) const
{
  for (const CSGCell & cell : _cells)
    if (cell.getName() == name)
      return true;
  return false;
}

void
CSGUniverse::removeCell(const std::string & name)
{
  if (!hasCell(name))
    mooseError("Cannot remove cell. Cell with name " + name + " does not exist in universe " +
               _name + ".");
  for (auto it = _cells.begin(); it != _cells.end(); ++it)
    if (it->get().getName() == name)
    {
      _cells.erase(it);
      break;
    }
}

bool
CSGUniverse::operator==(const CSGUniverse & other) const
{
  const bool names_eq = this->getName() == other.getName();
  if (names_eq)
  {
    const auto & all_cells = getAllCells();
    const auto & other_cells = other.getAllCells();
    const bool num_cells_eq = all_cells.size() == other_cells.size();
    if (num_cells_eq)
    {
      for (unsigned int i = 0; i < all_cells.size(); ++i)
        if (all_cells[i].get() != other_cells[i].get())
          return false;
      return true;
    }
    else
      return false;
  }
  else
    return false;
}

bool
CSGUniverse::operator!=(const CSGUniverse & other) const
{
  return !(*this == other);
}

void
CSGUniverse::applyTransformation(TransformationType type,
                                 const std::tuple<Real, Real, Real> & values)
{
  // Assert valid input as a safety measure
  // Main validation is done in CSGBase::applyTransformation
  mooseAssert(isValidTransformationValue(type, values),
              "Invalid transformation values for transformation type " +
                  getTransformationTypeString(type) + " on universe " + getName());
  _transformations.emplace_back(type, values);
}

} // namespace CSG
