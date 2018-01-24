//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef BICUBICSPLINEFUNCTION_H
#define BICUBICSPLINEFUNCTION_H

#include "Function.h"
#include "FunctionInterface.h"
#include "BicubicSplineInterpolation.h"

class BicubicSplineFunction;

template <>
InputParameters validParams<BicubicSplineFunction>();

/**
 * Function that uses spline interpolation
 */
class BicubicSplineFunction : public Function, public FunctionInterface
{
public:
  BicubicSplineFunction(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p) override;

  virtual Real derivative(const Point & p, unsigned int deriv_var);
  virtual RealGradient gradient(Real t, const Point & p) override;
  virtual Real secondDerivative(const Point & p, unsigned int deriv_var);

protected:
  BicubicSplineInterpolation _ipol;

  std::vector<Real> _x1;
  std::vector<Real> _x2;
  std::vector<Real> _yx11;
  std::vector<Real> _yx1n;
  std::vector<Real> _yx21;
  std::vector<Real> _yx2n;

  Function & _yx1;
  Function & _yx2;
};

#endif /* BICUBICSPLINEFUNCTION_H */
