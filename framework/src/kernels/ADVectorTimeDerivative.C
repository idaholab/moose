//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADVectorTimeDerivative.h"

registerMooseObject("MooseApp", ADVectorTimeDerivative);

InputParameters
ADVectorTimeDerivative::validParams()
{
  InputParameters params = ADVectorTimeKernelValue::validParams();
  params.addClassDescription("The time derivative operator with the weak form of $(\\psi_i, "
                             "\\frac{\\partial u_h}{\\partial t})$.");
  return params;
}

ADVectorTimeDerivative::ADVectorTimeDerivative(const InputParameters & parameters)
  : ADVectorTimeKernelValue(parameters)
{
}

ADRealVectorValue
ADVectorTimeDerivative::precomputeQpResidual()
{
  return _u_dot[_qp];
}
