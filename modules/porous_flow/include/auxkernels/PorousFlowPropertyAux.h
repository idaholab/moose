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
 * AuxVariables can be used
 */
class PorousFlowPropertyAux : public AuxKernel
{
public:
  PorousFlowPropertyAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

private:
  /// Pressure of each phase (at the qps)
  const MaterialProperty<std::vector<Real>> * _pressure;

  /// Saturation of each phase (at the qps)
  const MaterialProperty<std::vector<Real>> * _saturation;

  /// Temperature of the fluid (at the qps)
  const MaterialProperty<Real> * _temperature;

  /// Fluid density of each phase (at the qps)
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

  /// PorousFlow Dictator UserObject
  const PorousFlowDictator & _dictator;

  /// enum of properties
  const enum class PropertyEnum {
    PRESSURE,
    SATURATION,
    TEMPERATURE,
    DENSITY,
    VISCOSITY,
    MASS_FRACTION,
    RELPERM,
    ENTHALPY,
    INTERNAL_ENERGY
  } _property_enum;

  /// Phase index
  unsigned int _phase;

  /// Fluid omponent index
  unsigned int _fluid_component;
};

#endif // POROUSFLOWPROPERTYAUX_H
