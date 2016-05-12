/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PolycrystalICTools.h"
#include "MooseMesh.h"

std::vector<unsigned int>
PolycrystalICTools::assignPointsToVariables(const std::vector<Point> & centerpoints, const Real op_num, const MooseMesh & mesh, const MooseVariable & var)
{
  Real grain_num = centerpoints.size();

  std::vector<unsigned int> assigned_op(grain_num);
  std::vector<int> min_op_ind(op_num);
  std::vector<Real> min_op_dist(op_num);

  //Assign grains to specific order parameters in a way that maximizes the distance
  for (unsigned int grain = 0; grain < grain_num; grain++)
  {
    // Determine the distance to the closest center assigned to each order parameter
    if (grain >= op_num)
    {
      // We can set the array to the distances to the grains 0..op_num-1 (see assignment in the else case)
      for (unsigned int i=0; i<op_num; ++i)
      {
        min_op_dist[i] = mesh.minPeriodicDistance(var.number(), centerpoints[grain], centerpoints[i]);
        min_op_ind[assigned_op[i]] = i;
      }

      // Now check if any of the extra grains are even closer
      for (unsigned int i=op_num; i<grain; ++i)
      {
        Real dist = mesh.minPeriodicDistance(var.number(), centerpoints[grain], centerpoints[i]);
        if (min_op_dist[assigned_op[i]] > dist)
        {
          min_op_dist[assigned_op[i]] = dist;
          min_op_ind[assigned_op[i]] = i;
        }
      }
    }
    else
    {
      assigned_op[grain] = grain;
      continue;
    }

    // Assign the current center point to the order parameter that is furthest away.
    unsigned int mx_ind = 0;
    for (unsigned int i = 1; i < op_num; i++) // Find index of max
      if (min_op_dist[mx_ind] < min_op_dist[i])
        mx_ind = i;

    assigned_op[grain] = mx_ind;
  }

  return assigned_op;
}

unsigned int
PolycrystalICTools::assignPointToGrain(const Point & p, const std::vector<Point> & centerpoints, const MooseMesh & mesh, const MooseVariable & var, const Real maxsize)
{
  unsigned int grain_num = centerpoints.size();

  Real min_distance = maxsize;
  unsigned int min_index = grain_num;
  //Loops through all of the grain centers and finds the center that is closest to the point p
  for (unsigned int grain = 0; grain < grain_num; grain++)
  {
    Real distance = mesh.minPeriodicDistance(var.number(), centerpoints[grain], p);

    if (min_distance > distance)
    {
      min_distance = distance;
      min_index = grain;
    }
  }

  if (min_index >= grain_num)
    mooseError("ERROR in PolycrystalVoronoiVoidIC: didn't find minimum values in grain_value_calc");

  return min_index;
}
