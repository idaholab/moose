//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeometryBase.h"
#include "libmesh/point.h"
#include "libmesh/sphere.h"

class LineSegment;

/**
 * Ball primitive: a circle in 2D or a sphere in 3D.
 *
 * Why not just use libMesh::Sphere?
 *  - libMesh::Sphere is 3D only; Ball supports both 2D and 3D in a single type.
 *  - Ball provides the operations the framework geometries layer needs:
 *    intersection with a LineSegment and a tight bounding ball used for fast
 *    nearest-neighbor / spatial searches. libMesh::Sphere exposes a different
 *    set of surface queries (closest point, normal, above/below surface) that
 *    we do not need here.
 *  - Ball derives from GeometryBase so every framework geometry primitive can
 *    be used through one virtual interface.
 *
 * If a caller does need the libMesh surface-query interface (e.g. ray tracing),
 * use `toSphere()` to convert.
 */
class Ball : public GeometryBase
{
public:
  Ball(const libMesh::Point & c, libMesh::Real r) : _c(c), _r(r) {}

  ~Ball() override = default;

  const libMesh::Point & center() const { return _c; }
  libMesh::Real radius() const { return _r; }

  /**
   * Check if a line segment intersects this ball.
   */
  bool intersect(const LineSegment & line_segment) const override;

  /**
   * return the ball itself
   */
  Ball computeBoundingBall() const override;

#if LIBMESH_DIM > 2
  libMesh::Sphere toSphere() const { return libMesh::Sphere(_c, _r); }
#endif

private:
  libMesh::Point _c;
  libMesh::Real _r;
};
