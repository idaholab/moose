/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "MatAnisoDiffusion.h"

template <>
InputParameters
validParams<MatAnisoDiffusion>()
{
  InputParameters params = MatDiffusionBase<RealTensorValue>::validParams();
  params.addClassDescription(
      "Diffusion equation Kernel that takes an anisotropic Diffusivity from a material property");
  return params;
}

MatAnisoDiffusion::MatAnisoDiffusion(const InputParameters & parameters)
  : MatDiffusionBase<RealTensorValue>(parameters)
{
}
