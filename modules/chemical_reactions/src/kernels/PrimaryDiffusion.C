//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PrimaryDiffusion.h"

registerMooseObject("ChemicalReactionsApp", PrimaryDiffusion);

InputParameters
PrimaryDiffusion::validParams()
{
  InputParameters params = Diffusion::validParams();
  params.addClassDescription("Diffusion of primary species");
  return params;
}

PrimaryDiffusion::PrimaryDiffusion(const InputParameters & parameters)
  : Diffusion(parameters), _diffusivity(getMaterialProperty<Real>("diffusivity"))
{
}

Real
PrimaryDiffusion::computeQpResidual()
{
  return _diffusivity[_qp] * Diffusion::computeQpResidual();
}

Real
PrimaryDiffusion::computeQpJacobian()
{
  return _diffusivity[_qp] * Diffusion::computeQpJacobian();
}
