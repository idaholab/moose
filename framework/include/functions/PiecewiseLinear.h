//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PIECEWISELINEAR_H
#define PIECEWISELINEAR_H

#include "Piecewise.h"

// Forward declarations
class PiecewiseLinear;

template <>
InputParameters validParams<PiecewiseLinear>();

/**
 * Function which provides a piecewise continuous linear interpolation
 * of a provided (x,y) point data set.
 */
class PiecewiseLinear : public Piecewise
{
public:
  PiecewiseLinear(const InputParameters & parameters);

  /**
   * Get the value of the function (based on time only)
   * \param t The time
   * \param pt The point in space (x,y,z) (unused)
   * \return The value of the function at the specified time
   */
  virtual Real value(Real t, const Point & pt) override;

  /**
   * Get the time derivative of the function (based on time only)
   * \param t The time
   * \param pt The point in space (x,y,z) (unused)
   * \return The time derivative of the function at the specified time
   */
  virtual Real timeDerivative(Real t, const Point & pt) override;

  virtual Real integral() override;

  virtual Real average() override;
};

#endif
