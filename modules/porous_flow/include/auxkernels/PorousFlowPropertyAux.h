//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "PorousFlowDictator.h"
#include "PorousFlowConstants.h"

/**
 * Provides a simple interface to PorousFlow material properties.
 * Note that as all properties are in materials, only elemental
 * AuxVariables can be used and as such, all properties are evaluated
 * at the qps only
 */
template <bool is_ad>
class PorousFlowPropertyAuxTempl : public AuxKernel
{
public:
  static InputParameters validParams();

  PorousFlowPropertyAuxTempl(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

private:
  /// Pressure of each phase
  const GenericMaterialProperty<std::vector<Real>, is_ad> * _pressure;

  /// Saturation of each phase
  const GenericMaterialProperty<std::vector<Real>, is_ad> * _saturation;

  /// Temperature of the fluid
  const GenericMaterialProperty<Real, is_ad> * _temperature;

  /// Fluid density of each phase
  const GenericMaterialProperty<std::vector<Real>, is_ad> * _fluid_density;

  /// Viscosity of each phase
  const GenericMaterialProperty<std::vector<Real>, is_ad> * _fluid_viscosity;

  /// Mass fraction of each component in each phase
  const GenericMaterialProperty<std::vector<std::vector<Real>>, is_ad> * _mass_fractions;

  /// Relative permeability of each phase
  const GenericMaterialProperty<std::vector<Real>, is_ad> * _relative_permeability;

  /// Enthalpy of each phase
  const GenericMaterialProperty<std::vector<Real>, is_ad> * _enthalpy;

  /// Internal energy of each phase
  const GenericMaterialProperty<std::vector<Real>, is_ad> * _internal_energy;

  /// Secondary-species concentration
  const GenericMaterialProperty<std::vector<Real>, is_ad> * _sec_conc;

  /// Mineral-species concentration
  const GenericMaterialProperty<std::vector<Real>, is_ad> * _mineral_conc;

  /// Mineral-species reacion rate
  const GenericMaterialProperty<std::vector<Real>, is_ad> * _mineral_reaction_rate;

  /// Porosity of the media
  const GenericMaterialProperty<Real, is_ad> * _porosity;

  /// Permeability of the media
  const GenericMaterialProperty<RealTensorValue, is_ad> * _permeability;

  /// Hysteresis order
  const MaterialProperty<unsigned int> * _hys_order;

  /// Hysteresis saturation turning points
  const MaterialProperty<std::array<Real, PorousFlowConstants::MAX_HYSTERESIS_ORDER>> *
      _hys_sat_tps;

  /// Hysteresis info: what this physically represents depends on the PorousFlowHystereticInfo Material
  const MaterialProperty<Real> * _hys_info;

  /// PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;

  /// Enum of properties
  const enum class PropertyEnum {
    PRESSURE,
    SATURATION,
    TEMPERATURE,
    DENSITY,
    VISCOSITY,
    MASS_FRACTION,
    RELPERM,
    CAPILLARY_PRESSURE,
    ENTHALPY,
    INTERNAL_ENERGY,
    SECONDARY_CONCENTRATION,
    MINERAL_CONCENTRATION,
    MINERAL_REACTION_RATE,
    POROSITY,
    PERMEABILITY,
    HYSTERESIS_ORDER,
    HYSTERESIS_SATURATION_TURNING_POINT,
    HYSTERETIC_INFO
  } _property_enum;

  /// Phase index
  const unsigned int _phase;

  /// Liquid phase index
  const unsigned int _liquid_phase;

  /// Gas phase index
  const unsigned int _gas_phase;

  /// Fluid component index
  const unsigned int _fluid_component;

  /// Secondary species number
  const unsigned int _secondary_species;

  /// Mineral species number
  const unsigned int _mineral_species;

  /// Hysteresis turning point number
  const unsigned int _hysteresis_turning_point;

  /// Permeability tensor row and column
  const unsigned int _k_row;
  const unsigned int _k_col;
};

typedef PorousFlowPropertyAuxTempl<false> PorousFlowPropertyAux;
typedef PorousFlowPropertyAuxTempl<true> ADPorousFlowPropertyAux;
