//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMatDiffusionBase.h"

defineADLegacyParams(ADMatDiffusionBase);

template <ComputeStage compute_stage, typename T>
InputParameters
ADMatDiffusionBase<compute_stage, T>::validParams()
{
  InputParameters params = ADKernelGrad<compute_stage>::validParams();
  params.addClassDescription("Diffusion kernel with a material property as diffusivity and "
                             "automatic differentiation to provide perfect Jacobians");
  params.addParam<MaterialPropertyName>(
      "diffusivity", "D", "The diffusivity value or material property");
  params.addCoupledVar("v",
                       "Coupled concentration variable for kernel to operate on; if this "
                       "is not specified, the kernel's nonlinear variable will be used as "
                       "usual");
  return params;
}
