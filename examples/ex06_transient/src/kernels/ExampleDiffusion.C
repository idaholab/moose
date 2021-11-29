//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExampleDiffusion.h"

registerMooseObject("ExampleApp", ExampleDiffusion);

InputParameters
ExampleDiffusion::validParams()
{
  InputParameters params = Diffusion::validParams();
  // Here we will look for a parameter from the input file
  params.addParam<Real>("diffusivity", 1.0, "Diffusivity Coefficient");
  return params;
}

ExampleDiffusion::ExampleDiffusion(const InputParameters & parameters)
  : Diffusion(parameters),
    // Initialize our member variable based on a default or input file
    _diffusivity(getParam<Real>("diffusivity"))
{
}

Real
ExampleDiffusion::computeQpResidual()
{
  return _diffusivity * Diffusion::computeQpResidual();
}

Real
ExampleDiffusion::computeQpJacobian()
{
  return _diffusivity * Diffusion::computeQpJacobian();
}
