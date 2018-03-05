//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPropertyAux.h"

template <>
InputParameters
validParams<PorousFlowPropertyAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names");
  MooseEnum property_enum("pressure saturation temperature density viscosity mass_fraction relperm "
                          "enthalpy internal_energy secondary_concentration mineral_concentration "
                          "mineral_reaction_rate");
  params.addRequiredParam<MooseEnum>(
      "property", property_enum, "The fluid property that this auxillary kernel is to calculate");
  params.addParam<unsigned int>("phase", 0, "The index of the phase this auxillary kernel acts on");
  params.addParam<unsigned int>(
      "fluid_component", 0, "The index of the fluid component this auxillary kernel acts on");
  params.addParam<unsigned int>("secondary_species", 0, "The secondary chemical species number");
  params.addParam<unsigned int>("mineral_species", 0, "The mineral chemical species number");
  params.addClassDescription("AuxKernel to provide access to properties evaluated at quadpoints. "
                             "Note that elemental AuxVariables must be used, so that these "
                             "properties are integrated over each element.");
  return params;
}

PorousFlowPropertyAux::PorousFlowPropertyAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _property_enum(getParam<MooseEnum>("property").getEnum<PropertyEnum>()),
    _phase(getParam<unsigned int>("phase")),
    _fluid_component(getParam<unsigned int>("fluid_component")),
    _secondary_species(getParam<unsigned int>("secondary_species")),
    _mineral_species(getParam<unsigned int>("mineral_species"))
{
  // Check that the phase and fluid_component are valid
  if (_phase >= _dictator.numPhases())
    mooseError("Phase number in the AuxKernel ",
               _name,
               " is greater than the number of phases in the problem");

  if (_fluid_component >= _dictator.numComponents())
    mooseError("Fluid component number in the AuxKernel ",
               _name,
               " is greater than the number of phases in the problem");

  if (_property_enum == PropertyEnum::SECONDARY_CONCENTRATION &&
      (_secondary_species >= _dictator.numAqueousEquilibrium()))
    mooseError(
        "Secondary species in the AuxKernel ",
        _name,
        " is greater than the number of aqueous equilibrium chemical reactions in the problem");

  if ((_property_enum == PropertyEnum::MINERAL_CONCENTRATION ||
       _property_enum == PropertyEnum::MINERAL_REACTION_RATE) &&
      (_mineral_species >= _dictator.numAqueousKinetic()))
    mooseError("Mineral species in the AuxKernel ",
               _name,
               " is greater than the number of aqueous precipitation-dissolution chemical "
               "reactions in the problem");

  // Only get material properties required by this instance of the AuxKernel
  switch (_property_enum)
  {
    case PropertyEnum::PRESSURE:
      _pressure = &getMaterialProperty<std::vector<Real>>("PorousFlow_porepressure_qp");
      break;

    case PropertyEnum::SATURATION:
      _saturation = &getMaterialProperty<std::vector<Real>>("PorousFlow_saturation_qp");
      break;

    case PropertyEnum::TEMPERATURE:
      _temperature = &getMaterialProperty<Real>("PorousFlow_temperature_qp");
      break;

    case PropertyEnum::DENSITY:
      _fluid_density = &getMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_qp");
      break;

    case PropertyEnum::VISCOSITY:
      _fluid_viscosity = &getMaterialProperty<std::vector<Real>>("PorousFlow_viscosity_qp");
      break;

    case PropertyEnum::MASS_FRACTION:
      _mass_fractions =
          &getMaterialProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_qp");
      break;

    case PropertyEnum::RELPERM:
      _relative_permeability =
          &getMaterialProperty<std::vector<Real>>("PorousFlow_relative_permeability_qp");
      break;

    case PropertyEnum::ENTHALPY:
      _enthalpy = &getMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_enthalpy_qp");
      break;

    case PropertyEnum::INTERNAL_ENERGY:
      _internal_energy =
          &getMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_internal_energy_qp");
      break;

    case PropertyEnum::SECONDARY_CONCENTRATION:
      _sec_conc = &getMaterialProperty<std::vector<Real>>("PorousFlow_secondary_concentration_qp");
      break;

    case PropertyEnum::MINERAL_CONCENTRATION:
      _mineral_conc =
          &getMaterialProperty<std::vector<Real>>("PorousFlow_mineral_concentration_qp");
      break;

    case PropertyEnum::MINERAL_REACTION_RATE:
      _mineral_reaction_rate =
          &getMaterialProperty<std::vector<Real>>("PorousFlow_mineral_reaction_rate_qp");
      break;
  }
}

Real
PorousFlowPropertyAux::computeValue()
{
  Real property = 0.0;

  switch (_property_enum)
  {
    case PropertyEnum::PRESSURE:
      property = (*_pressure)[_qp][_phase];
      break;

    case PropertyEnum::SATURATION:
      property = (*_saturation)[_qp][_phase];
      break;

    case PropertyEnum::TEMPERATURE:
      property = (*_temperature)[_qp];
      break;

    case PropertyEnum::DENSITY:
      property = (*_fluid_density)[_qp][_phase];
      break;

    case PropertyEnum::VISCOSITY:
      property = (*_fluid_viscosity)[_qp][_phase];
      break;

    case PropertyEnum::MASS_FRACTION:
      property = (*_mass_fractions)[_qp][_phase][_fluid_component];
      break;

    case PropertyEnum::RELPERM:
      property = (*_relative_permeability)[_qp][_phase];
      break;

    case PropertyEnum::ENTHALPY:
      property = (*_enthalpy)[_qp][_phase];
      break;

    case PropertyEnum::INTERNAL_ENERGY:
      property = (*_internal_energy)[_qp][_phase];
      break;

    case PropertyEnum::SECONDARY_CONCENTRATION:
      property = (*_sec_conc)[_qp][_secondary_species];
      break;

    case PropertyEnum::MINERAL_CONCENTRATION:
      property = (*_mineral_conc)[_qp][_mineral_species];
      break;

    case PropertyEnum::MINERAL_REACTION_RATE:
      property = (*_mineral_reaction_rate)[_qp][_mineral_species];
      break;
  }

  return property;
}
