/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "MatDiffusion.h"

template <>
InputParameters
validParams<MatDiffusion>()
{
  InputParameters params = MatDiffusionBase<Real>::validParams();
  params.addClassDescription(
      "Diffusion equation Kernel that takes an isotropic Diffusivity from a material property");
  return params;
}

MatDiffusion::MatDiffusion(const InputParameters & parameters) : MatDiffusionBase<Real>(parameters)
{
}
