#pragma once

#include "Material.h"

/**
 * Computes wall heat transfer coefficient using Dittus-Boelter equation
 */
class ADWallHeatTransferCoefficient3EqnDittusBoelterMaterial : public Material
{
public:
  ADWallHeatTransferCoefficient3EqnDittusBoelterMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Wall heat transfer coefficient
  ADMaterialProperty<Real> & _Hw;
  /// Density
  const ADMaterialProperty<Real> & _rho;
  /// Velocity
  const ADMaterialProperty<Real> & _vel;
  /// Hydraulic diameter
  const MaterialProperty<Real> & _D_h;
  /// Heat conduction
  const ADMaterialProperty<Real> & _k;
  /// Dynamic viscosity
  const ADMaterialProperty<Real> & _mu;
  /// Dynamic viscosity
  const ADMaterialProperty<Real> & _cp;
  /// Fluid temperature
  const ADMaterialProperty<Real> & _T;
  /// Wall temperature
  const ADMaterialProperty<Real> & _T_wall;

public:
  static InputParameters validParams();
};
