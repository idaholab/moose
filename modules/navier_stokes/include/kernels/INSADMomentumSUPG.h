//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelSUPG.h"

/**
 * This class computes the momentum equation residual and Jacobian
 * contributions for SUPG stabilization terms of the incompressible Navier-Stokes momentum
 * equation.
 */
class INSADMomentumSUPG : public ADVectorKernelSUPG
{
public:
  static InputParameters validParams();

  INSADMomentumSUPG(const InputParameters & parameters);

protected:
  virtual ADRealVectorValue precomputeQpStrongResidual() override;

  const ADMaterialProperty<RealVectorValue> & _momentum_strong_residual;
};
