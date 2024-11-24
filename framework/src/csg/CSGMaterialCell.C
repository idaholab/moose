//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGMaterialCell.h"

namespace CSG
{

CSGMaterialCell::CSGMaterialCell(const std::string name)
  : CSGCell(name, FillType::material), _fill_name("")
{
}

CSGMaterialCell::CSGMaterialCell(const std::string name, const std::string fill_name)
  : CSGCell(name, FillType::material), _fill_name(fill_name)
{
}
} // namespace CSG
