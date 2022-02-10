//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SmoothTransition.h"

/**
 *  Cubic polynomial transition between two functions of one variable
 */
class CubicTransition : public SmoothTransition
{
public:
  /**
   * Constructor.
   *
   * @param[in] x_center   Center point of transition
   * @param[in] transition_width   Width of transition
   */
  CubicTransition(const Real & x_center, const Real & transition_width);

  virtual Real value(const Real & x, const Real & f1, const Real & f2) const override;

  /**
   * Computes the derivative of the transition value
   *
   * @param[in] x   Point at which to evaluate transition
   * @param[in] df1dx   First function derivative
   * @param[in] df2dx   Second function derivative
   */
  Real derivative(const Real & x, const Real & df1dx, const Real & df2dx) const;

  /**
   * Initializes the polynomial coefficients
   *
   * @param[in] f1_end_value   Value of left function at left transition end point
   * @param[in] f2_end_value   Value of right function at right transition end point
   * @param[in] df1dx_end_value   Value of left function derivative at left transition end point
   * @param[in] df2dx_end_value   Value of right function derivative at right transition end point
   */
  void initialize(const Real & f1_end_value,
                  const Real & f2_end_value,
                  const Real & df1dx_end_value,
                  const Real & df2dx_end_value);

protected:
  // Polynomial coefficients
  Real _A;
  Real _B;
  Real _C;
  Real _D;

  /// Flag that transition has been initialized
  bool _initialized;
};
