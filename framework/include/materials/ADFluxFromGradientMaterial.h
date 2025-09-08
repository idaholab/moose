#pragma once

#include "Material.h"

/**
 * Material computing a diffusive flux from the gradient of a coupled variable.
 *
 * J = -D * grad(u)
 */
class ADFluxFromGradientMaterial : public Material
{
public:
  static InputParameters validParams();

  ADFluxFromGradientMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Gradient of the coupled variable
  const ADVariableGradient & _grad_u;
  /// Diffusivity
  const ADMaterialProperty<Real> & _diffusivity;
  /// Material property storing the flux vector
  ADMaterialProperty<RealVectorValue> & _flux;
};
