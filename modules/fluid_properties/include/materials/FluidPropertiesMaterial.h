/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef FLUIDPROPERTIESMATERIAL_H
#define FLUIDPROPERTIESMATERIAL_H

#include "Material.h"

class FluidPropertiesMaterial;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<FluidPropertiesMaterial>();

/**
 * Computes fluid properties using (u, v) formulation
 */
class FluidPropertiesMaterial : public Material
{
public:
  FluidPropertiesMaterial(const InputParameters & parameters);
  virtual ~FluidPropertiesMaterial();

protected:
  virtual void computeQpProperties() override;

  /// Specific internal energy
  const VariableValue & _e;
  /// Specific volume
  const VariableValue & _v;
  /// Pressure
  MaterialProperty<Real> & _p;
  /// Temperature
  MaterialProperty<Real> & _T;
  /// Speed of sound
  MaterialProperty<Real> & _c;
  /// Isobaric specific heat capacity
  MaterialProperty<Real> & _cp;
  /// Isochoric specific heat capacity
  MaterialProperty<Real> & _cv;
  /// Dynamic viscosity
  MaterialProperty<Real> & _mu;
  /// Thermal conductivity
  MaterialProperty<Real> & _k;
  /// Gibbs free energy
  MaterialProperty<Real> & _g;

  /// Fluid properties
  const SinglePhaseFluidProperties & _fp;
};

#endif /* FLUIDPROPERTIESMATERIAL_H */
