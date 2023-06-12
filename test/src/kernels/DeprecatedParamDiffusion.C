//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DeprecatedParamDiffusion.h"

registerMooseObject("MooseTestApp", DeprecatedParamDiffusion);

InputParameters
DeprecatedParamDiffusion::validParams()
{
  InputParameters params = Diffusion::validParams();
  params.addDeprecatedParam<Real>("D", 1, "The diffusivity coefficient.", "This is deprecated");
  params.addDeprecatedParam<Real>(
      "E", 1, "The E diffusivity coefficient.", "This is also deprecated");
  return params;
}

DeprecatedParamDiffusion::DeprecatedParamDiffusion(const InputParameters & parameters)
  : Diffusion(parameters), _D(getParam<Real>("D"))
{
}

DeprecatedParamDiffusion::~DeprecatedParamDiffusion() {}

Real
DeprecatedParamDiffusion::computeQpResidual()
{
  return _D * Diffusion::computeQpResidual();
}

Real
DeprecatedParamDiffusion::computeQpJacobian()
{
  return _D * Diffusion::computeQpJacobian();
}
