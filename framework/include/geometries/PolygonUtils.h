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

} // end of namespace polygon_utils
