#pragma once

#include "Material.h"

class PrandtlNumberMaterial;

template <>
InputParameters validParams<PrandtlNumberMaterial>();

/**
 * Computes Prandtl number as material property
 */
class PrandtlNumberMaterial : public Material
{
public:
  PrandtlNumberMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Prandtl number
  MaterialProperty<Real> & _Pr;
  /// Constant-pressure specific heat
  const MaterialProperty<Real> & _cp;
  /// Dynamic viscosity
  const MaterialProperty<Real> & _mu;
  /// Thermal conductivity
  const MaterialProperty<Real> & _k;
};
