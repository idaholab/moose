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
 * Computes a body force due to gravity
 */
class INSADGravityForce : public ADVectorKernelValue
{
public:
  INSADGravityForce(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  ADRealVectorValue precomputeQpResidual() override;

  const ADMaterialProperty<RealVectorValue> & _gravity_strong_residual;
};
