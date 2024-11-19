//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVCoupledGradDotElementalAdvection.h"

registerMooseObject("MooseTestApp", FVCoupledGradDotElementalAdvection);

InputParameters
FVCoupledGradDotElementalAdvection::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addRequiredCoupledVar("v", "The coupled variable.");
  return params;
}

FVCoupledGradDotElementalAdvection::FVCoupledGradDotElementalAdvection(
    const InputParameters & params)
  : FVElementalKernel(params), _grad_v(adCoupledGradient("v")), _grad_u(_var.adGradSln())
{
}

ADReal
FVCoupledGradDotElementalAdvection::computeQpResidual()
{
  return -1.0 * _grad_v[_qp] * _grad_u[_qp];
}
