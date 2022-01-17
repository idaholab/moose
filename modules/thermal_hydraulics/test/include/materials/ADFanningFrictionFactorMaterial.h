#pragma once

#include "Material.h"

/**
 * Computes Fanning friction factor from Darcy friction factor
 */
class ADFanningFrictionFactorMaterial : public Material
{
public:
  ADFanningFrictionFactorMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Darcy friction factor
  const ADMaterialProperty<Real> & _f_D;

  /// Fanning friction factor
  ADMaterialProperty<Real> & _f_F;

public:
  static InputParameters validParams();
};
