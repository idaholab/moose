//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Reference for B-Spline: https://mat.fsv.cvut.cz/gcg/sbornik/prochazkova.pdf

#pragma once

#include "libMeshReducedNamespace.h"

namespace SplineUtils
{
/**
 * Creates control points for the special case of parallel lines using an interpolating circle.
 * @param start_point start point for spline
 * @param end_point end point for spline
 * @param parallel_direction direction of both curves to be connected. must be the same for both
 * @param num_cps total number of control points to be created
 */
std::vector<Point> circularControlPoints(const libMesh::Point & start_point,
                                         const libMesh::Point & end_point,
                                         const libMesh::RealVectorValue & parallel_direction,
                                         const unsigned int num_cps);

/**
 * Creates control points for an open uniform BSpline
 * @param start_point start point for spline
 * @param end_point end point for spline
 * @param start_direction incoming direction (derivative)
 * @param end_direction outgoing direction (derivative)
 * @param cps_per_half number of control points per half of the spline -- will be along extrapolated
 * line from the point and its direction
 * @param sharpness number in [0,1] that determines the sharpness of the curve
 */
std::vector<Point> bSplineControlPoints(const libMesh::Point & start_point,
                                        const libMesh::Point & end_point,
                                        const libMesh::RealVectorValue & start_direction,
                                        const libMesh::RealVectorValue & end_direction,
                                        const unsigned int cps_per_half,
                                        const libMesh::Real sharpness);

/**
 * Creates control points along an extrapolated line up to a certain intercept.
 * @param start_point start point for the line
 * @param end_point end point of the line
 * @param direction_vector direction of the line
 * @param sharpness number in [0,1] that determines the sharpness of the future curve. In this
 * context, it will determine how close to the end point the control points are placed.
 * @param build_backwards boolean to build the second vector of control points in the correct order
 */
std::vector<Point> controlPointsAlongLine(const libMesh::Point & start_point,
                                          const libMesh::Point & end_point,
                                          const libMesh::RealVectorValue & direction_vector,
                                          const libMesh::Real sharpness,
                                          const unsigned int num_cps,
                                          const bool build_backwards = false);
/**
 * Makes the control point. Subroutine of controlPointsAlongLine
 * @param start_point start point for the line
 * @param end_point end point of the line
 * @param direction_vector direction of the line
 * @param sharpness number in [0,1] that determines the sharpness of the future curve. In this
 * context, it will determine how close to the end point the control point is placed.
 */
libMesh::Point makeControlPoint(const libMesh::Point & start_point,
                                const libMesh::Point & end_point,
                                const libMesh::RealVectorValue & direction_vector,
                                const libMesh::Real sharpness);
/**
 * Determines the two points defining the shortest line segment between two lines in space. If the
 * lines intersection, the closest points returned will be identical.
 * @param point_1 starting point of the first line
 * @param point_2 starting point of the second line
 * @param direction_1 direction of the first line
 * @param direction_2 direction of the second line
 */
std::vector<Point> closestPoints(const libMesh::Point & point_1,
                                 const libMesh::Point & point_2,
                                 const libMesh::RealVectorValue & direction_1,
                                 const libMesh::RealVectorValue & direction_2);

}
