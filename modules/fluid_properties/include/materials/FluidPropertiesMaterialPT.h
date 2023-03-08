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
class FluidPropertiesMaterialPT : public Material
{
public:
  static InputParameters validParams();

  FluidPropertiesMaterialPT(const InputParameters & parameters);
  virtual ~FluidPropertiesMaterialPT();

protected:
  virtual void computeQpProperties();

  /// Pressure (Pa)
  const VariableValue & _pressure;
  /// Temperature (K)
  const VariableValue & _temperature;
  /// Density (kg/m^3)
  MaterialProperty<Real> & _rho;
  /// Viscosity (Pa.s)
  MaterialProperty<Real> & _mu;
  /// Isobaric specific heat capacity (J/kg/K)
  MaterialProperty<Real> & _cp;
  /// Isochoric specific heat capacity (J/kg/K)
  MaterialProperty<Real> & _cv;
  /// Thermal conductivity (W/m/K)
  MaterialProperty<Real> & _k;
  /// Specific enthalpy (J/kg)
  MaterialProperty<Real> & _h;
  /// Internal energy (J/kg)
  MaterialProperty<Real> & _e;

  /// Whether to compute entropy
  const bool _compute_s;
  /// Whether to compute the speed of sound
  const bool _compute_c;

  /// Specific entropy (J/kg/K)
  MaterialProperty<Real> * const _s;
  /// Speed of sound (m/s)
  MaterialProperty<Real> * const _c;

  /// Fluid properties UserObject
  const SinglePhaseFluidProperties & _fp;
};
