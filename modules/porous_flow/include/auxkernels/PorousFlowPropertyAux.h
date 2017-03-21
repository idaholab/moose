/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
  enum PropertyEnum
  {
    PRESSURE,
    SATURATION,
    TEMPERATURE,
    DENSITY,
    VISCOSITY,
    MASS_FRACTION,
    RELPERM,
    ENTHALPY,
    INTERNAL_ENERGY
  };

  const PropertyEnum _property_enum;

  /// Phase index
  unsigned int _phase;

  /// Fluid omponent index
  unsigned int _fluid_component;
};

#endif // POROUSFLOWPROPERTYAUX_H
