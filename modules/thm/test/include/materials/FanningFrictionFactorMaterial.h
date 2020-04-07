#pragma once

#include "Material.h"

/**
 * Computes Fanning friction factor from Darcy friction factor
 */
class FanningFrictionFactorMaterial : public Material
{
public:
  FanningFrictionFactorMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Darcy friction factor
  const MaterialProperty<Real> & _f_D;

  /// Fanning friction factor
  MaterialProperty<Real> & _f_F;

public:
  static InputParameters validParams();
};
