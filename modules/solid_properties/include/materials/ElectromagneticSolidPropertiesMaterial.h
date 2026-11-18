//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

class ElectromagneticSolidProperties;

/**
 * Computes solid electromagnetic properties as a function of temperature.
 */
template <bool is_ad>
class ElectromagneticSolidPropertiesMaterialTempl : public Material
{
public:
  static InputParameters validParams();

  ElectromagneticSolidPropertiesMaterialTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Temperature
  const GenericVariableValue<is_ad> & _temperature;

  /// Electrical resistivity
  GenericMaterialProperty<Real, is_ad> & _electrical_resistivity;

  /// Electrical conductivity
  GenericMaterialProperty<Real, is_ad> & _electrical_conductivity;

  /// Magnetic permeability
  GenericMaterialProperty<Real, is_ad> & _magnetic_permeability;

  /// Solid properties user object
  const ElectromagneticSolidProperties & _sp;
};

typedef ElectromagneticSolidPropertiesMaterialTempl<false> ElectromagneticSolidPropertiesMaterial;
typedef ElectromagneticSolidPropertiesMaterialTempl<true> ADElectromagneticSolidPropertiesMaterial;
