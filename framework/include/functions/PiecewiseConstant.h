//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PiecewiseTabularBase.h"

/**
 * Function which provides a piecewise constant interpolation of a provided (x,y) point data set.
 */
class PiecewiseConstant : public PiecewiseTabularBase
{
public:
  static InputParameters validParams();

  PiecewiseConstant(const InputParameters & parameters);

  using Function::value;
  /**
   * Get the value of the function (based on time only)
   * \param t The time
   * \param pt The point in space (x,y,z) (unused)
   * \return The value of the function at the specified time
   */
  virtual Real value(Real t, const Point & p) const override;
  virtual ADReal value(const ADReal & t, const ADPoint & p) const override;

  /**
   * Get the time derivative of the function (based on time only)
   * \param t The time
   * \param pt The point in space (x,y,z) (unused)
   * \return The time derivative of the function at the specified time
   */
  virtual Real timeDerivative(Real t, const Point & pt) const override;
  virtual Real integral() const override;
  virtual Real average() const override;

private:
  /// Enum for which direction to apply values
  const enum class Direction { LEFT, RIGHT, LEFT_INCLUSIVE, RIGHT_INCLUSIVE } _direction;
};
