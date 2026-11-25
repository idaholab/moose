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
 * Utilities for polygons in the xy plane given by their corners, as a mesh generator producing
 * libMesh::C0Polygon elements works with. Every routine here uses only the first two coordinates.
 */
namespace polygon_utils
{

/**
 * Whether a point is inside a polygon, by ray casting, with a point on the boundary counting as
 * inside. Unlike geom_utils::pointInPolygon() this does not assume the polygon is convex.
 * @param[in] point point of interest
 * @param[in] polygon the corners of the polygon, in order around it
 * @return whether the point is inside the polygon
 */
bool pointInPolygon2D(const libMesh::Point & point, const std::vector<libMesh::Point> & polygon);

/**
 * The offset to rotate a polygon's corners by so that it triangulates into non-degenerate
 * triangles. A libMesh::C0Polygon is triangulated by fanning from its first corner, so a polygon
 * with three or more collinear corners, as mid-side nodes produce, yields a zero-area triangle
 * whenever that corner is an end of a collinear run. Such a polygon is otherwise perfectly valid;
 * only the choice of fan apex has to avoid the run.
 * @param[in] points the corners of the polygon, in order around it
 * @param[in] area_tol twice the area below which a triangle is considered degenerate
 * @return the offset of a sound fan apex, or 0 if no rotation avoids a degenerate triangle
 */
std::size_t fanApexOffset(const std::vector<libMesh::Point> & points, const Real area_tol);

/**
 * Order points counter-clockwise about a center, in place.
 *
 * Sorting by std::atan2() of the offset from the center is not reproducible across platforms: a
 * point lying along -x from the center gives +pi or -pi depending on whether its y offset rounds to
 * +0.0 or -0.0, which moves it between the first and last position, and exactly equal angles are
 * left to std::sort() to break. Either rotates the resulting corner list, and so renumbers the
 * sides of the element built from it, without changing its shape.
 *
 * This instead compares two points by the half plane of the center they lie in and then by the
 * exact orientation predicate, falling back on their coordinates when they are collinear with the
 * center, which is a strict weak ordering decided entirely by the sign of exact arithmetic.
 *
 * @param[in,out] points the points to order
 * @param[in] center the point to order them about, which does not have to be inside their hull
 */
void sortPointsCounterClockwise(std::vector<libMesh::Point> & points,
                                const libMesh::Point & center);

} // end of namespace polygon_utils
