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

class ThermalSolidProperties;

/**
 * Computes solid thermal properties as a function of temperature.
 */
template <bool is_ad>
class ThermalSolidPropertiesMaterialTempl : public Material
{
public:
  static InputParameters validParams();

  ThermalSolidPropertiesMaterialTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Temperature
  const GenericVariableValue<is_ad> & _temperature;

  /// Isobaric specific heat capacity
  GenericMaterialProperty<Real, is_ad> & _cp;

  /// Thermal conductivity
  GenericMaterialProperty<Real, is_ad> & _k;

  /// Density
  GenericMaterialProperty<Real, is_ad> & _rho;

  /// Solid properties
  const ThermalSolidProperties & _sp;
};

typedef ThermalSolidPropertiesMaterialTempl<false> ThermalSolidPropertiesMaterial;
typedef ThermalSolidPropertiesMaterialTempl<true> ADThermalSolidPropertiesMaterial;
