//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGCell.h"
#include "CSGUniverse.h"

namespace CSG
{

CSGCell::CSGCell(const std::string & name, const CSGRegion & region) : _name(name), _region(region)
{
  _fill_type = "VOID";
}

CSGCell::CSGCell(const std::string & name, const std::string & mat_name, const CSGRegion & region)
  : _name(name), _fill_name(mat_name), _region(region)
{
  _fill_type = "CSG_MATERIAL";
}

CSGCell::CSGCell(const std::string & name, const CSGUniverse * univ, const CSGRegion & region)
  : _name(name), _fill_name(univ->getName()), _region(region), _fill_universe(univ)
{
  _fill_type = "UNIVERSE";
}

const CSGUniverse &
CSGCell::getFillUniverse() const
{
  if (getFillType() != "UNIVERSE")
    mooseError("Cell '" + getName() + "' has " + getFillType() + " fill, not UNIVERSE.");
  else
    return *_fill_universe;
}

const std::string &
CSGCell::getFillMaterial() const
{
  if (getFillType() != "CSG_MATERIAL")
    mooseError("Cell '" + getName() + "' has " + getFillType() + " fill, not CSG_MATERIAL.");
  else
    return _fill_name;
}

bool
CSGCell::operator==(const CSG::CSGCell & other) const
{
  const auto name_eq = this->getName() == other.getName();
  const auto region_eq = this->getRegion() == other.getRegion();
  const auto fill_type_eq =
      (this->getFillType() == other.getFillType()) && (this->getFillName() == other.getFillName());
  if (name_eq && region_eq && fill_type_eq)
  {
    if (this->getFillType() == "CSG_MATERIAL")
      return this->getFillMaterial() == other.getFillMaterial();
    else if (this->getFillType() == "UNIVERSE")
      return this->getFillUniverse() == other.getFillUniverse();
    else
      return true;
  }
  else
    return false;
}

bool
CSGCell::operator!=(const CSG::CSGCell & other) const
{
  return !(*this == other);
}

} // namespace CSG
