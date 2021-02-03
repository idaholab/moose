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

  /// The reference variables which hold the value for K and mu
  const Real & _permeability;
  const Real & _viscosity;
};
