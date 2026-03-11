//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SplineUtils.h"
#include "MooseError.h"
#include "libMeshReducedNamespace.h"
#include <algorithm>

namespace SplineUtils
{
std::vector<Point>
circularControlPoints(const libMesh::Point & start_point,
                      const libMesh::Point & end_point,
                      const libMesh::RealVectorValue & parallel_direction,
                      const unsigned int num_cps)
{
  // establish the direction vector between the two points
  const auto orthogonal_direction = start_point - end_point;

  // circular control points will only honor the derivatives if the points are colinear along the
  // orthogonal direction
  if (!(MooseUtils::absoluteFuzzyEqual(parallel_direction.unit() * orthogonal_direction.unit(), 0)))
    mooseError("Parallel lines cannot be interpolated using points along a circle! Direction "
               "vectors are incompatible with point locations.\nStart point: ",
               start_point,
               "\nEnd point: ",
               end_point,
               "\nDirection: ",
               parallel_direction);

  // define the distance between the two points
  const auto distance_between = orthogonal_direction.norm();

  // connecting circle is assumed to be between each point along the orthogonal_direction vector
  const auto radius = distance_between / 2.0;

  // normalize vectors
  const auto unit_normal = orthogonal_direction / distance_between;
  const auto unit_parallel = parallel_direction / parallel_direction.norm();

  // calculate circle center
  const auto center_point = end_point + radius * unit_normal;

  // initialize vector of control points
  std::vector<Point> control_points;

  // loop over the number of control points to generate the control points
  mooseAssert(num_cps > 1, "This routine does not support a single control point");
  for (const auto i : make_range(num_cps))
  {
    const auto t = (Real)i / (Real)(num_cps - 1);
    control_points.push_back(center_point + radius * (std::cos(t * M_PI) * unit_normal +
                                                      std::sin(t * M_PI) * unit_parallel));
  }

  return control_points;
}

std::vector<Point>
bSplineControlPoints(const libMesh::Point & start_point,
                     const libMesh::Point & end_point,
                     const libMesh::RealVectorValue & start_direction,
                     const libMesh::RealVectorValue & end_direction,
                     const unsigned int cps_per_half,
                     const libMesh::Real sharpness)
{
  // check that start_point is different from end_point
  if ((start_point - end_point).norm_sq() < libMesh::TOLERANCE)
    mooseError("Start and end points must be different.");
  // check that neither direction is the zero vector
  if (start_direction.norm_sq() < libMesh::TOLERANCE ||
      end_direction.norm_sq() < libMesh::TOLERANCE)
    mooseError("Direction vectors must have a nonzero magnitude.");
  mooseAssert(sharpness <= 1.0 && sharpness >= 0, "Sharpness must be in [0,1]!");

  // check if directions are parallel, use a special case if so
  const bool parallel = ((start_direction.cross(end_direction)).norm() < libMesh::TOLERANCE);
  if (parallel)
  {
    mooseWarning("Directions are parallel! Attempting to use circular control points...");
    unsigned int num_cps = 2 * cps_per_half + 1;
    if (num_cps < 30)
      mooseWarning("Number of control points required for circular control points is much greater "
                   "than for BSplines. Current num_cps: " +
                   std::to_string(num_cps));
    return SplineUtils::circularControlPoints(start_point, end_point, start_direction, num_cps);
  }

  // find closest points --> these will be identical if the extrapolated lines intersect
  const auto closest_points_vector =
      SplineUtils::closestPoints(start_point, end_point, start_direction, end_direction);
  const auto & closest_point_1 = closest_points_vector.first;
  const auto & closest_point_2 = closest_points_vector.second;

  std::vector<Point> first_half = SplineUtils::controlPointsAlongLine(
      start_point, closest_point_1, start_direction, sharpness, cps_per_half);
  std::vector<Point> second_half = SplineUtils::controlPointsAlongLine(
      end_point, closest_point_2, end_direction, sharpness, cps_per_half, true);

  std::vector<Point> control_points;

  // put it all together
  for (const auto i : index_range(first_half))
    control_points.push_back(first_half[i]);
  for (const auto i : index_range(second_half))
    control_points.push_back(second_half[i]);

  return control_points;
}

std::vector<Point>
controlPointsAlongLine(const libMesh::Point & start_point,
                       const libMesh::Point & end_point,
                       const libMesh::RealVectorValue & direction_vector,
                       const libMesh::Real sharpness,
                       const unsigned int num_cps,
                       const bool reverse_order)
{
  // initialize a vector of control points with the starting point inserted
  std::vector<Point> control_points;

  mooseAssert(num_cps != 0, "Number of control points cannot be zero!");
  if (num_cps == 1)
    control_points.push_back(start_point);

  // create the control points by varying the sharpness linearly
  else
  {
    for (const auto i : libMesh::make_range(num_cps))
    {
      const auto point_sharpness = (Real)i / (Real)(num_cps - 1) * sharpness;
      control_points.push_back(
          SplineUtils::makeControlPoint(start_point, end_point, direction_vector, point_sharpness));
    }

    // reverse_order parameter only set to true when required
    if (reverse_order)
      std::reverse(control_points.begin(), control_points.end());
  }
  return control_points;
}

libMesh::Point
makeControlPoint(const libMesh::Point & start_point,
                 const libMesh::Point & end_point,
                 const libMesh::RealVectorValue & direction_vector,
                 const libMesh::Real sharpness)
{
  // check that sharpness value is ok
  mooseAssert(sharpness <= 1.0 && sharpness >= 0, "Sharpness must be in [0,1]!");

  // normalize direction vector
  const auto unit_direction = direction_vector.unit();

  // calculate the distance between points
  const auto distance = (end_point - start_point).norm();
  const auto distance_sq = distance * distance;

  // initialize return quantity and attempt to calculate control vector
  libMesh::Point control_point = start_point + sharpness * distance * unit_direction;

  // check if we went the wrong way, further from the end_point
  if ((end_point - control_point).norm_sq() > distance_sq)
  {
    control_point = start_point - sharpness * distance * unit_direction;
    mooseAssert((end_point - control_point).norm_sq() <= distance_sq,
                "Also going further from end point");
  }

  return control_point;
}

std::pair<Point, Point>
closestPoints(const libMesh::Point & point_1,
              const libMesh::Point & point_2,
              const libMesh::RealVectorValue & direction_1,
              const libMesh::RealVectorValue & direction_2)
{
  const auto n_vec = direction_1.cross(direction_2);

  // check if n_vec is the zero vector (indicates parallel lines)
  if (n_vec.norm_sq() < libMesh::TOLERANCE)
    mooseError("Lines are parallel! Infinitely many closest points exist.");

  const auto n1_vec = direction_1.cross(n_vec);
  const auto n2_vec = direction_2.cross(n_vec);

  // calculate closest points
  libMesh::Point point_a =
      point_1 + (point_2 - point_1) * n2_vec / (direction_1 * (n2_vec)) * direction_1;
  libMesh::Point point_b =
      point_2 + (point_1 - point_2) * n1_vec / (direction_2 * (n1_vec)) * direction_2;

  // points will be returned as a vector containing two points
  const auto closest_points = std::make_pair(point_a, point_b);

  return closest_points;
}
}
