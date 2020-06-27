//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADACInterfaceKobayashi2.h"

registerMooseObject("PhaseFieldApp", ADACInterfaceKobayashi2);

InputParameters
ADACInterfaceKobayashi2::validParams()
{
  InputParameters params = ADKernelGrad::validParams();
  params.addClassDescription("Anisotropic Gradient energy Allen-Cahn Kernel Part 2");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  params.addParam<MaterialPropertyName>("eps_name", "eps", "The anisotropic parameter");
  return params;
}

ADACInterfaceKobayashi2::ADACInterfaceKobayashi2(const InputParameters & parameters)
  : ADKernelGrad(parameters),
    _mob(getADMaterialProperty<Real>("mob_name")),
    _eps(getADMaterialProperty<Real>("eps_name"))
{
}

ADRealGradient
ADACInterfaceKobayashi2::precomputeQpResidual()
{
  // Set interfacial part of residual
  return _eps[_qp] * _eps[_qp] * _mob[_qp] * _grad_u[_qp];
}
