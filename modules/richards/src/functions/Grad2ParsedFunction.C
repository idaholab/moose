//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Grad2ParsedFunction.h"
#include "MooseParsedFunctionWrapper.h"

registerMooseObject("RichardsApp", Grad2ParsedFunction);

InputParameters
Grad2ParsedFunction::validParams()
{
  InputParameters params = MooseParsedFunction::validParams();
  params += MooseParsedFunction::validParams();
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
Grad2ParsedFunction::value(Real t, const Point & p) const
{
  return (_function_ptr->evaluate<Real>(t, p + _direction) -
          2 * _function_ptr->evaluate<Real>(t, p) +
          _function_ptr->evaluate<Real>(t, p - _direction)) /
         _len2;
}
