#pragma once

#include "ADKernel.h"

/**
 * Computes gravity term for the energy equation in 1-phase flow
 */
class ADOneD3EqnEnergyGravity : public ADKernel
{
public:
  ADOneD3EqnEnergyGravity(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();

  const ADVariableValue & _A;

  const ADMaterialProperty<Real> & _rho;

  const ADMaterialProperty<Real> & _vel;

  /// The direction of the flow channel
  const ADMaterialProperty<RealVectorValue> & _dir;
  /// Gravitational acceleration vector
  const RealVectorValue & _gravity_vector;

public:
  static InputParameters validParams();
};
