//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

/**
 * Base class for smooth transitions between two functions of one variable
 */
class ADSmoothTransition
{
public:
  /**
   * Constructor.
   *
   * @param[in] x_center   Center point of transition
   * @param[in] transition_width   Width of transition
   */
  ADSmoothTransition(const ADReal & x_center, const ADReal & transition_width);

  /**
   * Computes the transition value
   *
   * @param[in] x    Point at which to evaluate function
   * @param[in] f1   Left function
   * @param[in] f2   Right function
   */
  virtual ADReal value(const ADReal & x, const ADReal & f1, const ADReal & f2) const = 0;

  /**
   * Returns the coordinate of the left end of the transition
   */
  const ADReal & leftEnd() const { return _x1; }

  /**
   * Returns the coordinate of the right end of the transition
   */
  const ADReal & rightEnd() const { return _x2; }

protected:
  /// Center point of transition
  const ADReal _x_center;
  /// Width of transition
  const ADReal _transition_width;

  /// Left end point of transition
  const ADReal _x1;
  /// Right end point of transition
  const ADReal _x2;
};
