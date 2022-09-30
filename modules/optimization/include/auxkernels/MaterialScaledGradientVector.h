//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#pragma once

#include "AuxKernel.h"

/**
 * Auxiliary kernel to output scaled gradient of a variable in vector format
 */
class MaterialScaledGradientVector : public VectorAuxKernel
{
public:
  static InputParameters validParams();

  MaterialScaledGradientVector(const InputParameters & parameters);

protected:
  virtual RealVectorValue computeValue() override;

  /// The gradient of a coupled variable
  const VariableGradient & _var_grad;

  /// scaling material for gradient
  const MaterialProperty<Real> & _mat_scaling;
};
