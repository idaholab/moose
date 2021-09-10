//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADDiffusionRate.h"

registerMooseObject("PhaseFieldTestApp", ADDiffusionRate);

InputParameters
ADDiffusionRate::validParams()
{
  auto params = ADKernelGrad::validParams();
  params.addClassDescription("The Laplacian operator on the time derivative ($-\\nabla \\cdot "
                             "\\nabla \\dot{u}$), with the weak "
                             "form of $(\\nabla \\phi_i, \\nabla \\dot{u}_h)$.");

  params.addParam<Real>("mu", 1., "Viscosity on the Laplacian-rate");
  return params;
}

ADDiffusionRate::ADDiffusionRate(const InputParameters & parameters)
  : ADKernelGrad(parameters), _grad_u_dot(_var.adGradSlnDot()), _mu(getParam<Real>("mu"))
{
}

ADRealVectorValue
ADDiffusionRate::precomputeQpResidual()
{
  return _mu * _grad_u_dot[_qp];
}
