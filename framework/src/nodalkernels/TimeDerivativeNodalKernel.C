//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimeDerivativeNodalKernel.h"

registerMooseObject("MooseApp", TimeDerivativeNodalKernel);

InputParameters
TimeDerivativeNodalKernel::validParams()
{
  InputParameters params = TimeNodalKernel::validParams();
  params.addClassDescription(
      "Forms the contribution to the residual and jacobian of the time derivative term from an ODE "
      "being solved at all nodes.");
  return params;
}

TimeDerivativeNodalKernel::TimeDerivativeNodalKernel(const InputParameters & parameters)
  : TimeNodalKernel(parameters)
{
}

Real
TimeDerivativeNodalKernel::computeQpResidual()
{
  return _u_dot[_qp];
}

Real
TimeDerivativeNodalKernel::computeQpJacobian()
{
  return _du_dot_du[_qp];
}
