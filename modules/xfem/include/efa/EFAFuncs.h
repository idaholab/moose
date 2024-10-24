//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <map>
#include <set>
#include <vector>
#include <algorithm>

namespace Efa
{
static const double tol = 1.0e-10;

template <typename T>
bool
deleteFromMap(std::map<unsigned int, T *> & theMap, T * elemToDelete, bool delete_elem = true)
{
  bool didIt = false;
  typename std::map<unsigned int, T *>::iterator i = theMap.find(elemToDelete->id());
  if (i != theMap.end())
  {
    if (delete_elem)
      delete i->second;
    theMap.erase(i);
    didIt = true;
  }
  return didIt;
}

template <typename T>
unsigned int
getNewID(std::map<unsigned int, T *> & theMap)
{
  typename std::map<unsigned int, T *>::reverse_iterator last_elem = theMap.rbegin();
  unsigned int new_elem_id = 0;
  if (last_elem != theMap.rend())
    new_elem_id = last_elem->first + 1;
  return new_elem_id;
}

template <class T>
unsigned int
numCommonElems(std::set<T> & v1, std::set<T> & v2)
{
  std::vector<T> common_elems;
  std::set_intersection(
      v1.begin(), v1.end(), v2.begin(), v2.end(), std::inserter(common_elems, common_elems.end()));
  return common_elems.size();
}

template <class T>
unsigned int
numCommonElems(std::set<T> & v1, std::vector<T> & v2)
{
  std::vector<T> common_elems;
  std::set_intersection(
      v1.begin(), v1.end(), v2.begin(), v2.end(), std::inserter(common_elems, common_elems.end()));
  return common_elems.size();
}

template <class T>
std::vector<T>
getCommonElems(std::set<T> & v1, std::set<T> & v2)
{
  std::vector<T> common_elems;
  std::set_intersection(
      v1.begin(), v1.end(), v2.begin(), v2.end(), std::inserter(common_elems, common_elems.end()));
  return common_elems;
}

double linearQuadShape2D(unsigned int node_id, std::vector<double> & xi_2d);

double linearTriShape2D(unsigned int node_id, std::vector<double> & xi_2d);

double linearHexShape3D(unsigned int node_id, std::vector<double> & xi_3d);

double linearTetShape3D(unsigned int node_id, std::vector<double> & xi_3d);

/**
 * Determine the intersection point between two 2D line segments specified
 * by the coordinates of their end points.
 * @param segment1_point1 coordinates of point 1 of line segment 1
 * @param segment1_point2 coordinates of point 2 of line segment 1
 * @param segment2_point1 coordinates of point 1 of line segment 2
 * @param segment2_point2 coordinates of point 2 of line segment 2
 * @param intersect_point computed coordinates of intersection point if it exists
 * @return bool indicating if line segments intersect
 */
bool IntersectionPointTwoLineSegments2D(const std::vector<double> & segment1_point1,
                                        const std::vector<double> & segment1_point2,
                                        const std::vector<double> & segment2_point1,
                                        const std::vector<double> & segment2_point2,
                                        std::vector<double> & intersect_point);

/**
 * Determine the cross product of two 2D vectors
 * @param two length vector with coordinates of point a
 * @param two length vector with coordinates of point b
 * @return double of cross product
 */
double crossProduct2D(const std::vector<double> & point_a, const std::vector<double> & point_b);

/**
 * Determine the distance between two points
 * @param two length vector with coordinates of point a
 * @param two length vector with coordinates of point b
 * @return double of distance between points a and b
 */
double distanceBetweenPoints2D(const std::vector<double> & point_a,
                               const std::vector<double> & point_b);

} // namespace Efa
