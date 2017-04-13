/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowPropertyAux.h"

template <>
InputParameters
validParams<PorousFlowPropertyAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names");
  MooseEnum property_enum("pressure saturation temperature density viscosity mass_fraction relperm "
                          "enthalpy internal_energy");
  params.addRequiredParam<MooseEnum>(
      "property", property_enum, "The fluid property that this auxillary kernel is to calculate");
  params.addParam<unsigned int>("phase", 0, "The index of the phase this auxillary kernel acts on");
  params.addParam<unsigned int>(
      "fluid_component", 0, "The index of the fluid component this auxillary kernel acts on");
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
    _fluid_component(getParam<unsigned int>("fluid_component"))
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

  // Only get material properties required by this instance of the AuxKernel
  switch (_property_enum)
  {
    case PRESSURE:
      _pressure = &getMaterialProperty<std::vector<Real>>("PorousFlow_porepressure_qp");
      break;

    case SATURATION:
      _saturation = &getMaterialProperty<std::vector<Real>>("PorousFlow_saturation_qp");
      break;

    case TEMPERATURE:
      _temperature = &getMaterialProperty<Real>("PorousFlow_temperature_qp");
      break;

    case DENSITY:
      _fluid_density = &getMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_qp");
      break;

    case VISCOSITY:
      _fluid_viscosity = &getMaterialProperty<std::vector<Real>>("PorousFlow_viscosity_qp");
      break;

    case MASS_FRACTION:
      _mass_fractions =
          &getMaterialProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_qp");
      break;

    case RELPERM:
      _relative_permeability =
          &getMaterialProperty<std::vector<Real>>("PorousFlow_relative_permeability_qp");
      break;

    case ENTHALPY:
      _enthalpy = &getMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_enthalpy_qp");

    case INTERNAL_ENERGY:
      _internal_energy =
          &getMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_internal_energy_qp");
  }
}

Real
PorousFlowPropertyAux::computeValue()
{
  Real property = 0.0;

  switch (_property_enum)
  {
    case PRESSURE:
      property = (*_pressure)[_qp][_phase];
      break;

    case SATURATION:
      property = (*_saturation)[_qp][_phase];
      break;

    case TEMPERATURE:
      property = (*_temperature)[_qp];
      break;

    case DENSITY:
      property = (*_fluid_density)[_qp][_phase];
      break;

    case VISCOSITY:
      property = (*_fluid_viscosity)[_qp][_phase];
      break;

    case MASS_FRACTION:
      property = (*_mass_fractions)[_qp][_phase][_fluid_component];
      break;

    case RELPERM:
      property = (*_relative_permeability)[_qp][_phase];
      break;

    case ENTHALPY:
      property = (*_enthalpy)[_qp][_phase];
      break;

    case INTERNAL_ENERGY:
      property = (*_internal_energy)[_qp][_phase];
      break;
  }

  return property;
}
