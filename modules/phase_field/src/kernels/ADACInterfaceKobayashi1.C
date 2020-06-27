//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADACInterfaceKobayashi1.h"

registerMooseObject("PhaseFieldApp", ADACInterfaceKobayashi1);

InputParameters
ADACInterfaceKobayashi1::validParams()
{
  InputParameters params = ADKernelGrad::validParams();
  params.addClassDescription("Anisotropic gradient energy Allen-Cahn Kernel Part 1");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  params.addParam<MaterialPropertyName>("eps_name", "eps", "The anisotropic interface parameter");
  params.addParam<MaterialPropertyName>(
      "deps_name",
      "deps",
      "The derivative of the anisotropic interface parameter with respect to angle");
  return params;
}

ADACInterfaceKobayashi1::ADACInterfaceKobayashi1(const InputParameters & parameters)
  : ADKernelGrad(parameters),
    _mob(getADMaterialProperty<Real>("mob_name")),
    _eps(getADMaterialProperty<Real>("eps_name")),
    _deps(getADMaterialProperty<Real>("deps_name"))
{
}

ADRealGradient
ADACInterfaceKobayashi1::precomputeQpResidual()
{
  // Set modified gradient vector
  const ADRealGradient v(-_grad_u[_qp](1), _grad_u[_qp](0), 0);

  // Define anisotropic interface residual
  return _eps[_qp] * _deps[_qp] * _mob[_qp] * v;
}
