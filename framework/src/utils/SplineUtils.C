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
<<<<<<< HEAD
  /// establish the direction vector between the two points
  libMesh::RealVectorValue orthogonal_direction = start_point - end_point;

  /// circular control points will only honor the derivatives if the points are colinear along the orthogonal direction
=======
  // establish the direction vector between the two points
  libMesh::RealVectorValue orthogonal_direction = start_point - end_point;

  // circular control points will only honor the derivatives if the points are colinear along the
  // orthogonal direction
>>>>>>> BSplineGen-cleanup
  if (!(std::abs(parallel_direction * orthogonal_direction) < libMesh::TOLERANCE))
    mooseError("Parallel lines cannot be interpolated using points along a circle! Direction "
               "vectors are incompatible with point locations.");

<<<<<<< HEAD
  /// define the distance between the two points
  libMesh::Real distance_between = orthogonal_direction.norm();

  /// connecting circle is assumed to be between each point along the orthogonal_direction vector
  libMesh::Real radius = distance_between / 2.0;

  /// normalize vectors
  libMesh::RealVectorValue unit_normal = orthogonal_direction / distance_between;
  libMesh::RealVectorValue unit_parallel = parallel_direction / parallel_direction.norm();

  /// calculate circle center
  libMesh::Point center_point = end_point + radius * unit_normal;

  /// initialize vector of control points
  std::vector<Point> control_points;

  /// initialize parameter
  double t;

  /// loop over the number of control points to generate the control points
=======
  // define the distance between the two points
  libMesh::Real distance_between = orthogonal_direction.norm();

  // connecting circle is assumed to be between each point along the orthogonal_direction vector
  libMesh::Real radius = distance_between / 2.0;

  // normalize vectors
  libMesh::RealVectorValue unit_normal = orthogonal_direction / distance_between;
  libMesh::RealVectorValue unit_parallel = parallel_direction / parallel_direction.norm();

  // calculate circle center
  libMesh::Point center_point = end_point + radius * unit_normal;

  // initialize vector of control points
  std::vector<Point> control_points;

  // initialize parameter
  double t;

  // loop over the number of control points to generate the control points
>>>>>>> BSplineGen-cleanup
  for (const auto i : make_range(num_cps))
  {
    t = (double)i / (double)(num_cps - 1);
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
<<<<<<< HEAD
  ///
  /// start with a couple of checks to be sure the input is good
  ///
  /// check that start_point is different from end_point
  if ((start_point - end_point).norm() < libMesh::TOLERANCE)
    mooseError("Start and end points must be unique.");
  /// check that neither direction is the zero vector
=======
  //
  // start with a couple of checks to be sure the input is good
  //
  // check that start_point is different from end_point
  if ((start_point - end_point).norm() < libMesh::TOLERANCE)
    mooseError("Start and end points must be unique.");
  // check that neither direction is the zero vector
>>>>>>> BSplineGen-cleanup
  if (start_direction.norm() < libMesh::TOLERANCE || end_direction.norm() == 0)
    mooseError("Direction vectors must have a nonzero magnitude.");
  if (!(sharpness > 0 && sharpness <= 1))
    mooseError("Sharpness must be in (0,1].");

<<<<<<< HEAD
  /// check if directions are parallel
=======
  // check if directions are parallel
>>>>>>> BSplineGen-cleanup
  const bool parallel = ((start_direction.cross(end_direction)).norm() < libMesh::TOLERANCE);
  if (parallel)
  {
    mooseWarning("Directions are parallel! Attempting to use circular control points...");
    unsigned int num_cps = 2 * cps_per_half + 1;
    if (num_cps < 30)
      mooseWarning("Number of control points required for circular control points is much greater "
                   "than for BSplines. `num_cps` is now 30. Ensure this is acceptable!");
    return SplineUtils::circularControlPoints(start_point, end_point, start_direction, num_cps);
  }

<<<<<<< HEAD
  /// find closest points --> these will be identical if the extrapolated lines intersect
=======
  // find closest points --> these will be identical if the extrapolated lines intersect
>>>>>>> BSplineGen-cleanup
  std::vector<Point> closest_points_vector =
      SplineUtils::closestPoints(start_point, end_point, start_direction, end_direction);
  libMesh::Point closest_point_1 = closest_points_vector[0];
  libMesh::Point closest_point_2 = closest_points_vector[1];

  std::vector<Point> first_half = SplineUtils::controlPointsAlongLine(
      start_point, closest_point_1, start_direction, sharpness, cps_per_half);
  std::vector<Point> second_half = SplineUtils::controlPointsAlongLine(
      end_point, closest_point_2, end_direction, sharpness, cps_per_half, true);

  std::vector<Point> control_points;

<<<<<<< HEAD
  /// put it all together
=======
  // put it all together
>>>>>>> BSplineGen-cleanup
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
                       const bool build_backwards)
{
  // initialize a vector of control points with the starting point inserted
  std::vector<Point> control_points;

  // initialize a changing sharpness parameter
  double point_sharpness;

  mooseAssert(num_cps != 0, "Number of control points cannot be zero!");
  if (num_cps == 1)
    control_points.push_back(start_point);

  // create the control points by varying the sharpness linearly
  else
  {
    for (const auto i : libMesh::make_range(num_cps))
    {
      point_sharpness = (double)i / (double)(num_cps - 1) * sharpness;
      control_points.push_back(
          SplineUtils::makeControlPoint(start_point, end_point, direction_vector, point_sharpness));
    }

    // build_backwards parameter only set to true when required
    if (build_backwards)
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
  if (!(sharpness <= 1.0 && sharpness >= 0))
    mooseError("Sharpness must be in [0,1]!");

  // normalize direction vector
  libMesh::RealVectorValue unit_direction = direction_vector / direction_vector.norm();

  // calculate the distance between points
  double distance = (end_point - start_point).norm();

  // initialize return quantity
  libMesh::Point control_point;

  // attempt to calculate control vector
  control_point = start_point + sharpness * distance * unit_direction;

  // check if we went the wrong way...
  double end_point_distance_to_cp = (end_point - control_point).norm();
  if (end_point_distance_to_cp > distance)
    control_point = start_point - sharpness * distance * unit_direction;

  return control_point;
}

std::vector<Point>
closestPoints(const libMesh::Point & point_1,
              const libMesh::Point & point_2,
              const libMesh::RealVectorValue & direction_1,
              const libMesh::RealVectorValue & direction_2)
{
  libMesh::RealVectorValue n_vec = direction_1.cross(direction_2);

  // check if n_vec is the zero vector (indicates parallel lines)
  if (n_vec.norm() < libMesh::TOLERANCE)
    mooseError("Lines are parallel! Infinitely many closest points exist.");

  libMesh::RealVectorValue n1_vec = direction_1.cross(n_vec);
  libMesh::RealVectorValue n2_vec = direction_2.cross(n_vec);

  // calculate closest points
  // take absolute value to ensure specified directions are honored
  libMesh::Real point_a_dir_scalar =
      std::abs((point_2 - point_1) * n2_vec / (direction_1 * n2_vec));
  libMesh::Real point_b_dir_scalar =
      std::abs((point_1 - point_2) * n1_vec / (direction_2 * n1_vec));
  libMesh::Point point_a = point_1 + point_a_dir_scalar * direction_1;
  libMesh::Point point_b = point_2 + point_b_dir_scalar * direction_2;

  // points will be returned as a vector containing two points
  std::vector<Point> closest_points = {point_a, point_b};

  return closest_points;
}
}
