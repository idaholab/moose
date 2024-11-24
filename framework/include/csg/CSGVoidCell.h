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
 * CSGVoidCell creates an internal representation of a Constructive Solid Geometry (CSG)
 * cell filled by a void, used to represent the region outside of the finite element mesh
 */
class CSGVoidCell : public CSGCell
{
public:
  CSGVoidCell(const std::string name);

  /**
   * Destructor
   */
  virtual ~CSGVoidCell() = default;
};
} // namespace CSG
