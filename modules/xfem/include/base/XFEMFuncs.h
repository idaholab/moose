//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <vector>
#include "libmesh/point.h"
#include "EFAPoint.h"

using namespace libMesh;

namespace Xfem
{
static const double tol = 1.0e-10;

void dunavant_rule2(const Real * wts,
                    const Real * a,
                    const Real * b,
                    const unsigned int * permutation_ids,
                    unsigned int n_wts,
                    std::vector<Point> & points,
                    std::vector<Real> & weights);

void stdQuadr2D(unsigned int nen, unsigned int iord, std::vector<std::vector<Real>> & sg2);

void wissmannPoints(unsigned int nqp, std::vector<std::vector<Real>> & wss);

void shapeFunc2D(unsigned int nen,
                 std::vector<Real> & ss,
                 std::vector<Point> & xl,
                 std::vector<std::vector<Real>> & shp,
                 Real & xsj,
                 bool natl_flg);

double r8vec_norm(int n, double a[]);

void r8vec_copy(int n, double a1[], double a2[]);

bool r8vec_eq(int n, double a1[], double a2[]);

double r8vec_dot_product(int n, double a1[], double a2[]);

bool line_exp_is_degenerate_nd(int dim_num, double p1[], double p2[]);

int plane_normal_line_exp_int_3d(
    double pp[3], double normal[3], double p1[3], double p2[3], double pint[3]);

double polyhedron_volume_3d(
    double coord[], int order_max, int face_num, int node[], int node_num, int order[]);

void i4vec_zero(int n, int a[]);

void normalizePoint(Point & p);

void normalizePoint(EFAPoint & p);

double r8_acos(double c);

double angle_rad_3d(double p1[3], double p2[3], double p3[3]);

/**
 * Determine whether a line segment is intersected by a cutting line, and compute the
 * fraction along that line where the intersection occurs
 * @param segment_point1 Point at one end of the line segment
 * @param segment_point1 Point at other end of the line segment
 * @param cutting_line_points Pair of points that define the cutting line
 * @param cutting_line_fraction Fractional distance from the start to end point of the
 *                              cutting line over which the cutting line is currently
 *                              active
 * @param segment_intersection_fraction Frictional distance along the cut segment from
 *                                      segment_point1 where the intersection occurs
 * @return true if the segment is intersected, false if it is not
 */
bool intersectSegmentWithCutLine(const Point & segment_point1,
                                 const Point & segment_point2,
                                 const std::pair<Point, Point> & cutting_line_points,
                                 const Real & cutting_line_fraction,
                                 Real & segment_intersection_fraction);

/**
 * Compute the cross product of two vectors, provided as Point objects, which
 * have nonzero components only in the x,y plane. Because the vectors both lie
 * in the x,y plane, the only nonzero component of the cross product vector is
 * in the z direction, and that is returned as a scalar value by this function.
 * @param point_a First vector in cross product
 * @param point_b Second vector in cross product
 * @return z component of cross product vector
 */
Real crossProduct2D(const Point & point_a, const Point & point_b);

} // namespace Xfem
