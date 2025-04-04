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
  : _name(name), _fill_type(FillType::MATERIAL), _fill_material(mat_name), _region(region)
{
}

template <typename T>
T
CSGCell::getFill()
{
  if (getFillType() == FillType::VOID)
    return NULL;
  if (getFillType() == FillType::MATERIAL)
    return _fill_material;
  if (getFillType() == FillType::UNIVERSE)
    return _fill_universe;
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
