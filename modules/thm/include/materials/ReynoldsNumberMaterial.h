#pragma once

#include "Material.h"

class ReynoldsNumberMaterial;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<ReynoldsNumberMaterial>();

/**
 * Computes Reynolds number as a material property
 */
class ReynoldsNumberMaterial : public Material
{
public:
  ReynoldsNumberMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Density of the phase
  const MaterialProperty<Real> & _rho;
  /// Velocity of the phase
  const MaterialProperty<Real> & _vel;
  /// Hydraulic diameter
  const MaterialProperty<Real> & _D_h;
  /// Dynamic viscosity of the phase
  const MaterialProperty<Real> & _mu;
  /// Reynolds
  MaterialProperty<Real> & _Re;
};
