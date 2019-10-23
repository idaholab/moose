//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PiecewiseBase.h"

// Forward declarations
class PiecewiseLinearBase;

template <>
InputParameters validParams<PiecewiseLinearBase>();

/**
 * Function which provides a piecewise continuous linear interpolation
 * of a provided (x,y) point data set.
 */
class PiecewiseLinearBase : public PiecewiseBase
{
public:
  PiecewiseLinearBase(const InputParameters & parameters);

  virtual void initialSetup() override;

  /**
   * Get the value of the function (based on time only)
   * \param t The time
   * \param pt The point in space (x,y,z) (unused)
   * \return The value of the function at the specified time
   */
  virtual Real value(Real t, const Point & pt) const override;

  /**
   * Get the time derivative of the function (based on time only)
   * \param t The time
   * \param pt The point in space (x,y,z) (unused)
   * \return The time derivative of the function at the specified time
   */
  virtual Real timeDerivative(Real t, const Point & pt) const override;

  virtual Real integral() const override;

  virtual Real average() const override;

protected:
  /// build the linear interpolation object from the x/y data
  void buildInterpolation();

  /// helper object to perform the linear interpolation of the function data
  std::unique_ptr<LinearInterpolation> _linear_interp;
};
