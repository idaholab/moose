//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ImplicitODEx.h"

registerMooseObject("MooseTestApp", ImplicitODEx);

InputParameters
ImplicitODEx::validParams()
{
  InputParameters params = ODEKernel::validParams();
  params.addRequiredCoupledVar("y", "Y");
  return params;
}

ImplicitODEx::ImplicitODEx(const InputParameters & parameters)
  : ODEKernel(parameters), _y_var(coupledScalar("y")), _y(coupledScalarValue("y"))
{
}

ImplicitODEx::~ImplicitODEx() {}

Real
ImplicitODEx::computeQpResidual()
{
  return -3. * _u[_i] - 2. * _y[_i];
}

Real
ImplicitODEx::computeQpJacobian()
{
  return -3.;
}

Real
ImplicitODEx::computeQpOffDiagJacobianScalar(unsigned int jvar)
{
  if (jvar == _y_var)
    return -2.;
  else
    return 0.;
}
