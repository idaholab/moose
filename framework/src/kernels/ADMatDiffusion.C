//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMatDiffusion.h"

registerMooseObject("MooseApp", ADMatDiffusion);

InputParameters
ADMatDiffusion::validParams()
{
  auto params = ADMatDiffusionBase<Real>::validParams();
  params.addClassDescription(
      "Diffusion equation kernel that takes an isotropic diffusivity from a material property");
  return params;
}

ADMatDiffusion::ADMatDiffusion(const InputParameters & parameters)
  : ADMatDiffusionBase<Real>(parameters)
{
}
