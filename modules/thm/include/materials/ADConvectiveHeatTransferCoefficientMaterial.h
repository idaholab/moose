#pragma once

#include "Material.h"

/**
 * Computes convective heat transfer coefficient from Nusselt number
 */
class ADConvectiveHeatTransferCoefficientMaterial : public Material
{
public:
  ADConvectiveHeatTransferCoefficientMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Wall heat transfer coefficient
  ADMaterialProperty<Real> & _Hw;
  /// Nusselt number
  const ADMaterialProperty<Real> & _Nu;
  /// Hydraulic diameter
  const MaterialProperty<Real> & _D_h;
  /// Thermal conductivity
  const ADMaterialProperty<Real> & _k;

public:
  static InputParameters validParams();
};
