//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatDiffusion.h"

registerMooseObject("MooseApp", MatDiffusion);

InputParameters
MatDiffusion::validParams()
{
  InputParameters params = MatDiffusionBase<Real>::validParams();
  params.addClassDescription(
      "Diffusion equation Kernel that takes an isotropic Diffusivity from a material property");
  return params;
}

MatDiffusion::MatDiffusion(const InputParameters & parameters) : MatDiffusionBase<Real>(parameters)
{
}
