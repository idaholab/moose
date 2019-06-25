//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMatDiffusionBase.h"

defineADValidParams(
    ADMatDiffusionBase,
    ADKernelGrad,
    params.addClassDescription("Diffusion kernel with a material property as diffusivity and "
                               "automatic differentiation to provide perfect Jacobians");
    params.addParam<MaterialPropertyName>("diffusivity",
                                          "D",
                                          "The diffusivity value or material property");
    params.addCoupledVar("args", "Vector of arguments of the diffusivity");
    params.addCoupledVar("v",
                         "Coupled concentration variable for kernel to operate on; if this "
                         "is not specified, the kernel's nonlinear variable will be used as "
                         "usual"););
