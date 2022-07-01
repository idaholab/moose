//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SolidPropertiesMaterial.h"

class ThermalSolidProperties;

/**
 * Computes solid thermal properties as a function of temperature.
 */
class ThermalSolidPropertiesMaterial : public Material
{
public:
  static InputParameters validParams();

  ThermalSolidPropertiesMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Temperature
  const VariableValue & _temperature;

  /// Isobaric specific heat capacity
  MaterialProperty<Real> & _cp;

  /// Thermal conductivity
  MaterialProperty<Real> & _k;

  /// Density
  MaterialProperty<Real> & _rho;

  /// Solid properties
  const ThermalSolidProperties & _sp;
};
