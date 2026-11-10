//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThreadedGeneralUserObject.h"
#include "PointInSurfaceCheckInterface.h"

#include <vector>

/**
 * Classifies a point against the union of several geometries, each supplied by a
 * user object implementing PointInSurfaceCheckInterface (a meshed surface checker,
 * a signed-function checker, or another union). Geometries may therefore be given
 * in mixed representations.
 *
 * The tri-state result follows the precedence INSIDE > ON > OUTSIDE: the point is
 * INSIDE if it is inside any geometry, else ON if it is on any geometry, else
 * OUTSIDE. Equivalently the boolean union is "inside any / outside every".
 *
 * This is a ThreadedGeneralUserObject: some providers (e.g. the signed-function
 * checker) keep per-thread state, so each thread must resolve and call its own
 * per-thread provider copies. It also implements PointInSurfaceCheckInterface so a
 * union may itself be nested inside another union.
 */
class PointInUnionCheckUO : public ThreadedGeneralUserObject, public PointInSurfaceCheckInterface
{
public:
  static InputParameters validParams();
  PointInUnionCheckUO(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}

  // Answers queries on demand and stores no results, so there is nothing to
  // combine across threads.
  virtual void threadJoin(const UserObject &) override {}

  virtual SurfaceGeometry::SurfaceSide sideness(const Point & p) const override;

  /// 1 for INSIDE/ON, 0 for OUTSIDE -- usable directly by SpatialUserObjectAux.
  virtual Real spatialValue(const Point & p) const override { return contains(p) ? 1.0 : 0.0; }

protected:
  /// The geometries whose union is tested (non-owning; owned by the warehouse).
  std::vector<const PointInSurfaceCheckInterface *> _providers;
};
