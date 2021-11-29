//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledODETimeDerivative.h"

registerMooseObject("MooseApp", CoupledODETimeDerivative);

InputParameters
CoupledODETimeDerivative::validParams()
{
  InputParameters params = ODETimeKernel::validParams();
  params.addClassDescription(
      "Residual contribution of ODE from the time derivative of a coupled variable.");
  params.addRequiredCoupledVar("v", "Coupled variable.");
  return params;
}

CoupledODETimeDerivative::CoupledODETimeDerivative(const InputParameters & parameters)
  : ODETimeKernel(parameters), _v_dot(coupledScalarDot("v")), _dv_dot_dv(coupledScalarDotDu("v"))
{
}

Real
CoupledODETimeDerivative::computeQpResidual()
{
  return _v_dot[_i];
}

Real
CoupledODETimeDerivative::computeQpJacobian()
{
  if (_i == _j)
    return _dv_dot_dv[_i];
  else
    return 0;
}
