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
 * CSGRegions creates an internal representation of a CSG region, which can refer to
 * an intersection, union, complement, or halfspace
 * CSGHalfspace objects
 */
class CSGRegion
{
public:
  /// An enum for type of type of operation that defines region
  enum class Operation
  {
    HALFSPACE,
    COMPLEMENT,
    INTERSECTION,
    UNION
  };

  /**
   * Constructor for halfspace
   */
  CSGRegion(std::shared_ptr<CSGSurface> surf, const CSGSurface::Direction direction, const CSGRegion::Operation region_op);

  /**
   * Constructor for complement, union, and intersection
   */
  CSGRegion(std::vector<const CSGRegion> & regions, const CSGRegion::Operation region_op);

  /**
   * Destructor
   */
  virtual ~CSGRegion() = default;

  std::string toString() const;

protected:
  /// String representation of region
  std::string _region_str;
};
} // namespace CSG
