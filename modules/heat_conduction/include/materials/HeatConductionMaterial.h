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
#include "MooseTypes.h"

// Forward Declarations
class Function;

/**
 * Simple material with constant properties.
 */
template <bool is_ad>
class HeatConductionMaterialTempl : public Material
{
public:
  static InputParameters validParams();

  HeatConductionMaterialTempl(const InputParameters & parameters);

protected:
  virtual void computeProperties();

  const bool _has_temp;
  const GenericVariableValue<is_ad> & _temperature;

  const Real _my_thermal_conductivity;
  const Real _my_specific_heat;

  GenericMaterialProperty<Real, is_ad> & _thermal_conductivity;
  MaterialProperty<Real> & _thermal_conductivity_dT;
  const Function * const _thermal_conductivity_temperature_function;

  GenericMaterialProperty<Real, is_ad> & _specific_heat;
  const Function * const _specific_heat_temperature_function;
};

typedef HeatConductionMaterialTempl<false> HeatConductionMaterial;
typedef HeatConductionMaterialTempl<true> ADHeatConductionMaterial;
