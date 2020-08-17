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

// Forward declarations
class PiecewiseLinearBase;

template <>
InputParameters validParams<PiecewiseLinearBase>();

/**
 * Base class for functions which provides a piecewise continuous linear
 * interpolation of an (x,y) point data set.
 */
class PiecewiseLinearBase : public PiecewiseTabularBase
{
public:
  static InputParameters validParams();

  PiecewiseLinearBase(const InputParameters & parameters);

  void initialSetup() override;
  Real value(Real t, const Point & p) const override;
  Real timeDerivative(Real t, const Point &) const override;
  RealGradient gradient(Real, const Point & p) const override;
  Real integral() const override;
  Real average() const override;
  void setData(const std::vector<Real> & x, const std::vector<Real> & y) override;

protected:
  /// build the linear interpolation object from the x/y data
  void buildInterpolation();

  /// helper object to perform the linear interpolation of the function data
  std::unique_ptr<LinearInterpolation> _linear_interp;
};
