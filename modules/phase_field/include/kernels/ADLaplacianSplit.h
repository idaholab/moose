//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelGrad.h"

// Forward Declarations

/**
 * Split with a variable that holds the Laplacian of the phase field.
 */
class ADLaplacianSplit : public ADKernelGrad
{
public:
  static InputParameters validParams();

  ADLaplacianSplit(const InputParameters & parameters);

protected:
  virtual ADRealGradient precomputeQpResidual();

private:
  const ADVariableValue & _var_c;
  const ADVariableGradient & _grad_c;
};
