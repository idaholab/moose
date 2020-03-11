//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADSplitCHWRes.h"

registerADMooseObject("PhaseFieldApp", ADSplitCHWRes);

defineADLegacyParams(ADSplitCHWRes);

template <ComputeStage compute_stage>
InputParameters
ADSplitCHWRes<compute_stage>::validParams()
{
  InputParameters params = ADSplitCHWResBase<compute_stage>::validParams();
  params.addClassDescription("Split formulation Cahn-Hilliard Kernel for the chemical potential "
                             "variable with a scalar (isotropic) mobility");
  return params;
}

template <ComputeStage compute_stage>
ADSplitCHWRes<compute_stage>::ADSplitCHWRes(const InputParameters & parameters)
  : ADSplitCHWResBase<compute_stage, Real>(parameters)
{
}
