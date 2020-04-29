//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADTimeKernelValue.h"

/**
 * This class computes the time derivative for the incompressible
 * Navier-Stokes momentum equation.  Could instead use CoefTimeDerivative
 * for this.
 */
class INSADMomentumTimeDerivative : public ADVectorTimeKernelValue
{
public:
  static InputParameters validParams();

  INSADMomentumTimeDerivative(const InputParameters & parameters);

protected:
  virtual ADRealVectorValue precomputeQpResidual() override;

  const ADMaterialProperty<RealVectorValue> & _td_strong_residual;
};
