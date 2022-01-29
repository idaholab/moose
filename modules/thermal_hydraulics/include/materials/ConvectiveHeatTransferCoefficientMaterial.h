#pragma once

#include "Material.h"

/**
 * Computes convective heat transfer coefficient from Nusselt number
 */
class ConvectiveHeatTransferCoefficientMaterial : public Material
{
public:
  ConvectiveHeatTransferCoefficientMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Wall heat transfer coefficient
  MaterialProperty<Real> & _Hw;
  /// Nusselt number
  const MaterialProperty<Real> & _Nu;
  /// Hydraulic diameter
  const MaterialProperty<Real> & _D_h;
  /// Thermal conductivity
  const MaterialProperty<Real> & _k;

public:
  static InputParameters validParams();
};
