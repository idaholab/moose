//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef LINEVALUESAMPLER_H
#define LINEVALUESAMPLER_H

// MOOSE includes
#include "PointSamplerBase.h"

// Forward Declarations
class LineValueSampler;

template <>
InputParameters validParams<LineValueSampler>();

class LineValueSampler : public PointSamplerBase
{
public:
  LineValueSampler(const InputParameters & parameters);

  /**
   * Helper function to generate the list of points along a line and a unique ID for each point.
   * @param start_point The beginning of the line
   * @param end_point The end of the line
   * @param num_points The number of points along the line
   * @param points The vector of points to fill in
   * @param ids The vector of ids to fill in
   */
  static void generatePointsAndIDs(const Point & start_point,
                                   const Point & end_point,
                                   unsigned int num_points,
                                   std::vector<Point> & points,
                                   std::vector<Real> & ids);
};

#endif
