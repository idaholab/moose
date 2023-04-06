//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorTimeDerivative.h"

registerMooseObject("MooseApp", FunctorTimeDerivative);

InputParameters
FunctorTimeDerivative::validParams()
{
  InputParameters params = ADTimeKernelValue::validParams();
  return params;
}

FunctorTimeDerivative::FunctorTimeDerivative(const InputParameters & parameters)
  : ADTimeKernelValue(parameters)
{
}

ADReal
FunctorTimeDerivative::precomputeQpResidual()
{
  return _var.dot(std::make_tuple(_current_elem, _qp, _qrule), Moose::currentState());
}
