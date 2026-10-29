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

class MechanicalSolidProperties;

/**
 * Computes solid mechanical properties as a function of temperature.
 */
template <bool is_ad>
class MechanicalSolidPropertiesMaterialTempl : public Material
{
public:
  static InputParameters validParams();

  MechanicalSolidPropertiesMaterialTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Temperature
  const GenericVariableValue<is_ad> & _temperature;

  /// Young's modulus
  GenericMaterialProperty<Real, is_ad> & _E;

  /// Poisson's ratio
  GenericMaterialProperty<Real, is_ad> & _nu;

  /// Coefficient of thermal expansion
  GenericMaterialProperty<Real, is_ad> & _alpha;

  /// Solid properties
  const MechanicalSolidProperties & _sp;
};

typedef MechanicalSolidPropertiesMaterialTempl<false> MechanicalSolidPropertiesMaterial;
typedef MechanicalSolidPropertiesMaterialTempl<true> ADMechanicalSolidPropertiesMaterial;
