#pragma once

// Including the "ADKernel" base class here so we can extend it
#include "ADKernelGrad.h"

/**
 * Computes the residual contribution: K / mu * grad_test * grad_u.
 */
class DarcyPressure : public ADKernelGrad
{
public:
  static InputParameters validParams();

  DarcyPressure(const InputParameters & parameters);

protected:
  /// ADKernel objects must override precomputeQpResidual
  virtual ADRealVectorValue precomputeQpResidual() override;

  /// The material properties which hold the values for K and mu
  const ADMaterialProperty<Real> & _permeability;
  const ADMaterialProperty<Real> & _viscosity;
};
