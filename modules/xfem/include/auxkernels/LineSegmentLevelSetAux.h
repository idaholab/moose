//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef LINESEGMENTLEVELSETAUX_H
#define LINESEGMENTLEVELSETAUX_H

#include "AuxKernel.h"

// Forward Declarations
class LineSegmentLevelSetAux;
class LineSegmentCutSetUserObject;

template <>
InputParameters validParams<LineSegmentLevelSetAux>();

/**
 * Calculate level set values for an interface that is defined by a set of line segments
 */
class LineSegmentLevelSetAux : public AuxKernel
{
public:
  LineSegmentLevelSetAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;
  virtual void compute() override;

  /**
   * calculate the signed distance value for a given point.
   * @param p Coordinate of point
   * @return Signed distance
   */
  Real calculateSignedDistance(Point p);

  /// Pointer to the LineSegmentCutSetUserObject object
  const LineSegmentCutSetUserObject * _linesegment_uo;

  /// Store the cut locations
  std::vector<Real> _cut_data;
};

#endif // LINESEGMENTLEVELSETAUX_H
