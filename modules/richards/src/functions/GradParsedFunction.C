/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "GradParsedFunction.h"
#include "MooseParsedFunctionWrapper.h"

template <>
InputParameters
validParams<GradParsedFunction>()
{
  InputParameters params = validParams<MooseParsedFunction>();
  params += validParams<MooseParsedFunction>();
  params.addRequiredParam<RealVectorValue>(
      "direction",
      "The direction in which to take the derivative.  This must not be a zero-length vector");
  return params;
}

GradParsedFunction::GradParsedFunction(const InputParameters & parameters)
  : MooseParsedFunction(parameters), _direction(getParam<RealVectorValue>("direction"))
{
  _len = _direction.norm();
  if (_len == 0)
    mooseError("The direction in the GradParsedFunction must have positive length.");
  _direction /= 2.0; // note - so we can do central differences
}

Real
GradParsedFunction::value(Real t, const Point & p)
{
  return (_function_ptr->evaluate<Real>(t, p + _direction) -
          _function_ptr->evaluate<Real>(t, p - _direction)) /
         _len;
}
