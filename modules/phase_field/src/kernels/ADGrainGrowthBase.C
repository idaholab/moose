//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADGrainGrowthBase.h"

defineADValidParams(ADGrainGrowthBase,
                    ADAllenCahnBase,
                    params.addRequiredCoupledVar(
                        "v", "Array of coupled order parameter names for other order parameters"););

template <ComputeStage compute_stage>
ADGrainGrowthBase<compute_stage>::ADGrainGrowthBase(const InputParameters & parameters)
  : ADAllenCahnBase<compute_stage, Real>(parameters),
    _op_num(coupledComponents("v")),
    _vals(_op_num),
    _mu(getADMaterialProperty<Real>("mu"))
{
  // Loop through grains and load coupled variables into the arrays
  for (unsigned int i = 0; i < _op_num; ++i)
    _vals[i] = &adCoupledValue("v", i);
}

adBaseClass(ADGrainGrowthBase);
