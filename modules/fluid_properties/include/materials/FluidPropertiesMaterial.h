//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
