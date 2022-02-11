//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADSmoothTransition.h"

/**
 * Cubic polynomial transition between two functions of one variable
 */
class ADCubicTransition : public ADSmoothTransition
{
public:
  /**
   * Constructor.
   *
   * @param[in] x_center   Center point of transition
   * @param[in] transition_width   Width of transition
   */
  ADCubicTransition(const ADReal & x_center, const ADReal & transition_width);

  virtual ADReal value(const ADReal & x, const ADReal & f1, const ADReal & f2) const override;

  /**
   * Computes the derivative of the transition value
   *
   * @param[in] x   Point at which to evaluate transition
   * @param[in] df1dx   First function derivative
   * @param[in] df2dx   Second function derivative
   */
  void initialize(const ADReal & f1_end_value,
                  const ADReal & f2_end_value,
                  const ADReal & df1dx_end_value,
                  const ADReal & df2dx_end_value);

protected:
  // Polynomial coefficients
  ADReal _A;
  ADReal _B;
  ADReal _C;
  ADReal _D;

  /// Flag that transition has been initialized
  bool _initialized;
};
