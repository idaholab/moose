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

template <>
InputParameters
validParams<ADDiffusion>()
{
  InputParameters p = validParams<ADKernel>();
  p.addClassDescription("Same as `Diffusion` in terms of physics/residual, but the Jacobian "
                        "is computed using forward automatic differentiation");
  return p;
}

ADDiffusion::ADDiffusion(const InputParameters & parameters) : ADKernel(parameters) {}

ADReal
ADDiffusion::computeQpResidual()
{
  return _grad_u[_qp] * _grad_test[_i][_qp];
}
