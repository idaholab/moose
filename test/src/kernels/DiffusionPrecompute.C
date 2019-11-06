//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionPrecompute.h"

registerMooseObject("MooseTestApp", DiffusionPrecompute);

InputParameters
DiffusionPrecompute::validParams()
{
  InputParameters p = KernelGrad::validParams();
  return p;
}

DiffusionPrecompute::DiffusionPrecompute(const InputParameters & parameters)
  : KernelGrad(parameters)
{
}

DiffusionPrecompute::~DiffusionPrecompute() {}

RealGradient
DiffusionPrecompute::precomputeQpResidual()
{
  // Note we do not multiple by the gradient of the test function.  That is done in the parent class
  return _grad_u[_qp];
}

RealGradient
DiffusionPrecompute::precomputeQpJacobian()
{
  // Note we do not multiple by the gradient of the test function.  That is done in the parent class
  return _grad_phi[_j][_qp];
}
