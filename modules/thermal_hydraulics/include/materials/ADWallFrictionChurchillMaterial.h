#pragma once

#include "Material.h"

/**
 * Computes drag coefficient using the Churchill formula for Fanning friction factor
 */
class ADWallFrictionChurchillMaterial : public Material
{
public:
  ADWallFrictionChurchillMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Darcy wall friction coefficient
  const MaterialPropertyName _f_D_name;
  ADMaterialProperty<Real> & _f_D;

  /// Dynamic viscosity
  const ADMaterialProperty<Real> & _mu;

  /// Density of the phase
  const ADMaterialProperty<Real> & _rho;
  /// Velocity (x-component)
  const ADMaterialProperty<Real> & _vel;
  /// Hydraulic diameter
  const ADMaterialProperty<Real> & _D_h;
  /// Roughness of the surface
  const Real & _roughness;

public:
  static InputParameters validParams();
};
