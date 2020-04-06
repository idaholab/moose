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

#include "SodiumProperties.h"

class SodiumPropertiesMaterial : public Material
{
public:
  static InputParameters validParams();

  SodiumPropertiesMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Coupled temperature variable
  const VariableValue & _temperature;

  /// Thermal conductivity
  MaterialProperty<Real> & _k;

  /// Enthalpy
  MaterialProperty<Real> & _h;

  /// Heat capacity
  MaterialProperty<Real> & _cp;

  /// Temperature from enthalpy
  MaterialProperty<Real> & _T_from_h;

  /// Density
  MaterialProperty<Real> & _rho;

  /// Derivative of density with respect to temperature
  MaterialProperty<Real> & _drho_dT;

  /// Derivative of density with respect to enthalpy
  MaterialProperty<Real> & _drho_dh;

  // Sodium fluid properties UserObject
  const SodiumProperties & _sodium;
};
