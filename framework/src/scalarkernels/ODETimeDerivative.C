//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ODETimeDerivative.h"

registerMooseObject("MooseApp", ODETimeDerivative);

InputParameters
ODETimeDerivative::validParams()
{
  InputParameters params = ODETimeKernel::validParams();
  params.addClassDescription(
      "Returns the time derivative contribution to the residual for a scalar variable.");
  return params;
}

ODETimeDerivative::ODETimeDerivative(const InputParameters & parameters) : ODETimeKernel(parameters)
{
}

Real
ODETimeDerivative::computeQpResidual()
{
  return _u_dot[_i];
}

Real
ODETimeDerivative::computeQpJacobian()
{
  if (_i == _j)
    return _du_dot_du[_i];
  else
    return 0;
}
