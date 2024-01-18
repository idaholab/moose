//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EFAFuncs.h"

namespace Efa
{

double
linearQuadShape2D(unsigned int node_id, std::vector<double> & xi_2d)
{
  double node_xi[4][2] = {{-1.0, -1.0}, {1.0, -1.0}, {1.0, 1.0}, {-1.0, 1.0}};
  return 0.25 * (1.0 + node_xi[node_id][0] * xi_2d[0]) * (1.0 + node_xi[node_id][1] * xi_2d[1]);
}

double
linearTriShape2D(unsigned int node_id, std::vector<double> & xi_2d)
{
  std::vector<double> area_xi(3, 0.0);
  area_xi[0] = xi_2d[0];
  area_xi[1] = xi_2d[1];
  area_xi[2] = 1.0 - xi_2d[0] - xi_2d[1];
  return area_xi[node_id];
}

double
linearHexShape3D(unsigned int node_id, std::vector<double> & xi_3d)
{
  double node_xi[8][3] = {{-1.0, -1.0, -1.0},
                          {1.0, -1.0, -1.0},
                          {1.0, 1.0, -1.0},
                          {-1.0, 1.0, -1.0},
                          {-1.0, -1.0, 1.0},
                          {1.0, -1.0, 1.0},
                          {1.0, 1.0, 1.0},
                          {-1.0, 1.0, 1.0}};
  return 0.125 * (1.0 + node_xi[node_id][0] * xi_3d[0]) * (1.0 + node_xi[node_id][1] * xi_3d[1]) *
         (1.0 + node_xi[node_id][2] * xi_3d[2]);
}

double
linearTetShape3D(unsigned int node_id, std::vector<double> & xi_3d)
{
  std::vector<double> vol_xi(4, 0.0);
  for (unsigned int i = 0; i < 3; ++i)
    vol_xi[i] = xi_3d[i];
  vol_xi[3] = 1.0 - xi_3d[0] - xi_3d[1] - xi_3d[2];
  return vol_xi[node_id];
}

bool
IntersectionPointTwoLineSegments2D(const std::vector<double> & segment1_point1,
                                   const std::vector<double> & segment1_point2,
                                   const std::vector<double> & segment2_point1,
                                   const std::vector<double> & segment2_point2,
                                   std::vector<double> & intersect_point)
{
  // Use the algorithm described here to determine whether a line segment is intersected
  // by a cutting line, and to compute the coordinates of the point where the intersection
  // occurs:
  // http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
  // additionally modified for the specific use here
  bool cut_segment = false;
  std::vector<double> seg_dir = {0, 0};
  seg_dir[0] = segment1_point2[0] - segment1_point1[0];
  seg_dir[1] = segment1_point2[1] - segment1_point1[1];
  std::vector<double> cut_dir = {0, 0};
  cut_dir[0] = segment2_point2[0] - segment2_point1[0];
  cut_dir[1] = segment2_point2[1] - segment2_point1[1];
  std::vector<double> cut_start_to_seg_start = {0, 0};
  cut_start_to_seg_start[0] = segment1_point1[0] - segment2_point1[0];
  cut_start_to_seg_start[1] = segment1_point1[1] - segment2_point1[1];

  double cut_dir_cross_seg_dir = crossProduct2D(cut_dir, seg_dir);

  if (std::abs(cut_dir_cross_seg_dir) > Efa::tol)
  {
    // Fraction of the distance along the cutting segment where it intersects the edge segment
    double cut_int_frac = crossProduct2D(cut_start_to_seg_start, seg_dir) / cut_dir_cross_seg_dir;

    if (cut_int_frac >= 0.0 && cut_int_frac <= 1)
    { // Cutting segment intersects the line of the edge segment, but the intersection point may
      // be outside the segment
      double int_frac = crossProduct2D(cut_start_to_seg_start, cut_dir) / cut_dir_cross_seg_dir;
      if (int_frac >= 0.0 && int_frac <= 1.0)
      {
        cut_segment = true;
        intersect_point.resize(2);
        intersect_point[0] = segment1_point1[0] + int_frac * seg_dir[0];
        intersect_point[1] = segment1_point1[1] + int_frac * seg_dir[1];
      }
    }
  }

  return cut_segment;
}

double
crossProduct2D(const std::vector<double> & point_a, const std::vector<double> & point_b)
{
  return (point_a[0] * point_b[1] - point_b[0] * point_a[1]);
}

double
distanceBetweenPoints2D(const std::vector<double> & point_a, const std::vector<double> & point_b)
{
  // Multiplying out the long way to avoid calling std::pow():
  return std::sqrt((point_b[0] - point_a[0]) * (point_b[0] - point_a[0]) +
                   (point_b[1] - point_a[1]) * (point_b[1] - point_a[1]));
}

} // namespace Efa
