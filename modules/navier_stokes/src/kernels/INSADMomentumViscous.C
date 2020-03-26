//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMomentumViscous.h"

registerMooseObject("NavierStokesApp", INSADMomentumViscous);

InputParameters
INSADMomentumViscous::validParams()
{
  InputParameters params = ADVectorKernelGrad::validParams();
  params.addClassDescription("Adds the viscous term to the INS momentum equation");
  params.addParam<MaterialPropertyName>(
      "mu_name", "mu", "The name of the viscosity material property");
  return params;
}

INSADMomentumViscous::INSADMomentumViscous(const InputParameters & parameters)
  : ADVectorKernelGrad(parameters), _mu(getADMaterialProperty<Real>("mu_name"))
{
}

ADRealTensorValue
INSADMomentumViscous::precomputeQpResidual()
{
  return _mu[_qp] * _grad_u[_qp];
}
