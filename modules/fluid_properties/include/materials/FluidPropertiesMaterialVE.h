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

class SinglePhaseFluidProperties;

/**
 * Computes fluid properties using (specific energy, specific volume) formulation
 */
class FluidPropertiesMaterialVE : public Material
{
public:
  static InputParameters validParams();

  FluidPropertiesMaterialVE(const InputParameters & parameters);
  virtual ~FluidPropertiesMaterialVE();

protected:
  void computeQpProperties() override;

  /// Specific internal energy (J/kg)
  const VariableValue & _e;
  /// Specific volume (1/m^3)
  const VariableValue & _v;
  /// Pressure (Pa)
  MaterialProperty<Real> & _p;
  /// Temperature (K)
  MaterialProperty<Real> & _T;
  /// Speed of sound (m/s)
  MaterialProperty<Real> & _c;
  /// Isobaric specific heat capacity (J/kg/K)
  MaterialProperty<Real> & _cp;
  /// Isochoric specific heat capacity (J/kg/K)
  MaterialProperty<Real> & _cv;
  /// Dynamic viscosity (Pa.s)
  MaterialProperty<Real> & _mu;
  /// Thermal conductivity (W/m/K)
  MaterialProperty<Real> & _k;
  /// Specific entropy (J/kg/K)
  MaterialProperty<Real> & _s;
  /// Gibbs free energy
  MaterialProperty<Real> & _g;

  /// Fluid properties
  const SinglePhaseFluidProperties & _fp;
};
