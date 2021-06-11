#pragma once

#include "Material.h"

/**
 * Computes T_wall from the constitutive model
 */
class ADTemperatureWall3EqnMaterial : public Material
{
public:
  ADTemperatureWall3EqnMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Wall temperature
  ADMaterialProperty<Real> & _T_wall;
  /// Wall heat flux
  const ADMaterialProperty<Real> & _q_wall;
  /// Heat transfer coefficient
  const ADMaterialProperty<Real> & _Hw;
  /// Fluid temperature
  const ADMaterialProperty<Real> & _T;

public:
  static InputParameters validParams();
};
