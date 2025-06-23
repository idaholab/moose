//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUTimeDerivativeNodalKernel.h"

registerMooseObject("MooseApp", GPUTimeDerivativeNodalKernel);

InputParameters
GPUTimeDerivativeNodalKernel::validParams()
{
  InputParameters params = GPUTimeNodalKernel::validParams();
  params.addClassDescription(
      "Forms the contribution to the residual and jacobian of the time derivative term from an ODE "
      "being solved at all nodes.");
  return params;
}

GPUTimeDerivativeNodalKernel::GPUTimeDerivativeNodalKernel(const InputParameters & parameters)
  : GPUTimeNodalKernel(parameters)
{
}
