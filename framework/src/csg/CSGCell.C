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

CSGCell::CSGCell(const std::string name, const CSGRegion & region)
  : _name(name), _fill_type(FillType::VOID), _region(region)
{
}

CSGCell::CSGCell(const std::string name, const std::string mat_name, const CSGRegion & region)
  : _name(name), _fill_type(FillType::MATERIAL), _fill_name(mat_name), _region(region)
{
}

CSGCell::CSGCell(const std::string name, const CSGUniverse * univ, const CSGRegion & region)
  : _name(name),
    _fill_type(FillType::UNIVERSE),
    _fill_name(univ->getName()),
    _region(region),
    _fill_universe(univ)
{
}

const CSGUniverse &
CSGCell::getFillUniverse() const
{
  if (getFillType() != FillType::UNIVERSE)
  {
    mooseError("Cell '" + getName() + "' has " + getFillTypeString() + " fill, not UNIVERSE.");
  }
  else
    return *_fill_universe;
}

const std::string
CSGCell::getFillMaterial() const
{
  if (getFillType() != FillType::MATERIAL)
  {
    mooseError("Cell '" + getName() + "' has " + getFillTypeString() + " fill, not MATERIAL.");
  }
  else
    return _fill_name;
}

const std::string
CSGCell::getFillTypeString() const
{
  switch (_fill_type)
  {
    case FillType::MATERIAL:
      return "MATERIAL";
    case FillType::UNIVERSE:
      return "UNIVERSE";
    default:
      return "VOID";
  }
}

bool
CSGCell::operator==(const CSG::CSGCell & other) const
{
  const auto name_eq = this->getName() == other.getName();
  const auto region_eq = this->getRegion() == other.getRegion();
  const auto fill_type_eq = (this->getFillTypeString() == other.getFillTypeString()) &&
                            (this->getFillName() == other.getFillName());
  if (name_eq && region_eq && fill_type_eq)
  {
    switch (this->getFillType())
    {
      case FillType::MATERIAL:
        return this->getFillMaterial() == other.getFillMaterial();
      case FillType::UNIVERSE:
        return this->getFillUniverse() == other.getFillUniverse();
      default:
        return true;
    }
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
