//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelValue.h" // Provides AD(Vector)KernelValue

/**
 * Computes a body force due to a coupled vector variable or vector function
 */
class INSADMomentumCoupledForce : public ADVectorKernelValue
{
public:
  INSADMomentumCoupledForce(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  ADRealVectorValue precomputeQpResidual() override;

  const ADMaterialProperty<RealVectorValue> & _coupled_force_strong_residual;
};
