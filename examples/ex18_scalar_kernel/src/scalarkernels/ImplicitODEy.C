//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ImplicitODEy.h"

/**
 * This function defines the valid parameters for
 * this Kernel and their default values
 */
registerMooseObject("ExampleApp", ImplicitODEy);

InputParameters
ImplicitODEy::validParams()
{
  InputParameters params = ODEKernel::validParams();
  params.addCoupledVar("x", "variable X coupled into this kernel");
  return params;
}

ImplicitODEy::ImplicitODEy(const InputParameters & parameters)
  : // You must call the constructor of the base class first
    ODEKernel(parameters),
    // get the coupled variable number and values
    _x_var(coupledScalar("x")),
    _x(coupledScalarValue("x"))
{
}

Real
ImplicitODEy::computeQpResidual()
{
  // the term of the ODE without the time derivative term
  return -4 * _x[_i] - _u[_i];
}

Real
ImplicitODEy::computeQpJacobian()
{
  // dF/dy
  return -1.;
}

Real
ImplicitODEy::computeQpOffDiagJacobianScalar(unsigned int jvar)
{
  if (jvar == _x_var)
    return -4.; // dF/dx
  else
    return 0.; // everything else
}
