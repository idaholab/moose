//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include "MaterialProperty.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

/**
 * A diffusion kernel with a vector material property, representing a diagonal tensor
 */
class VectorMatDiffusion : public DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>
{
public:
  static InputParameters validParams();

  VectorMatDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  const MaterialProperty<RealVectorValue> & _coef;

  const MaterialProperty<RealVectorValue> & _grad_coef;
};
