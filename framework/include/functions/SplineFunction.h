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

#ifndef SPLINEFUNCTION_H
#define SPLINEFUNCTION_H

#include "Function.h"
#include "SplineInterpolation.h"

class SplineFunction;

template<>
InputParameters validParams<SplineFunction>();

/**
 * Function that uses spline interpolation
 */
class SplineFunction : public Function
{
public:
  SplineFunction(const std::string & name, InputParameters parameters);
  virtual ~SplineFunction();

  virtual Real value(Real t, const Point & p);
  virtual Real derivative(const Point & p);
  virtual Real secondDerivative(const Point & p);

protected:
  SplineInterpolation _ipol;
};


#endif /* SPLINEFUNCTION_H */
