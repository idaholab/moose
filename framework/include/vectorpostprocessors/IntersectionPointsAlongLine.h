//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"

/**
 * Get the intersection points for all of the elements that are intersected by a line
 */
class IntersectionPointsAlongLine : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  IntersectionPointsAlongLine(const InputParameters & parameters);

  virtual void initialize() override;

  /**
   * Find the elements
   */
  virtual void execute() override;

protected:
  /// The beginning of the line
  Point _start;

  /// The end of the line
  Point _end;

  /// The elements that intersect the line
  VectorPostprocessorValue & _x_intersections;
  VectorPostprocessorValue & _y_intersections;
  VectorPostprocessorValue & _z_intersections;

  /// Tie them together for convenience
  std::vector<VectorPostprocessorValue *> _intersections;
};
