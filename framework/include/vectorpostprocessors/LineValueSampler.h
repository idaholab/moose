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
