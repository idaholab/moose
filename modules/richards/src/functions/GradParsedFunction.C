//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GradParsedFunction.h"
#include "MooseParsedFunctionWrapper.h"

registerMooseObject("RichardsApp", GradParsedFunction);

InputParameters
GradParsedFunction::validParams()
{
  InputParameters params = MooseParsedFunction::validParams();
  params += MooseParsedFunction::validParams();
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
GradParsedFunction::value(Real t, const Point & p) const
{
  return (_function_ptr->evaluate<Real>(t, p + _direction) -
          _function_ptr->evaluate<Real>(t, p - _direction)) /
         _len;
}
