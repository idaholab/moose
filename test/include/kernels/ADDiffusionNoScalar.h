//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelScalarBase.h"

/**
 * This kernel implements the Laplacian operator:
 * $\nabla u \cdot \nabla \phi_i$
 * It is derived from the kernel scalar wrapper, and ignores the scalar variables.
 */
class ADDiffusionNoScalar : public ADKernelScalarBase
{
public:
  static InputParameters validParams();

  ADDiffusionNoScalar(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;
};
