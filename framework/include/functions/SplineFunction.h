//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Function.h"
#include "SplineInterpolation.h"

/**
 * Function that uses spline interpolation
 */
class SplineFunction : public Function
{
public:
  static InputParameters validParams();

  SplineFunction(const InputParameters & parameters);

  using Function::value;
  virtual Real value(Real t, const Point & p) const override;
  virtual ADReal value(const ADReal & t, const ADPoint & p) const override;

  virtual RealGradient gradient(Real t, const Point & p) const override;

  virtual Real derivative(const Point & p) const;
  virtual Real secondDerivative(const Point & p) const;

protected:
  SplineInterpolation _ipol;

  /// Desired component
  int _component;
};
