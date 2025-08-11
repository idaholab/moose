//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolycrystalMatAnisoDiffusion.h"

registerMooseObject("PhaseFieldApp", PolycrystalMatAnisoDiffusion);

InputParameters
PolycrystalMatAnisoDiffusion::validParams()
{
  InputParameters params = PolycrystalMatDiffusionBase<RealTensorValue>::validParams();
  params.addClassDescription( //TODO fix comment
      "Diffusion equation Kernel that takes an anisotropic Diffusivity from a material property");
  return params;
}

PolycrystalMatAnisoDiffusion::PolycrystalMatAnisoDiffusion(const InputParameters & parameters)
  : PolycrystalMatDiffusionBase<RealTensorValue>(parameters)
{
}
