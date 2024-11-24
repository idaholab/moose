//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CSGSurface.h"

namespace CSG
{

/**
 * CSGHalfspace creates an internal representation of a positive or negative halfspace
 * for a CSGSurface structure
 */
class CSGHalfspace
{
public:
  /**
   * Default constructor
   */
  CSGHalfspace(std::shared_ptr<CSGSurface> surf, const CSGSurface::Direction direction);

  /**
   * Destructor
   */
  virtual ~CSGHalfspace() = default;

  CSGSurface::Direction getInverseDirection() const;

  std::string toString() const;

protected:
  /// CSGSurface object
  std::shared_ptr<CSGSurface> _surface;

  /// Type of surface that is being represented
  const CSGSurface::Direction _direction;
};
} // namespace CSG
