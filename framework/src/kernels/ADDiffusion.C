//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADDiffusion.h"

registerMooseObject("MooseApp", ADDiffusion);

InputParameters
ADDiffusion::validParams()
{
  auto params = ADKernelGrad::validParams();
  params.addClassDescription("Same as `Diffusion` in terms of physics/residual, but the Jacobian "
                             "is computed using forward automatic differentiation");
  return params;
}

ADDiffusion::ADDiffusion(const InputParameters & parameters) : ADKernelGrad(parameters) {}

ADRealVectorValue
ADDiffusion::precomputeQpResidual()
{
  return _grad_u[_qp];
}
