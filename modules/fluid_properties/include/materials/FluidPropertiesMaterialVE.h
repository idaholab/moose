//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "SinglePhaseFluidProperties.h"

/**
 * Computes fluid properties using (pressure, temperature) formulation
 */
class FluidPropertiesMaterialVE : public Material
{
public:
  static InputParameters validParams();

  FluidPropertiesMaterialVE(const InputParameters & parameters);
  virtual ~FluidPropertiesMaterialVE();

protected:
  virtual void computeQpProperties();

  /// Specific volume (m^3/kg)
  const VariableValue & _v;
  /// Internal energy (kJ/kg)
  const VariableValue & _e;
  /// Density (kg/m^3)
  MaterialProperty<Real> & _rho;
  // /// Pressure (Pa)
  // MaterialProperty<Real> & _pressure;
  // /// Temperature (K)
  // MaterialProperty<Real> & _temperature;
  // /// Speed of sound (m/s)
  // MaterialProperty<Real> & _c;
  // /// Specific heat capacity at constant pressure (kJ/kg/K)
  // MaterialProperty<Real> & _cp;
  // /// Specific heat capacity at constant volume (kJ/kg/K)
  // MaterialProperty<Real> & _cv;
  // /// Viscosity (Pa.s)
  // MaterialProperty<Real> & _mu;
  // /// Thermal conductivity (W/m/K)
  // MaterialProperty<Real> & _k;
  // /// Gibbs free energty (kJ/kg)
  // MaterialProperty<Real> & _g;

  /// Fluid properties UserObject
  const SinglePhaseFluidProperties & _fp;
};
