//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "JacobianCheck.h"

registerMooseObject("MooseTestApp", JacobianCheck);

InputParameters
JacobianCheck::validParams()
{
  InputParameters params = NodalKernel::validParams();
  return params;
}

JacobianCheck::JacobianCheck(const InputParameters & parameters) : NodalKernel(parameters) {}

Real
JacobianCheck::computeQpResidual()
{
  return -5.0 * _u[_qp];
}

Real
JacobianCheck::computeQpJacobian()
{
  return -5.0;
}
