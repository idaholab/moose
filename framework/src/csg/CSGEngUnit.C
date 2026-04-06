//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGEngUnit.h"

namespace CSG
{

CSGEngUnit::CSGEngUnit(const std::string & behavior, const std::string & unit_type)
  : _unit_type(unit_type)
{
  _behavior = behavior;
}

bool
CSGEngUnit::operator==(const CSGEngUnit & other) const
{
  if (getName() != other.getName())
    return false;
  if (getBehavior() != other.getBehavior())
    return false;
  if (getUnitType() != other.getUnitType())
    return false;
  if (getAttributes() != other.getAttributes())
    return false;
  if (getTransformations() != other.getTransformations())
    return false;
  return true;
}

bool
CSGEngUnit::operator!=(const CSGEngUnit & other) const
{
  return !(*this == other);
}

} // namespace CSG
