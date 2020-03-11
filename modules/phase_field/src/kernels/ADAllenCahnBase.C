//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADAllenCahnBase.h"

template <ComputeStage compute_stage, typename T>
InputParameters
ADAllenCahnBase<compute_stage, T>::validParams()
{
  InputParameters params = ADKernelValue<compute_stage>::validParams();
  params.addClassDescription(
      "Allen-Cahn bulk contribution Kernel with forward mode automatic differentiation");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  return params;
}
