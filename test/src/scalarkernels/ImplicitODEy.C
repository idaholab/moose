//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ImplicitODEy.h"

registerMooseObject("MooseTestApp", ImplicitODEy);

InputParameters
ImplicitODEy::validParams()
{
  InputParameters params = ODEKernel::validParams();
  params.addRequiredCoupledVar("x", "X");
  return params;
}

ImplicitODEy::ImplicitODEy(const InputParameters & parameters)
  : ODEKernel(parameters), _x_var(coupledScalar("x")), _x(coupledScalarValue("x"))
{
}

ImplicitODEy::~ImplicitODEy() {}

Real
ImplicitODEy::computeQpResidual()
{
  return -4 * _x[_i] - _u[_i];
}

Real
ImplicitODEy::computeQpJacobian()
{
  return -1.;
}

Real
ImplicitODEy::computeQpOffDiagJacobianScalar(unsigned int jvar)
{
  if (jvar == _x_var)
    return -4.;
  else
    return 0.;
}
