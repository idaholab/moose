#pragma once

#include "ADKernel.h"

/**
 * Computes gravity term for the momentum equation for 1-phase flow
 */
class ADOneD3EqnMomentumGravity : public ADKernel
{
public:
  ADOneD3EqnMomentumGravity(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();

  const ADVariableValue & _A;

  const ADMaterialProperty<Real> & _rho;

  /// The direction of the flow channel
  const MaterialProperty<RealVectorValue> & _dir;
  /// Gravitational acceleration vector
  const RealVectorValue & _gravity_vector;

public:
  static InputParameters validParams();
};
