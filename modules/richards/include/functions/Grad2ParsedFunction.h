/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef GRAD2PARSEDFUNCTION_H
#define GRAD2PARSEDFUNCTION_H

#include "MooseParsedFunction.h"

// Forward declarations
class Grad2ParsedFunction;

template <>
InputParameters validParams<Grad2ParsedFunction>();

/**
 * returns the central difference approx to the derivative (direction.nabla)^2 function
 * viz
 * (f(t, p + direction) - 2*f(t, p) + f(t, p - direction))/|direction|^2
 * This derives from MooseParsedFunction, so it already knows about a function
 */
class Grad2ParsedFunction : public MooseParsedFunction
{
public:
  Grad2ParsedFunction(const InputParameters & parameters);

  virtual Real value(Real t, const Point & pt);

protected:
  /// central difference direction
  RealVectorValue _direction;

  /// |_direction|^2
  Real _len2;
};
#endif // GRAD2PARSEDFUNCTION_H
