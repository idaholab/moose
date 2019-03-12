//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMomentumForces.h"

registerADMooseObject("NavierStokesApp", INSADMomentumForces);

defineADValidParams(INSADMomentumForces,
                    ADVectorKernelValue,
                    params.addClassDescription("Adds body forces to the INS momentum equation"););

template <ComputeStage compute_stage>
INSADMomentumForces<compute_stage>::INSADMomentumForces(const InputParameters & parameters)
  : ADVectorKernelValue<compute_stage>(parameters),
    _gravity_strong_residual(adGetADMaterialProperty<RealVectorValue>("gravity_strong_residual")),
    _mms_function_strong_residual(
        adGetADMaterialProperty<RealVectorValue>("mms_function_strong_residual"))
{
}

template <ComputeStage compute_stage>
ADVectorResidual
INSADMomentumForces<compute_stage>::precomputeQpResidual()
{
  return _gravity_strong_residual[_qp] + _mms_function_strong_residual[_qp];
}
