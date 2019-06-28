//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMatDiffusion.h"

registerADMooseObject("MooseApp", ADMatDiffusion);

defineADValidParams(
    ADMatDiffusion,
    ADMatDiffusionBase,
    params.addClassDescription(
        "Diffusion equation kernel that takes an isotropic diffusivity from a material property"););

template <ComputeStage compute_stage>
ADMatDiffusion<compute_stage>::ADMatDiffusion(const InputParameters & parameters)
  : ADMatDiffusionBase<compute_stage, Real>(parameters)
{
}
