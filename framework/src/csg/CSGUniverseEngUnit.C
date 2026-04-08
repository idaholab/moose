//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGUniverseEngUnit.h"

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
  if (!_expanded_universe)
    mooseError("getExpandedUniverse() cannot be called on CSGUniverseEngUnit '",
               getName(),
               "' before expandUnit() has been called.");
  return *_expanded_universe;
}

} // namespace CSG
