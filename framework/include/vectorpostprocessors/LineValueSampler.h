//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "PointSamplerBase.h"

class LineValueSampler : public PointSamplerBase
{
public:
  static InputParameters validParams();

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

  /**
   * Gets the value of the variable at a point p.
   * Used with MultiAppUserObjectTransfer to transfer
   * variable information from one domain to another.
   **/
  virtual Real spatialValue(const Point & p) const override { return getValue(p); }

  /**
   * Gets the value of the variable at a point p.
   * Returns zero if p does not lie along the line segment.
   **/
  Real getValue(const Point & p) const;

protected:
  const Point _start_point;
  const Point _end_point;

  unsigned int & _num_points;

  /// Vector connecting the start and end points of the line segment
  const RealVectorValue _line_vector;

  /// Zero vector
  const RealVectorValue _zero;

  /// Length of line segment
  const Real _line_vector_norm;

private:
  const VectorPostprocessorValue & _vpp_value;
};
