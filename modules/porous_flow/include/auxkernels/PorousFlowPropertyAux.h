//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWPROPERTYAUX_H
#define POROUSFLOWPROPERTYAUX_H

#include "AuxKernel.h"
#include "PorousFlowDictator.h"

class PorousFlowPropertyAux;

template <>
InputParameters validParams<PorousFlowPropertyAux>();

/**
 * Provides a simple interface to PorousFlow material properties.
 * Note that as all properties are in materials, only elemental
 * AuxVariables can be used and as such, all properties are evaluated
 * at the qps only
 */
class PorousFlowPropertyAux : public AuxKernel
{
public:
  PorousFlowPropertyAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

private:
  /// Pressure of each phase
  const MaterialProperty<std::vector<Real>> * _pressure;

  /// Saturation of each phase
  const MaterialProperty<std::vector<Real>> * _saturation;

  /// Temperature of the fluid
  const MaterialProperty<Real> * _temperature;

  /// Fluid density of each phase
  const MaterialProperty<std::vector<Real>> * _fluid_density;

  /// Viscosity of each phase
  const MaterialProperty<std::vector<Real>> * _fluid_viscosity;

  /// Mass fraction of each component in each phase
  const MaterialProperty<std::vector<std::vector<Real>>> * _mass_fractions;

  /// Relative permeability of each phase
  const MaterialProperty<std::vector<Real>> * _relative_permeability;

  /// Enthalpy of each phase
  const MaterialProperty<std::vector<Real>> * _enthalpy;

  /// Internal energy of each phase
  const MaterialProperty<std::vector<Real>> * _internal_energy;

  /// Secondary-species concentration
  const MaterialProperty<std::vector<Real>> * _sec_conc;

  /// Mineral-species concentration
  const MaterialProperty<std::vector<Real>> * _mineral_conc;

  /// Mineral-species reacion rate
  const MaterialProperty<std::vector<Real>> * _mineral_reaction_rate;

  /// Porosity of the media
  const MaterialProperty<Real> * _porosity;

  /// Permeability of the media
  const MaterialProperty<RealTensorValue> * _permeability;

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
    PERMEABILITY
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

  /// Permeability tensor row and column
  const unsigned int _k_row;
  const unsigned int _k_col;
};

#endif // POROUSFLOWPROPERTYAUX_H
