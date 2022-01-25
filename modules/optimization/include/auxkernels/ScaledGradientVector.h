#pragma once

#include "AuxKernel.h"

/**
 * Auxiliary kernel to output scaled gradient of a variable in vector format
 */
class ScaledGradientVector : public VectorAuxKernel
{
public:
  static InputParameters validParams();

  ScaledGradientVector(const InputParameters & parameters);

protected:
  virtual RealVectorValue computeValue() override;

  /// The gradient of a coupled variable
  const VariableGradient & _var_grad;

  /// scaling material for gradient
  const MaterialProperty<Real> & _mat_scaling;
};
