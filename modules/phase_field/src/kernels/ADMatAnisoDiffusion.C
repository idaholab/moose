//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMatAnisoDiffusion.h"

registerADMooseObject("MooseApp", ADMatAnisoDiffusion);

defineADValidParams(
    ADMatAnisoDiffusion,
    ADMatDiffusionBase,
    params.addClassDescription("Diffusion equation Kernel that takes an anisotropic Diffusivity "
                               "from a material property"););

template <ComputeStage compute_stage>
ADMatAnisoDiffusion<compute_stage>::ADMatAnisoDiffusion(const InputParameters & parameters)
  : ADMatDiffusionBase<compute_stage, RealTensorValue>(parameters)
{
}
