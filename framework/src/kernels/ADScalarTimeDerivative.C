//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADScalarTimeDerivative.h"

registerMooseObject("MooseApp", ADScalarTimeDerivative);

InputParameters
ADScalarTimeDerivative::validParams()
{
  InputParameters params = ADScalarTimeKernel::validParams();
  params.addClassDescription(
      "Adds the time derivative contribution to the residual for a scalar variable.");
  return params;
}

ADScalarTimeDerivative::ADScalarTimeDerivative(const InputParameters & parameters)
  : ADScalarTimeKernel(parameters)
{
}

ADReal
ADScalarTimeDerivative::computeQpResidual()
{
  return _u_dot[_i];
}
