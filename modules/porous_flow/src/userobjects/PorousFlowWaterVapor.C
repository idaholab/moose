//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowWaterVapor.h"
#include "SinglePhaseFluidProperties.h"
#include "Conversion.h"
#include "MooseUtils.h"

registerMooseObject("PorousFlowApp", PorousFlowWaterVapor);

InputParameters
PorousFlowWaterVapor::validParams()
{
  InputParameters params = PorousFlowFluidStateSingleComponentBase::validParams();
  params.addRequiredParam<UserObjectName>("water_fp", "The name of the user object for water");
  params.addClassDescription("Fluid state class for water and vapor");
  return params;
}

PorousFlowWaterVapor::PorousFlowWaterVapor(const InputParameters & parameters)
  : PorousFlowFluidStateSingleComponentBase(parameters),
    _water_fp(getUserObject<SinglePhaseFluidProperties>("water_fp")),
    _Mh2o(_water_fp.molarMass()),
    _p_triple(_water_fp.triplePointPressure()),
    _p_critical(_water_fp.criticalPressure()),
    _T_triple(_water_fp.triplePointTemperature()),
    _T_critical(_water_fp.criticalTemperature())
{
  // Check that the correct FluidProperties UserObjects have been provided
  if (_water_fp.fluidName() != "water")
    paramError("water_fp", "A valid water FluidProperties UserObject must be provided in water_fp");

  // Set the number of phases and components, and their indexes
  _num_phases = 2;
  _num_components = 1;
  _gas_phase_number = 1 - _aqueous_phase_number;

  // Check that _aqueous_phase_number is <= total number of phases
  if (_aqueous_phase_number >= _num_phases)
    paramError("liquid_phase_number",
               "This value is larger than the possible number of phases ",
               _num_phases);

  // Check that _fluid_component is <= total number of fluid components
  if (_fluid_component >= _num_components)
    paramError("fluid_component",
               "This value is larger than the possible number of fluid components",
               _num_components);

  _empty_fsp = FluidStateProperties(_num_components);
}

std::string
PorousFlowWaterVapor::fluidStateName() const
{
  return "water-vapor";
}

void
PorousFlowWaterVapor::thermophysicalProperties(Real pressure,
                                               Real enthalpy,
                                               unsigned int qp,
                                               std::vector<FluidStateProperties> & fsp) const
{
  // AD versions of primary variables
  ADReal p = pressure;
  Moose::derivInsert(p.derivatives(), _pidx, 1.0);
  ADReal h = enthalpy;
  Moose::derivInsert(h.derivatives(), _hidx, 1.0);

  thermophysicalProperties(p, h, qp, fsp);
}

void
PorousFlowWaterVapor::thermophysicalProperties(const ADReal & pressure,
                                               const ADReal & enthalpy,
                                               unsigned int qp,
                                               std::vector<FluidStateProperties> & fsp) const
{
  FluidStatePhaseEnum phase_state;

  thermophysicalProperties(pressure, enthalpy, qp, phase_state, fsp);
}

void
PorousFlowWaterVapor::thermophysicalProperties(const ADReal & pressure,
                                               const ADReal & enthalpy,
                                               unsigned int qp,
                                               FluidStatePhaseEnum & phase_state,
                                               std::vector<FluidStateProperties> & fsp) const
{
  FluidStateProperties & liquid = fsp[_aqueous_phase_number];
  FluidStateProperties & gas = fsp[_gas_phase_number];

  ADReal Tsat = 0.0;
  ADReal hl = 0.0;
  ADReal hv = 0.0;

  // Determine the phase state of the system
  if (pressure.value() >= _p_triple && pressure.value() <= _p_critical)
  {
    // Saturation temperature at the given pressure
    Tsat = _water_fp.vaporTemperature(pressure);

    // Enthalpy of saturated liquid and saturated vapor
    hl = _water_fp.h_from_p_T(pressure, Tsat - _dT);
    hv = _water_fp.h_from_p_T(pressure, Tsat + _dT);

    if (enthalpy.value() < hl.value())
      phase_state = FluidStatePhaseEnum::LIQUID;

    else if (enthalpy.value() >= hl.value() && enthalpy.value() <= hv.value())
      phase_state = FluidStatePhaseEnum::TWOPHASE;

    else // h > hv
      phase_state = FluidStatePhaseEnum::GAS;
  }
  else // p.value() > _p_critical
  {
    // Check whether the phase point is in the liquid or vapor state
    const ADReal T = _water_fp.T_from_p_h(pressure, enthalpy);

    if (T.value() <= _T_critical)
      phase_state = FluidStatePhaseEnum::LIQUID;
    else
    {
      // The supercritical state is treated as a gas
      phase_state = FluidStatePhaseEnum::GAS;
    }
  }

  // Calculate the properties for each phase as required
  switch (phase_state)
  {
    case FluidStatePhaseEnum::GAS:
    {
      gas.pressure = pressure + _pc.capillaryPressure(0.0, qp);
      gas.saturation = 1.0;

      const ADReal T = _water_fp.T_from_p_h(gas.pressure, enthalpy);

      gas.temperature = T;
      liquid.temperature = T;

      gas.density = _water_fp.rho_from_p_T(gas.pressure, T);
      gas.viscosity = _water_fp.mu_from_p_T(gas.pressure, T);
      gas.enthalpy = enthalpy;
      gas.internal_energy = _water_fp.e_from_p_T(gas.pressure, T);
      gas.mass_fraction[_fluid_component] = 1.0;

      break;
    }

    case FluidStatePhaseEnum::LIQUID:
    {
      const ADReal T = _water_fp.T_from_p_h(pressure, enthalpy);

      liquid.pressure = pressure;
      liquid.temperature = T;
      liquid.density = _water_fp.rho_from_p_T(pressure, T);
      liquid.viscosity = _water_fp.mu_from_p_T(pressure, T);
      liquid.enthalpy = enthalpy;
      liquid.internal_energy = _water_fp.e_from_p_T(pressure, T);
      liquid.saturation = 1.0;
      liquid.mass_fraction[_fluid_component] = 1.0;

      break;
    }

    case FluidStatePhaseEnum::TWOPHASE:
    {
      // Latent heat of vaporization
      const ADReal hvl = hv - hl;

      // Vapor quality
      const ADReal X = (enthalpy - hl) / hvl;

      // Perturbed saturation temperature to ensure that the correct
      // phase properties are calculated
      const ADReal Tsatl = Tsat - _dT;
      const ADReal Tsatv = Tsat + _dT;

      // Density
      const ADReal rhol = _water_fp.rho_from_p_T(pressure, Tsatl);
      const ADReal rhov = _water_fp.rho_from_p_T(pressure, Tsatv);

      // Vapor (gas) saturation
      const ADReal satv = X * rhol / (rhov + X * (rhol - rhov));

      gas.temperature = Tsat;
      gas.density = rhov;
      gas.viscosity = _water_fp.mu_from_p_T(pressure, Tsatv);
      gas.enthalpy = hv;
      gas.internal_energy = _water_fp.e_from_p_T(pressure, Tsatv);
      gas.saturation = satv;

      liquid.temperature = Tsat;
      liquid.density = rhol;
      liquid.viscosity = _water_fp.mu_from_p_T(pressure, Tsatl);
      liquid.enthalpy = hl;
      liquid.internal_energy = _water_fp.e_from_p_T(pressure, Tsatl);
      liquid.saturation = 1.0 - satv;

      liquid.pressure = pressure;
      gas.pressure = pressure + _pc.capillaryPressure(liquid.saturation, qp);

      gas.mass_fraction[_fluid_component] = 1.0;
      liquid.mass_fraction[_fluid_component] = 1.0;

      break;
    }
  }
}
