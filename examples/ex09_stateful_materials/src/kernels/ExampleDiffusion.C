//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExampleDiffusion.h"

#include "Material.h"

registerMooseObject("ExampleApp", ExampleDiffusion);

InputParameters
ExampleDiffusion::validParams()
{
  InputParameters params = Diffusion::validParams();
  return params;
}

ExampleDiffusion::ExampleDiffusion(const InputParameters & parameters)
  : Diffusion(parameters), _diffusivity(getMaterialProperty<Real>("diffusivity"))
{
}

Real
ExampleDiffusion::computeQpResidual()
{
  return _diffusivity[_qp] * Diffusion::computeQpResidual();
}

Real
ExampleDiffusion::computeQpJacobian()
{
  return _diffusivity[_qp] * Diffusion::computeQpJacobian();
}
