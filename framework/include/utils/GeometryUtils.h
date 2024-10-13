//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"
#include "libmesh/point.h"
#include "libmesh/mesh_tools.h"

namespace geom_utils
{
/**
 * Check whether a point is equal to zero
 * @param[in] pt point
 * @return whether point is equal to the zero point
 */
bool isPointZero(const libMesh::Point & pt);

/**
 * Get the unit vector for a point parameter
 * @param[in] pt point
 * @param[in] name name of the parameter
 */
libMesh::Point unitVector(const libMesh::Point & pt, const std::string & name);

/**
 * Rotate point about an axis
 * @param[in] p point
 * @param[in] angle angle to rotate (radians)
 * @param[in] axis axis expressed as vector
 * @return rotated point
 */
libMesh::Point rotatePointAboutAxis(const libMesh::Point & p,
                                    const libMesh::Real angle,
                                    const libMesh::Point & axis);

/**
 * Get the minimum distance from a point to another set of points, in the plane
 * perpendicular to the specified axis
 * @param[in] pt point
 * @param[in] candidates set of points we will find the nearest of
 * @param[in] axis axis perpendicular to the plane of the polygon
 * @return minimum distance to the provided points
 */
libMesh::Real minDistanceToPoints(const libMesh::Point & pt,
                                  const std::vector<libMesh::Point> & candidates,
                                  const unsigned int axis);

/**
 * Get the corner coordinates of a regular 2-D polygon, assuming a face of the polygon
 * is parallel to the x0 axis
 * @param[in] num_sides number of sides to polygon
 * @param[in] radius distance from polygon center to a corner
 * @param[in] axis axis perpendicular to the plane of the polygon
 * @return corner coordinates
 */
std::vector<libMesh::Point>
polygonCorners(const unsigned int num_sides, const libMesh::Real radius, const unsigned int axis);

/**
 * Get the indices of the plane perpendicular to the specified axis.
 * For example, if the axis is the y-axis (1), then this will return
 * (0, 2), indicating that the coordinates of a general 3-D point once
 * projected onto the x-z plane can be obtained with the 0 and 2 indices.
 * @param[in] axis axis perpendicular to the projection plane
 * @return indices of coordinates on plane
 */
std::pair<unsigned int, unsigned int> projectedIndices(const unsigned int axis);

/**
 * Given two coordinates, construct a point in the 2-D plane perpendicular to the
 * specified axis.
 * @param[in] x0 first coordinate
 * @param[in] x1 second coordinate
 * @param[in] axis axis perpendicular to the projection plane
 * @return point
 */
libMesh::Point
projectPoint(const libMesh::Real x0, const libMesh::Real x1, const unsigned int axis);

/**
 * Get the unit normal vector between two points (which are first projected onto
 * the plane perpendicular to the 'axis'), such that the cross product of
 * the unit normal with the line from pt1 to pt2 has a positive 'axis' component.
 * @param[in] pt1 first point for line
 * @param[in] pt2 second point for line
 * @param[in] axis project points onto plane perpendicular to this axis
 * @return unit normal
 */
libMesh::Point projectedUnitNormal(libMesh::Point pt1, libMesh::Point pt2, const unsigned int axis);

/**
 * Compute the distance from a 3-D line, provided in terms of two points on the line
 * @param[in] pt point of interest
 * @param[in] line0 first point on line
 * @param[in] line1 second point on line
 * @return distance from line
 */
libMesh::Real distanceFromLine(const libMesh::Point & pt,
                               const libMesh::Point & line0,
                               const libMesh::Point & line1);

/**
 * Compute the distance from a 3-D line, provided in terms of two points on the line.
 * Both the input point and the points on the line are projected into the 2-d plane
 * perpendicular to the specified axis.
 * @param[in] pt point of interest
 * @param[in] line0 first point on line
 * @param[in] line1 second point on line
 * @param[in] axis axis index (0 = x, 1 = y, 2 = z) perpendicular to the projection plane
 * @return distance from line
 */
libMesh::Real projectedDistanceFromLine(libMesh::Point pt,
                                        libMesh::Point line0,
                                        libMesh::Point line1,
                                        const unsigned int axis);

/**
 * If positive, point is on the positive side of the half space (and vice versa). Because
 * a 3-D line does not have a negative or positive "side," you must provide the 'axis'
 * perpendicular to the plane into which the point and line are first projected.
 * @param[in] pt1 point of interest
 * @param[in] pt2 one end point of line
 * @param[in] pt3 other end point of line
 * @param[in] axis axis perpendicular to plane onto which point and line are first projected
 * @return half space of line
 */
libMesh::Real projectedLineHalfSpace(libMesh::Point pt1,
                                     libMesh::Point pt2,
                                     libMesh::Point pt3,
                                     const unsigned int axis);

/**
 * Whether a point is in 2-D a polygon in the plane perpendicular to the specified
 * axis, given by corner points
 * @param[in] point point of interest
 * @param[in] corners corner points of polygon
 * @param[in] axis axis perpendicular to the plane of the polygon
 * @return whether point is inside the polygon
 */
bool pointInPolygon(const libMesh::Point & point,
                    const std::vector<libMesh::Point> & corners,
                    const unsigned int axis);

/**
 * Whether a point is on the edge of a 2-D polygon in the plane perpendicular to
 * the specified axis, given its corner points
 * @param[in] point point of interest
 * @param[in] corners corner points of polygon
 * @param[in] axis axis perpendicular to the plane of the polygon
 * @return whether point is on edge of polygon
 */
bool pointOnEdge(const libMesh::Point & point,
                 const std::vector<libMesh::Point> & corners,
                 const unsigned int axis);

/**
 * Get corner points of a bounding box, with side length re-scaled
 * @param[in] box bounding box to start from
 * @param[in] factor by which to multiply the bounding box side
 */
std::vector<libMesh::Point> boxCorners(const libMesh::BoundingBox & box,
                                       const libMesh::Real factor);
} // end of namespace geom_utils
