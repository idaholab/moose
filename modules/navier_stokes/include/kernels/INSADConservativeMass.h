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

/**
 * This class computes the mass equation residual and Jacobian
 * contributions (the latter using automatic differentiation) for the incompressible Navier-Stokes
 * equations.
 */
class INSADConservativeMass : public ADKernelGrad
{
public:
  static InputParameters validParams();

  INSADConservativeMass(const InputParameters & parameters);

protected:
  ADRealVectorValue precomputeQpResidual() override;

  /// The velocity
  const ADVectorVariableValue & _vel;
};
