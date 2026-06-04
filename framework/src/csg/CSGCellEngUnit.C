//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGCellEngUnit.h"
#include "CSGBase.h"
#include "CSGRegion.h"

namespace CSG
{

CSGCellEngUnit::CSGCellEngUnit(const std::string & name, const std::string & unit_type)
  : CSGCell(name, CSGRegion()), CSGEngUnit("CELL", unit_type)
{
}

const CSGCell &
CSGCellEngUnit::getExpandedCell() const
{
  // must contain exactly one cell to be considered a correct implementation and be expanded
  // properly
  const auto & cells = _internal_base->getRootUniverse().getAllCells();
  if (cells.size() == 0)
    mooseError("The root universe of CSGCellEngUnit ",
               getName(),
               " contains no cells. ",
               "Either getExpandedCell() was called before expandUnit() or expandUnit() is not "
               "implemented correctly");
  if (cells.size() > 1)
    mooseError("CSGCellEngUnit '",
               getName(),
               "' expandUnit() must create exactly one cell in _internal_base's root universe "
               "(found ",
               cells.size(),
               ").");
  return cells[0];
}

} // namespace CSG
