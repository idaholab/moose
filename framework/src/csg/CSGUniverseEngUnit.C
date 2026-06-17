//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGUniverseEngUnit.h"
#include "CSGBase.h"

namespace CSG
{

CSGUniverseEngUnit::CSGUniverseEngUnit(const std::string & name,
                                       const std::string & unit_type,
                                       bool is_root)
  : CSGUniverse(name, is_root), CSGEngUnit("UNIVERSE", unit_type)
{
}

const CSGUniverse &
CSGUniverseEngUnit::getExpandedUniverse() const
{
  // if root universe contains no cells, then it has not be expanded (or not implemented correctly)
  if (_internal_base->getRootUniverse().getAllCells().empty())
    mooseError("Root universe of ",
               getName(),
               " contains no cells. ",
               "Either getExpandedUniverse() has been called before expandUnit() or expandUnit() ",
               "is not implemented correctly.");

  // Check that all universes and cells created in _internal_base are reachable from the root.
  if (!_internal_base->areUniversesLinked())
    mooseError("CSGUniverseEngUnit '",
               getName(),
               "' of type '",
               getUnitType(),
               "' contains unlinked universes or cells. All objects created in expandUnit() must be"
               " reachable from the expanded root universe. Check implementation of expandUnit().");

  return _internal_base->getRootUniverse();
}

} // namespace CSG
