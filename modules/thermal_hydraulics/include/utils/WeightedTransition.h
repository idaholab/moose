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
 * Weighted transition between two functions of one variable
 */
class WeightedTransition : public SmoothTransition
{
public:
  /**
   * Constructor.
   *
   * @param[in] x_center   Center point of transition
   * @param[in] transition_width   Width of transition
   */
  WeightedTransition(const Real & x_center, const Real & transition_width);

  virtual Real value(const Real & x, const Real & f1, const Real & f2) const override;

  /**
   * Computes the derivative of the transition value
   *
   * @param[in] x   Point at which to evaluate transition
   * @param[in] f1   First function value
   * @param[in] f2   Second function value
   * @param[in] df1dx   First function derivative
   * @param[in] df2dx   Second function derivative
   */
  Real derivative(const Real & x,
                  const Real & f1,
                  const Real & f2,
                  const Real & df1dx,
                  const Real & df2dx) const;

  /**
   * Computes the weight of the first function
   *
   * @param[in] x   Point at which to evaluate weight
   */
  Real weight(const Real & x) const;
};
