/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowPropertyAux.h"

template<>
InputParameters validParams<PorousFlowPropertyAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<UserObjectName>("PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names");
  MooseEnum property_enum("pressure saturation density viscosity mass_fraction relperm");
  params.addRequiredParam<MooseEnum>("property", property_enum, "The fluid property that this auxillary kernel is to calculate");
  params.addParam<unsigned int>("phase", 0, "The index of the phase this auxillary kernel acts on");
  params.addParam<unsigned int>("fluid_component", 0, "The index of the fluid component this auxillary kernel acts on");
  return params;
}

PorousFlowPropertyAux::PorousFlowPropertyAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _property_enum(getParam<MooseEnum>("property")),
    _phase(getParam<unsigned int>("phase")),
    _fluid_component(getParam<unsigned int>("fluid_component"))
{
  // Check that the phase and fluid_component are valid
  if (_phase >= _dictator.numPhases())
    mooseError("Phase number in the AuxKernel " << _name << " is greater than the number of phases in the problem");

  if (_fluid_component >= _dictator.numComponents())
    mooseError("Fluid component number in the AuxKernel " << _name << " is greater than the number of phases in the problem");

  // Only get material properties required by this instance of the AuxKernel
  switch (_property_enum)
  {
    case 0: // pressure
      _pressure = & getMaterialProperty<std::vector<Real> >("PorousFlow_porepressure_qp");
      break;

    case 1: // saturation
      _saturation = & getMaterialProperty<std::vector<Real> >("PorousFlow_saturation_qp");
      break;

    case 2: // density
      _fluid_density = & getMaterialProperty<std::vector<Real> >("PorousFlow_fluid_phase_density_qp");
      break;

    case 3: // viscosity
      _fluid_viscosity = & getMaterialProperty<std::vector<Real> >("PorousFlow_viscosity");
      break;

    case 4: // mass fraction
      _mass_fractions = & getMaterialProperty<std::vector<std::vector<Real> > >("PorousFlow_mass_frac");
      break;

    case 5: // relative permeability
      _relative_permeability = & getMaterialProperty<std::vector<Real> >("PorousFlow_relative_permeability");
      break;
  }
}

Real
PorousFlowPropertyAux::computeValue()
{
  Real property = 0.0;

  switch (_property_enum)
  {
    case 0: // pressure
      property = (*_pressure)[_qp][_phase];
      break;

    case 1: // saturation
      property = (*_saturation)[_qp][_phase];
      break;

    case 2: // density
      property = (*_fluid_density)[_qp][_phase];
      break;

    case 3: // viscosity
      property = (*_fluid_viscosity)[_qp][_phase];
      break;

    case 4: // mass fraction
      property = (*_mass_fractions)[_qp][_phase][_fluid_component];
      break;

    case 5: // relative permeability
      property = (*_relative_permeability)[_qp][_phase];
      break;
  }

  return property;
}
