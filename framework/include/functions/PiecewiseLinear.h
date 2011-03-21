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

#ifndef PIECEWISELINEAR_H
#define PIECEWISELINEAR_H

#include "Function.h"
#include "LinearInterpolation.h"

class PiecewiseLinear;

template<>
InputParameters validParams<PiecewiseLinear>();

/**
 * Base class for function objects.  Functions override value to supply a
 * value at a point.
 */
class PiecewiseLinear : public Function
{
public:
  PiecewiseLinear(const std::string & name, InputParameters parameters);
  virtual ~PiecewiseLinear();

  /**
   * This function will return a value based on the first input argument only.
   */
  virtual Real value(Real t, const Point & pt);

private:
  LinearInterpolation _linear_interp;
  Real _scale_factor;

};

#endif //PIECEWISELINEAR_H
