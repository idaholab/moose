//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PointInPolyhedronBaseUO.h"
#include "PointInSurfaceCheckInterface.h"
#include "PointContainmentClassifier.h"

class BoundaryMeshBuilder;

/**
 * Determines whether a point is inside a single closed surface mesh provided by a
 * BoundaryMeshBuilder. Owns one PointContainmentClassifier (the selected backend) and
 * overrides UserObject::spatialValue() so it works directly with
 * SpatialUserObjectAux (1 for INSIDE/ON, 0 for OUTSIDE).
 *
 * Implements PointInSurfaceCheckInterface so it can be composed into a
 * PointInUnionCheckUO. The classifier is read-only after setup and thread-safe, so
 * this stays a (non-threaded) GeneralUserObject.
 */
class PointInPolyhedronCheckUO : public PointInPolyhedronBaseUO, public PointInSurfaceCheckInterface
{
public:
  static InputParameters validParams();
  PointInPolyhedronCheckUO(const InputParameters & parameters);

  virtual void initialSetup() override;

  /// Classify a point against the surface.
  SurfaceGeometry::SurfaceSide sideness(const Point & p) const override
  {
    return _classifier->sideness(p);
  }

  /// The resolved ray direction used by the ray-casting backend.
  Point rayDirection() const { return _classifier->rayDirection(); }

  /// 1 for INSIDE/ON, 0 for OUTSIDE -- usable directly by SpatialUserObjectAux.
  virtual Real spatialValue(const Point & p) const override { return contains(p) ? 1.0 : 0.0; }

protected:
  /// Builder that owns the surface mesh and SurfaceElementSet.
  const BoundaryMeshBuilder & _builder;

  /// The selected point-containment backend, built in initialSetup().
  std::unique_ptr<PointContainmentClassifier> _classifier;
};
