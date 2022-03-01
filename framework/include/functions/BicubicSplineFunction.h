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
#include "FunctionInterface.h"
#include "BicubicSplineInterpolation.h"

/**
 * Function that uses spline interpolation
 */
class BicubicSplineFunction : public Function, public FunctionInterface
{
public:
  static InputParameters validParams();

  BicubicSplineFunction(const InputParameters & parameters);

  using Function::value;
  virtual Real value(Real t, const Point & p) const override;

  virtual Real derivative(const Point & p, unsigned int deriv_var) const;
  virtual RealGradient gradient(Real t, const Point & p) const override;
  virtual Real secondDerivative(const Point & p, unsigned int deriv_var) const;

protected:
  mutable BicubicSplineInterpolation _ipol;

  // Desired normal direction
  unsigned int _normal_component;

  std::vector<Real> _x1;
  std::vector<Real> _x2;
  std::vector<Real> _yx11;
  std::vector<Real> _yx1n;
  std::vector<Real> _yx21;
  std::vector<Real> _yx2n;

  const Function & _yx1;
  const Function & _yx2;

  // The xyz index that x1/x2 should map to
  Real _x1_index;
  Real _x2_index;
};
