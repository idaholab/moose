/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef BICUBICSPLINEFUNCTION_H
#define BICUBICSPLINEFUNCTION_H

#include "Function.h"
#include "BicubicSplineInterpolation.h"

class BicubicSplineFunction;

template<>
InputParameters validParams<BicubicSplineFunction>();

/**
 * Function that uses spline interpolation
 */
class BicubicSplineFunction : public Function
{
public:
  BicubicSplineFunction(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p) override;

  virtual Real derivative(const Point & p, unsigned int deriv_var);
  virtual Real secondDerivative(const Point & p, unsigned int deriv_var);

protected:
  BicubicSplineInterpolation _ipol;
};


#endif /* BICUBICSPLINEFUNCTION_H */
