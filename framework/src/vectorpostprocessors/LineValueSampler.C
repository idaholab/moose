/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "LineValueSampler.h"

template <>
InputParameters
validParams<LineValueSampler>()
{
  InputParameters params = validParams<PointSamplerBase>();

  params.addRequiredParam<Point>("start_point", "The beginning of the line");
  params.addRequiredParam<Point>("end_point", "The ending of the line");

  params.addRequiredParam<unsigned int>("num_points",
                                        "The number of points to sample along the line");

  return params;
}

LineValueSampler::LineValueSampler(const InputParameters & parameters)
  : PointSamplerBase(parameters)
{
  Point start_point = getParam<Point>("start_point");
  Point end_point = getParam<Point>("end_point");

  unsigned int num_points = getParam<unsigned int>("num_points");

  generatePointsAndIDs(start_point, end_point, num_points, _points, _ids);
}

void
LineValueSampler::generatePointsAndIDs(const Point & start_point,
                                       const Point & end_point,
                                       unsigned int num_points,
                                       std::vector<Point> & points,
                                       std::vector<Real> & ids)
{

  Point difference = end_point - start_point;

  Point delta = difference / Real(num_points - 1);

  points.resize(num_points);
  ids.resize(num_points);

  for (unsigned int i = 0; i < num_points - 1;
       i++) // -1 so that we can manually put in the end point to get it perfect
  {
    Point p = start_point + (i * delta);

    points[i] = p;
    ids[i] = (p - start_point).norm(); // The ID is the distance along the line
  }

  // Add the end point explicitly
  points[num_points - 1] = end_point;
  ids[num_points - 1] = (end_point - start_point).norm();
}
