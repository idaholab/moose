//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimeDerivativeNodalKernel.h"

template <>
InputParameters
validParams<TimeDerivativeNodalKernel>()
{
  InputParameters params = validParams<NodalKernel>();
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
