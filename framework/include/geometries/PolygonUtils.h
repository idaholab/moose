//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"
#include "libmesh/point.h"

#include <vector>

/**
 * Utilities for building polygons in the xy plane from points, as a mesh generator producing
 * libMesh::C0Polygon elements has to. Every routine here uses only the first two coordinates.
 */
namespace polygon_utils
{

/**
 * Append a point to a list unless the list already holds one at the same position. Used wherever a
 * polygon's vertices are gathered from several constructions that can reach the same point.
 * @param[in,out] points the points gathered so far
 * @param[in] point the candidate point
 * @param[in] length_tol distance within which two points are considered the same
 */
void addUniquePoint(std::vector<libMesh::Point> & points,
                    const libMesh::Point & point,
                    const Real length_tol);

/**
 * Whether a point lies on the segment between two points, treating a degenerate segment as the
 * single point it collapses to.
 * @param[in] point point of interest
 * @param[in] a first end point of the segment
 * @param[in] b second end point of the segment
 * @return whether the point lies on the segment
 */
bool pointOnSegment2D(const libMesh::Point & point,
                      const libMesh::Point & a,
                      const libMesh::Point & b);

/**
 * Add the points where the segment p0-p1 meets the segment q0-q1, which may be none, one, or, for
 * overlapping collinear segments, the end points of the overlap. Degenerate segments are handled as
 * the single point they collapse to. These points are the ones a polygon needs when it is clipped
 * against another.
 * @param[in,out] points the points gathered so far, appended to uniquely
 * @param[in] p0 first end point of the first segment
 * @param[in] p1 second end point of the first segment
 * @param[in] q0 first end point of the second segment
 * @param[in] q1 second end point of the second segment
 * @param[in] length_tol distance within which two points are considered the same
 */
void addSegmentIntersections2D(std::vector<libMesh::Point> & points,
                               const libMesh::Point & p0,
                               const libMesh::Point & p1,
                               const libMesh::Point & q0,
                               const libMesh::Point & q1,
                               const Real length_tol);

/**
 * Whether a point is inside a polygon, by ray casting, with a point on the boundary counting as
 * inside. Unlike geom_utils::pointInPolygon() this does not assume the polygon is convex.
 * @param[in] point point of interest
 * @param[in] polygon the corners of the polygon, in order around it
 * @return whether the point is inside the polygon
 */
bool pointInPolygon2D(const libMesh::Point & point, const std::vector<libMesh::Point> & polygon);

/**
 * The offset to rotate a polygon's vertices by so that it triangulates into non-degenerate
 * triangles. A libMesh::C0Polygon is triangulated by fanning from its first vertex, so that vertex
 * must not be collinear with any edge the fan spans; a polygon with three or more collinear
 * vertices, as mid-side nodes produce, can otherwise yield a zero-area triangle.
 * @param[in] points the corners of the polygon, in order around it
 * @param[in] area_tol twice the area below which a triangle is considered degenerate
 * @return the offset of a sound fan apex, or 0 if no rotation avoids a degenerate triangle
 */
std::size_t fanApexOffset(const std::vector<libMesh::Point> & points, const Real area_tol);

} // end of namespace polygon_utils
