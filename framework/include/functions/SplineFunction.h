//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPLINEFUNCTION_H
#define SPLINEFUNCTION_H

#include "Function.h"
#include "SplineInterpolation.h"

class SplineFunction;

template <>
InputParameters validParams<SplineFunction>();

/**
 * Function that uses spline interpolation
 */
class SplineFunction : public Function
{
public:
  SplineFunction(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p) override;
  virtual RealGradient gradient(Real t, const Point & p) override;

  virtual Real derivative(const Point & p);
  virtual Real secondDerivative(const Point & p);

protected:
  SplineInterpolation _ipol;

  /// Desired component
  int _component;
};

#endif /* SPLINEFUNCTION_H */
