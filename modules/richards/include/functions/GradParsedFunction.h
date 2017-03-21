/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef GRADPARSEDFUNCTION_H
#define GRADPARSEDFUNCTION_H

#include "MooseParsedFunction.h"

// Forward declarations
class GradParsedFunction;

template <>
InputParameters validParams<GradParsedFunction>();

/**
 * returns the central difference approx to the derivative
 * of the function, ie
 * (f(t, p + direction) - f(t, p - direction))/2/|direction|
 * This derives from MooseParsedFunction, so it already knows about a function
 */
class GradParsedFunction : public MooseParsedFunction
{
public:
  GradParsedFunction(const InputParameters & parameters);

  virtual Real value(Real t, const Point & pt);

protected:
  /// central difference direction
  RealVectorValue _direction;

  /// 2*|_direction|
  Real _len;
};
#endif // GRADPARSEDFUNCTION_H
