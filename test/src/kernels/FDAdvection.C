//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FDAdvection.h"

template <>
InputParameters
validParams<FDAdvection>()
{
  InputParameters params = validParams<FDKernel>();

  params.addRequiredCoupledVar(
      "advector", "The gradient of this variable will be used as the velocity vector.");
  return params;
}

FDAdvection::FDAdvection(const InputParameters & parameters)
  : FDKernel(parameters), _grad_advector(coupledGradient("advector"))
{
}

Real
FDAdvection::computeQpResidual()
{
  return _test[_i][_qp] * (_grad_advector[_qp] * _grad_u[_qp]);
}
