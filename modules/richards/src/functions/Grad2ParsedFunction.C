/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "Grad2ParsedFunction.h"
#include "MooseParsedFunctionWrapper.h"

template <>
InputParameters
validParams<Grad2ParsedFunction>()
{
  InputParameters params = validParams<MooseParsedFunction>();
  params += validParams<MooseParsedFunction>();
  params.addRequiredParam<RealVectorValue>(
      "direction",
      "The direction in which to take the derivative.  This must not be a zero-length "
      "vector.  This function returned a finite-difference approx to "
      "(direction.nabla)^2 function");
  return params;
}

Grad2ParsedFunction::Grad2ParsedFunction(const InputParameters & parameters)
  : MooseParsedFunction(parameters), _direction(getParam<RealVectorValue>("direction"))
{
  _len2 = _direction * _direction;
  if (_len2 == 0)
    mooseError("The direction in the Grad2ParsedFunction must have positive length.");
}

Real
Grad2ParsedFunction::value(Real t, const Point & p)
{
  return (_function_ptr->evaluate<Real>(t, p + _direction) -
          2 * _function_ptr->evaluate<Real>(t, p) +
          _function_ptr->evaluate<Real>(t, p - _direction)) /
         _len2;
}
