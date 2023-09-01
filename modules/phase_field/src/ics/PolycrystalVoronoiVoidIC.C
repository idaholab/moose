//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolycrystalVoronoiVoidIC.h"

// MOOSE includes
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "DelimitedFileReader.h"
#include "GrainTrackerInterface.h"
#include "PolycrystalVoronoi.h"

registerMooseObject("PhaseFieldApp", PolycrystalVoronoiVoidIC);

InputParameters
PolycrystalVoronoiVoidIC::validParams()
{
  InputParameters params = MultiSmoothCircleIC::validParams();
  params.addClassDescription("Random distribution of smooth circles at grain boundaries "
                             "given minimum spacing");
  params.addRequiredParam<unsigned int>("op_num", "Number of order parameters");
  params.addRequiredParam<UserObjectName>(
      "polycrystal_ic_uo", "UserObject for obtaining the polycrystal grain structure.");
  params.addParam<FileName>("file_name",
                            "",
                            "File containing grain centroids, if file_name is provided, "
                            "the centroids from the file will be used.");
  return params;
}

PolycrystalVoronoiVoidIC::PolycrystalVoronoiVoidIC(const InputParameters & parameters)
  : MultiSmoothCircleIC(parameters),
    _op_num(getParam<unsigned int>("op_num")),
    _poly_ic_uo(getUserObject<PolycrystalVoronoi>("polycrystal_ic_uo")),
    _file_name(getParam<FileName>("file_name"))
{
  if (_invalue < _outvalue)
    mooseWarning("Detected invalue < outvalue in PolycrystalVoronoiVoidIC. Please make sure that's "
                 "the intended usage for representing voids.");
  if (_numbub == 0)
    mooseError("PolycrystalVoronoiVoidIC requires numbub > 0. If you want no voids to "
               "be represented, use invalue = outvalue. In general, you should use "
               "PolycrystalVoronoi to represent Voronoi grain structures without "
               "voids.");
}

void
PolycrystalVoronoiVoidIC::initialSetup()
{
  // Obtain total number and centerpoints of the grains
  _grain_num = _poly_ic_uo.getNumGrains();
  _centerpoints = _poly_ic_uo.getGrainCenters();

  // Call initial setup from MultiSmoothCircleIC to create _centers and _radii
  // for voids
  MultiSmoothCircleIC::initialSetup();
}

void
PolycrystalVoronoiVoidIC::computeCircleCenters()
{
  _centers.resize(_numbub);

  // This Code will place void center points on grain boundaries
  for (unsigned int vp = 0; vp < _numbub; ++vp)
  {
    bool try_again;
    unsigned int num_tries = 0;

    do
    {
      try_again = false;
      num_tries++;

      if (num_tries > _max_num_tries)
        mooseError("Too many tries of assigning void centers in "
                   "PolycrystalVoronoiVoidIC");

      Point rand_point;

      for (const auto i : make_range(Moose::dim))
        rand_point(i) = _bottom_left(i) + _range(i) * _random.rand(_tid);

      // Allow the vectors to be sorted based on their distance from the
      // rand_point
      std::vector<PolycrystalVoronoiVoidIC::DistancePoint> diff(_grain_num);

      for (unsigned int gr = 0; gr < _grain_num; ++gr)
      {
        diff[gr].d = _mesh.minPeriodicDistance(_var.number(), rand_point, _centerpoints[gr]);
        diff[gr].gr = gr;
      }

      std::sort(diff.begin(), diff.end(), _customLess);

      Point closest_point = _centerpoints[diff[0].gr];
      Point next_closest_point = _centerpoints[diff[1].gr];

      // Find Slope of Line in the plane orthogonal to the diff_centerpoint
      // vector
      Point pa = rand_point + _mesh.minPeriodicVector(_var.number(), rand_point, closest_point);
      Point pb =
          rand_point + _mesh.minPeriodicVector(_var.number(), rand_point, next_closest_point);
      Point diff_centerpoints = pb - pa;

      Point diff_rand_center = _mesh.minPeriodicVector(_var.number(), closest_point, rand_point);
      Point normal_vector = diff_centerpoints.cross(diff_rand_center);
      Point slope = normal_vector.cross(diff_centerpoints);

      // Midpoint position vector between two center points
      Point midpoint = closest_point + (0.5 * diff_centerpoints);

      // Solve for the scalar multiplier solution on the line
      Real lambda = 0;
      Point mid_rand_vector = _mesh.minPeriodicVector(_var.number(), midpoint, rand_point);

      Real slope_dot = slope * slope;
      mooseAssert(slope_dot > 0, "The dot product of slope with itself is zero");
      for (const auto i : make_range(Moose::dim))
        lambda += (mid_rand_vector(i) * slope(i)) / slope_dot;

      // Assigning points to vector
      _centers[vp] = slope * lambda + midpoint;

      // Checking to see if points are in the domain ONLY WORKS FOR PERIODIC
      for (const auto i : make_range(Moose::dim))
        if ((_centers[vp](i) > _top_right(i)) || (_centers[vp](i) < _bottom_left(i)))
          try_again = true;

      for (unsigned int i = 0; i < vp; ++i)
      {
        Real dist = _mesh.minPeriodicDistance(_var.number(), _centers[vp], _centers[i]);

        if (dist < _bubspac)
          try_again = true;
      }

      // Two algorithms are available for screening bubbles falling in grain
      // interior. They produce
      // nearly identical results.
      // Here only one is listed. The other one is available upon request.

      // Use circle center for checking whether voids are at GBs
      if (try_again == false)
      {
        Real min_rij_1, min_rij_2, rij, rij_diff_tol;

        min_rij_1 = _range.norm();
        min_rij_2 = _range.norm();

        rij_diff_tol = 0.1 * _radius;

        for (unsigned int gr = 0; gr < _grain_num; ++gr)
        {
          rij = _mesh.minPeriodicDistance(_var.number(), _centers[vp], _centerpoints[gr]);

          if (rij < min_rij_1)
          {
            min_rij_2 = min_rij_1;
            min_rij_1 = rij;
          }
          else if (rij < min_rij_2)
            min_rij_2 = rij;
        }

        if (std::abs(min_rij_1 - min_rij_2) > rij_diff_tol)
          try_again = true;
      }

    } while (try_again == true);
  }
}

Real
PolycrystalVoronoiVoidIC::value(const Point & p)
{
  Real value = 0.0;

  // Determine value for voids
  Real void_value = MultiSmoothCircleIC::value(p);

  value = void_value;

  return value;
}

RealGradient
PolycrystalVoronoiVoidIC::gradient(const Point & p)
{
  RealGradient gradient;
  RealGradient void_gradient = MultiSmoothCircleIC::gradient(p);

  // Order parameter assignment assumes zero gradient (sharp interface)
  // assigning gradient for voids
  gradient = void_gradient;

  return gradient;
}
