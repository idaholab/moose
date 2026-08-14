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

class Function;

/**
 * Point-containment checker backed by a signed level-set Function: the point is
 * labeled from the sign of the function value (INSIDE where it is negative,
 * OUTSIDE where positive, ON within tolerance of zero).
 *
 * This is a ThreadedGeneralUserObject because evaluating a parsed Function mutates
 * internal state; each thread keeps its own Function pointer to avoid a data race,
 * mirroring ShortestDistanceToSurface.
 */
class PointInSignedFunctionCheckUO : public ThreadedGeneralUserObject,
                                     public PointInSurfaceCheckInterface
{
public:
  static InputParameters validParams();
  PointInSignedFunctionCheckUO(const InputParameters & parameters);

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
  /// The signed level-set function (per-thread copy).
  const Function & _func;

  /// Half-width (in function value space) of the ON band around the zero level set.
  const Real _tolerance;

  /// Whether negative function values denote the interior (SBM convention).
  const bool _inside_is_negative;
};
