//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelValue.h"

/**
 * This class computes the momentum equation residual and Jacobian
 * contributions for force terms in the incompressible Navier-Stokes momentum
 * equation.
 */
class INSADMomentumForces : public ADVectorKernelValue
{
public:
  static InputParameters validParams();

  INSADMomentumForces(const InputParameters & parameters);

protected:
  virtual ADRealVectorValue precomputeQpResidual() override;

  const ADMaterialProperty<RealVectorValue> & _gravity_strong_residual;
  const ADMaterialProperty<RealVectorValue> & _mms_function_strong_residual;
};
