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
#include "MultiComponentFluidProperties.h"

/**
 * Material for calculating fluid properties for a fluid comprised of two components:
 * the solute (eg, NaCl), and the solution (eg, water). This material uses the
 * pressure - temperature formulation.
 */
class MultiComponentFluidPropertiesMaterialPT : public Material
{
public:
  static InputParameters validParams();

  MultiComponentFluidPropertiesMaterialPT(const InputParameters & parameters);
  virtual ~MultiComponentFluidPropertiesMaterialPT();

protected:
  virtual void computeQpProperties();

  /// Pressure (Pa)
  const VariableValue & _pressure;
  /// Temperature (K)
  const VariableValue & _temperature;
  /// Mass fraction of solute (-)
  const VariableValue & _xmass;
  /// Density (kg/m^3)
  MaterialProperty<Real> & _rho;
  /// Enthalpy (kJ/kg)
  MaterialProperty<Real> & _h;
  /// Isobaric heat capacity (kJ/kg/K)
  MaterialProperty<Real> & _cp;
  /// Internal energy (kJ/kg)
  MaterialProperty<Real> & _e;

  // Multicomponent fluid properties UserObject
  const MultiComponentFluidProperties & _fp;
};
