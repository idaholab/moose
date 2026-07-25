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
  BoundingBox bbox = _elem->loose_bounding_box();

  const Point & min_pt = bbox.first;
  const Point & max_pt = bbox.second;

  // Step (a): Calculate box_vec
  Point box_vec = max_pt - min_pt;

  // Step (b): Project box_vec onto normal_dir
  Real normal_scale = box_vec * normal_dir;
  Point normal_box_vec = normal_dir * normal_scale;

  // Step (c): Calculate tangent_vec and its norm
  Point tangent_vec = box_vec - normal_box_vec;

  return tangent_vec.norm();
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
