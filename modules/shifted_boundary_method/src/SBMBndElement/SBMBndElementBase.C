//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SBMBndElementBase.h"
#include "Ball.h"
#include "GeometryBase.h"
#include "LineSegment.h"
#include "libmesh/string_to_enum.h"

SBMBndElementBase::SBMBndElementBase(const Elem * elem, const Point & normal)
  : _elem(elem), _normal(normal)
{
  mooseAssert(elem, "Element must not be null");
  mooseAssert(MooseUtils::absoluteFuzzyEqual(_normal.norm(), 1),
              "normal vector must be unit length, length = " << _normal.norm());
}

Point
SBMBndElementBase::distanceFrom(const Point & pt) const
{
  // Validate side type upfront so misconfigured elements fail loudly on the
  // first query, even when the projection short-circuit below would otherwise
  // skip the side loop and silently return a "valid" answer.
  // libMesh's standard element types have uniform side types, so checking
  // side(0) is sufficient.
  if (_elem->n_sides() > 0)
  {
    const auto t = _elem->build_side_ptr(0)->type();
    if (t != EDGE2 && t != NODEELEM)
      mooseError("Unsupported side type in distanceFrom(): ", libMesh::Utility::enum_to_string(t));
  }

  // (a) Project pt onto the normal direction
  const auto vec_to_first = _elem->point(0) - pt;
  const auto scale = vec_to_first * _normal;
  const auto projection = _normal * scale;

  // Check if projection point lands inside the geometry
  if (_elem->contains_point(pt + projection))
    return projection;

  // (d) Point to closest edge or node
  Real min_dist = std::numeric_limits<Real>::max();
  Point closest_vec;

  const unsigned int n_edges = _elem->n_sides();
  for (unsigned int j = 0; j < n_edges; ++j)
  {
    std::unique_ptr<const Elem> curr_edge = _elem->build_side_ptr(j);

    switch (curr_edge->type())
    {
      case EDGE2:
      {
        const Point & p1 = *curr_edge->node_ptr(0);
        const Point & p2 = *curr_edge->node_ptr(1);

        const Point edge = p2 - p1;
        Real t = ((pt - p1) * edge) / (edge * edge);
        t = std::clamp(t, 0.0, 1.0);
        const Point proj = p1 + t * edge;
        const Real dist = (pt - proj).norm();

        if (dist < min_dist)
        {
          min_dist = dist;
          closest_vec = proj - pt;
        }
        break;
      }

      case NODEELEM:
      {
        const Point & p = *curr_edge->node_ptr(0);
        const Real dist = (pt - p).norm();
        if (dist < min_dist)
        {
          min_dist = dist;
          closest_vec = p - pt;
        }
        break;
      }

      default:
        mooseAssert(false, "unreachable: side type validated at top of distanceFrom()");
    }
  }

  return closest_vec;
}

Real
SBMBndElementBase::getProjectedBoundingBoxDiagonal(const Point & normal_dir) const
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
SBMBndElementBase::intersect(const LineSegment & line_segment) const
{
  if (const auto * geom = dynamic_cast<const GeometryBase *>(this))
    return geom->intersect(line_segment);

  mooseError("SBMBndElementBase::intersect: unsupported geometry type");
}

Ball
SBMBndElementBase::computeBoundingBall() const
{
  if (const auto * geom = dynamic_cast<const GeometryBase *>(this))
    return geom->computeBoundingBall();

  mooseError("SBMBndElementBase::computeBoundingBall: unsupported geometry type");
}
