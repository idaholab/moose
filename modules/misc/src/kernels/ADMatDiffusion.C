//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.htmlo
#include "ADMatDiffusion.h"

registerADMooseObject("MiscApp", ADMatDiffusion);

defineADValidParams(ADMatDiffusion,
                    ADDiffusion,
                    params.addRequiredParam<MaterialPropertyName>(
                        "diffusivity",
                        "the name of the diffusivity (or thermal conductivity, viscosity, etc. "
                        "depending on the physics) material property"););

template <ComputeStage compute_stage>
ADMatDiffusion<compute_stage>::ADMatDiffusion(const InputParameters & parameters)
  : ADDiffusion<compute_stage>(parameters),
    _diffusivity(adGetADMaterialProperty<Real>("diffusivity"))
{
}

template <ComputeStage compute_stage>
ADVectorResidual
ADMatDiffusion<compute_stage>::precomputeQpResidual()
{
  return _diffusivity[_qp] * ADDiffusion<compute_stage>::precomputeQpResidual();
}
