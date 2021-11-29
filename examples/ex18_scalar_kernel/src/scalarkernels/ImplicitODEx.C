//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ImplicitODEx.h"

/**
 * This function defines the valid parameters for
 * this Kernel and their default values
 */
registerMooseObject("ExampleApp", ImplicitODEx);

InputParameters
ImplicitODEx::validParams()
{
  InputParameters params = ODEKernel::validParams();
  params.addCoupledVar("y", "variable Y coupled into this kernel");
  return params;
}

ImplicitODEx::ImplicitODEx(const InputParameters & parameters)
  : // You must call the constructor of the base class first
    ODEKernel(parameters),
    // get the coupled variable number and values
    _y_var(coupledScalar("y")),
    _y(coupledScalarValue("y"))
{
}

Real
ImplicitODEx::computeQpResidual()
{
  // the term of the ODE without the time derivative term
  return -3. * _u[_i] - 2. * _y[_i];
}

Real
ImplicitODEx::computeQpJacobian()
{
  // dF/dx
  return -3.;
}

Real
ImplicitODEx::computeQpOffDiagJacobianScalar(unsigned int jvar)
{
  if (jvar == _y_var)
    return -2.; // dF/dy
  else
    return 0.; // everything else
}
