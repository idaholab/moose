//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CSGCell.h"

namespace CSG
{

/**
 * CSGMaterialCell creates an internal representation of a Constructive Solid Geometry (CSG)
 * cell filled by a material. In this case, the subdomain ID / name of the mesh element
 * is used as a proxy for the material name
 */
class CSGMaterialCell : public CSGCell
{
public:
  CSGMaterialCell(const std::string name);

  CSGMaterialCell(const std::string name, const std::string fill_name);

  std::string getFill() const { return _fill_name; }

  /**
   * Destructor
   */
  virtual ~CSGMaterialCell() = default;

protected:
  /// Name of material fill for cell
  std::string _fill_name;
};
} // namespace CSG
