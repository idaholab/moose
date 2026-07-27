//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SurfaceElement.h"
#include "Ball.h"
#include "GeometryBase.h"
#include "LineSegment.h"

#include <algorithm>

SurfaceElement::SurfaceElement(const Elem * elem, const Point & normal)
  : _elem(elem), _normal(normal)
{
  mooseAssert(elem, "Element must not be null");
  mooseAssert(MooseUtils::absoluteFuzzyEqual(_normal.norm(), 1),
              "normal vector must be unit length, length = " << _normal.norm());
}

Real
SurfaceElement::getProjectedBoundingBoxDiagonal(const Point & normal_dir) const
{
  const BoundingBox bbox = _elem->loose_bounding_box();
  const Point d = bbox.second - bbox.first; // (dx, dy, dz), each >= 0

  // The longest diagonal of the projected AABB (its shadow on the plane orthogonal to
  // normal_dir) is the search radius that must cover this element's projected footprint.
  // Projecting only the main diagonal (dx, dy, dz) underestimates it whenever that diagonal
  // is nearly parallel to normal_dir. The projected footprint diameter is the max projected
  // length over all four space diagonals (dx, +/-dy, +/-dz); their negatives have identical
  // projected length, so these four combinations are exhaustive.
  Real max_projected = 0.0;
  for (const Real sy : {1.0, -1.0})
    for (const Real sz : {1.0, -1.0})
    {
      const Point diag(d(0), sy * d(1), sz * d(2));
      const Point tangent_vec = diag - normal_dir * (diag * normal_dir);
      max_projected = std::max(max_projected, tangent_vec.norm());
    }

  return max_projected;
}

bool
SurfaceElement::intersect(const LineSegment & line_segment) const
{
  if (const auto * geom = dynamic_cast<const GeometryBase *>(this))
    return geom->intersect(line_segment);

  mooseError("SurfaceElement::intersect: unsupported geometry type");
}

Ball
SurfaceElement::computeBoundingBall() const
{
  if (const auto * geom = dynamic_cast<const GeometryBase *>(this))
    return geom->computeBoundingBall();

  mooseError("SurfaceElement::computeBoundingBall: unsupported geometry type");
}
