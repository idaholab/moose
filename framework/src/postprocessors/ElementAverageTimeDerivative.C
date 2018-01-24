//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementAverageTimeDerivative.h"

template <>
InputParameters
validParams<ElementAverageTimeDerivative>()
{
  InputParameters params = validParams<ElementAverageValue>();
  return params;
}

ElementAverageTimeDerivative::ElementAverageTimeDerivative(const InputParameters & parameters)
  : ElementAverageValue(parameters)
{
}

Real
ElementAverageTimeDerivative::computeQpIntegral()
{
  return _u_dot[_qp];
}
