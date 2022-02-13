//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SmoothTransitionFunction.h"
#include "CubicTransition.h"

/**
 * Computes a cubic polynomial transition between two functions
 *
 * Currently it is assumed that the direction of the transition is in time or in
 * the x, y, z direction, but this could later be extended to an arbitrary
 * direction.
 */
class CubicTransitionFunction : public SmoothTransitionFunction
{
public:
  CubicTransitionFunction(const InputParameters & parameters);

  using Function::value;
  virtual Real value(Real t, const Point & p) const;
  virtual RealVectorValue gradient(Real t, const Point & p) const;

protected:
  /// Derivative of first function at end point
  const Real & _df1dx_end_point;
  /// Derivative of second function at end point
  const Real & _df2dx_end_point;

  /// Transition object
  CubicTransition _transition;

public:
  static InputParameters validParams();
};
