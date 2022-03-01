//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementAverageTimeDerivative.h"

registerMooseObject("MooseApp", ElementAverageTimeDerivative);

InputParameters
ElementAverageTimeDerivative::validParams()
{
  InputParameters params = ElementAverageValue::validParams();
  params.addClassDescription(
      "Computes a volume integral of the time derivative of a given variable");
  return params;
}

ElementAverageTimeDerivative::ElementAverageTimeDerivative(const InputParameters & parameters)
  : ElementAverageValue(parameters), _u_dot(_is_transient ? coupledDot("variable") : _zero)
{
}

Real
ElementAverageTimeDerivative::computeQpIntegral()
{
  return _u_dot[_qp];
}
