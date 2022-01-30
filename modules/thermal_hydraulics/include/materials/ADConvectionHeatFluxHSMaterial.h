#pragma once

#include "Material.h"

/**
 * Computes heat flux from convection with heat structure for a given fluid phase.
 */
class ADConvectionHeatFluxHSMaterial : public Material
{
public:
  ADConvectionHeatFluxHSMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Wall heat flux
  ADMaterialProperty<Real> & _q_wall;
  /// Fluid temperature for phase
  const ADMaterialProperty<Real> & _T;
  /// Wall temperature
  const ADMaterialProperty<Real> & _T_wall;
  /// Wall heat transfer coefficient for phase
  const ADMaterialProperty<Real> & _htc_wall;
  /// Wall contact fraction for phase
  const ADMaterialProperty<Real> & _kappa;

public:
  static InputParameters validParams();
};
