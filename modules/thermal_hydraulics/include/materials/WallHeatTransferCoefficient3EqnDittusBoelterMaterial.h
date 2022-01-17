#pragma once

#include "Material.h"

/**
 * Computes wall heat transfer coefficient using Dittus-Boelter equation
 */
class WallHeatTransferCoefficient3EqnDittusBoelterMaterial : public Material
{
public:
  WallHeatTransferCoefficient3EqnDittusBoelterMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Wall heat transfer coefficient
  MaterialProperty<Real> & _Hw;
  /// Density
  const MaterialProperty<Real> & _rho;
  /// Velocity
  const MaterialProperty<Real> & _vel;
  /// Hydraulic diameter
  const MaterialProperty<Real> & _D_h;
  /// Heat conduction
  const MaterialProperty<Real> & _k;
  /// Dynamic viscosity
  const MaterialProperty<Real> & _mu;
  /// Dynamic viscosity
  const MaterialProperty<Real> & _cp;
  /// Fluid temperature
  const MaterialProperty<Real> & _T;
  /// Wall temperature
  const MaterialProperty<Real> & _T_wall;

public:
  static InputParameters validParams();
};
