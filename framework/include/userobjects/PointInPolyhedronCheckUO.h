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
#include "SurfacePointClassifier.h"

class BoundaryMeshBuilder;

/**
 * Determines whether a point is inside a single closed surface mesh provided by a
 * BoundaryMeshBuilder. Owns one SurfacePointClassifier (the selected backend) and
 * overrides UserObject::spatialValue() so it works directly with
 * SpatialUserObjectAux (1 for INSIDE/ON, 0 for OUTSIDE).
 */
class PointInPolyhedronCheckUO : public PointInPolyhedronBaseUO
{
public:
  static InputParameters validParams();
  PointInPolyhedronCheckUO(const InputParameters & parameters);

  virtual void initialSetup() override;

  /// Classify a point against the surface.
  SurfaceSide sideness(const Point & p) const { return _classifier->sideness(p); }

  /// Whether the point is inside or on the surface. (ON is treated as inside.)
  virtual bool ifInside(const Point & p) const { return _classifier->contains(p); }

  /// 1 for INSIDE/ON, 0 for OUTSIDE -- usable directly by SpatialUserObjectAux.
  virtual Real spatialValue(const Point & p) const override { return ifInside(p) ? 1.0 : 0.0; }

protected:
  /// Builder that owns the surface mesh and SurfaceElementSet.
  const BoundaryMeshBuilder & _builder;

  /// The selected point-containment backend, built in initialSetup().
  std::unique_ptr<SurfacePointClassifier> _classifier;
};
