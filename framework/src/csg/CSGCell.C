//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGCell.h"

namespace CSG
{

CSGCell::CSGCell(const std::string name, const FillType fill_type, const CSGRegion & region)
  : _name(name), _fill_type(fill_type), _region(region)
{
}

CSGCell::CSGCell(const std::string name, const CSGRegion & region)
  : _name(name), _fill_type(FillType::VOID), _region(region)
{
}

CSGCell::CSGCell(const std::string name, const std::string mat_name, const CSGRegion & region)
  : _name(name),
    _fill_type(FillType::MATERIAL),
    _fill_name(mat_name),
    _region(region)
{
}

CSGCell::CSGCell(const std::string name, const CSGUniverse & univ, const CSGRegion & region)
  : _name(name), _fill_type(FillType::UNIVERSE), _fill_name(univ.getName()), _region(region)
{
  _fill_universe = &univ;
}

template <typename T>
T
CSGCell::getFill()
{
  if (getFillType() == FillType::VOID)
    return nullptr;
  if (getFillType() == FillType::MATERIAL)
    return static_cast<std::string>(_fill_name); // TODO: return material object when available
  if (getFillType() == FillType::UNIVERSE)
    return static_cast<const CSGUniverse *>(_fill_universe);
}

const std::string
CSGCell::getFillTypeString()
{
  switch (_fill_type)
  {
    case FillType::VOID:
      return "VOID";
    case FillType::MATERIAL:
      return "MATERIAL";
    case FillType::UNIVERSE:
      return "UNIVERSE";
    default:
      return "VOID";
  }
}
} // namespace CSG
