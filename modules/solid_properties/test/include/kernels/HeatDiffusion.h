#pragma once

#include "Kernel.h"

/**
 * Kernel providing the heat diffusion kernel for example purposes, with
 * strong form $-\nabla\cdot\left(k\nabla T\right)$, where $k$ is the
 * thermal conductivity and $T$ is the temperature.
 */
class HeatDiffusion : public Kernel
{
public:
  HeatDiffusion(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  /// Compute weak form residual
  virtual Real computeQpResidual() override;

  /// Compute Jacobian of weak form residual
  virtual Real computeQpJacobian() override;

  /// thermal conductivity
  const MaterialProperty<Real> & _k;

  /// derivative of thermal conductivity with respect to temperature
  const MaterialProperty<Real> & _dk_dT;
};
