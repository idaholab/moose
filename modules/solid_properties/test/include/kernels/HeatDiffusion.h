#ifndef HEATDIFFUSION_H
#define HEATDIFFUSION_H

#include "Kernel.h"
#include "DerivativeMaterialInterface.h"

class HeatDiffusion;

template<>
InputParameters validParams<HeatDiffusion>();

/**
 * Kernel providing the heat diffusion kernel for example purposes, with
 * strong form $-\nabla\cdot\left(k\nabla T\right)$, where $k$ is the
 * thermal conductivity and $T$ is the temperature.
 */
class HeatDiffusion : public DerivativeMaterialInterface<Kernel>
{
public:
  HeatDiffusion(const InputParameters & parameters);

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

#endif //HEATDIFFUSION_H
