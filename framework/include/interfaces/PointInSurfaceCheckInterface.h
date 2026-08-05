//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SurfaceSide.h"

namespace libMesh
{
class Point;
}

/**
 * Mixin interface implemented by user objects that can classify a query point
 * against a closed surface (or a union of closed surfaces).
 *
 * This is deliberately NOT a UserObject: keeping it a plain abstract class lets a
 * concrete object multiply-inherit it alongside either GeneralUserObject (for the
 * mesh-based, thread-safe checkers) or ThreadedGeneralUserObject (for checkers
 * that evaluate a Function and therefore need a per-thread copy). Deriving the
 * interface itself from GeneralUserObject would create a diamond in the threaded
 * case, so it carries no base and no data.
 *
 * Consumers hold objects polymorphically as PointInSurfaceCheckInterface and, to
 * fetch one by name, dynamic_cast the UserObjectBase to this interface -- the same
 * pattern MOOSE uses for role mixins such as BlockRestrictable.
 */
class PointInSurfaceCheckInterface
{
public:
  virtual ~PointInSurfaceCheckInterface() = default;

  /// Classify a point as INSIDE, ON, or OUTSIDE the surface(s).
  virtual SurfaceSide sideness(const libMesh::Point & p) const = 0;

  /// Whether the point is inside or on the surface(s). ON maps to true.
  bool contains(const libMesh::Point & p) const { return sideness(p) != SurfaceSide::OUTSIDE; }
};
