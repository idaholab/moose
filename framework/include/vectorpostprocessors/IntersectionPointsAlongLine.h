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

#ifndef INTERSECTIONPOINTSALONGLINE_H
#define INTERSECTIONPOINTSALONGLINE_H

#include "GeneralVectorPostprocessor.h"

// Forward Declarations
class IntersectionPointsAlongLine;

template <>
InputParameters validParams<IntersectionPointsAlongLine>();

/**
 * Get the intersection points for all of the elements that are intersected by a line
 */
class IntersectionPointsAlongLine : public GeneralVectorPostprocessor
{
public:
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

#endif
