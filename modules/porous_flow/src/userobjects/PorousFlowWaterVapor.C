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
  _gas_fluid_component = 1 - _aqueous_fluid_component;

  // Check that _aqueous_phase_number is <= total number of phases
  if (_aqueous_phase_number >= _num_phases)
    paramError("liquid_phase_number",
               "This value is larger than the possible number of phases ",
               _num_phases);

  // Check that _aqueous_fluid_component is <= total number of fluid components
  if (_aqueous_fluid_component >= _num_components)
    paramError("liquid_fluid_component",
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
                                               FluidStatePhaseEnum & phase_state,
                                               std::vector<FluidStateProperties> & fsp) const
{
  FluidStateProperties & liquid = fsp[_aqueous_phase_number];
  FluidStateProperties & gas = fsp[_gas_phase_number];

  // AD versions of primary variables
  DualReal p = pressure;
  Moose::derivInsert(p.derivatives(), _pidx, 1.0);
  DualReal h = enthalpy;
  Moose::derivInsert(h.derivatives(), _hidx, 1.0);

  DualReal Tsat = 0.0;
  DualReal hl = 0.0;
  DualReal hv = 0.0;

  // Determine the phase state of the system
  if (p.value() >= _p_triple && p.value() <= _p_critical)
  {
    // Saturation temperature at the given pressure
    Tsat = _water_fp.vaporTemperature(p);

    // Enthalpy of saturated liquid and saturated vapor
    hl = _water_fp.h_from_p_T(p, Tsat - dT);
    hv = _water_fp.h_from_p_T(p, Tsat + dT);

    if (h.value() < hl)
      phase_state = FluidStatePhaseEnum::LIQUID;

    else if (h.value() >= hl && h.value() <= hv)
      phase_state = FluidStatePhaseEnum::TWOPHASE;

    else // h > hv
      phase_state = FluidStatePhaseEnum::GAS;
  }
  else // p.value() > _p_critical
  {
    // Check whether the phase point is in the liquid or vapor state
    const DualReal T = _water_fp.T_from_p_h(p, h);

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
      gas.pressure = p + _pc.capillaryPressure(0.0, qp);
      gas.saturation = 1.0;

      const DualReal T = _water_fp.T_from_p_h(gas.pressure, h);

      gas.temperature = T;
      liquid.temperature = T;

      gas.density = _water_fp.rho_from_p_T(gas.pressure, T);
      gas.viscosity = _water_fp.mu_from_p_T(gas.pressure, T);
      gas.enthalpy = h;
      gas.internal_energy = _water_fp.e_from_p_T(gas.pressure, T);

      break;
    }

    case FluidStatePhaseEnum::LIQUID:
    {
      const DualReal T = _water_fp.T_from_p_h(p, h);

      liquid.pressure = p;
      liquid.temperature = T;
      liquid.density = _water_fp.rho_from_p_T(p, T);
      liquid.viscosity = _water_fp.mu_from_p_T(p, T);
      liquid.enthalpy = h;
      liquid.internal_energy = _water_fp.e_from_p_T(p, T);
      liquid.saturation = 1.0;

      break;
    }

    case FluidStatePhaseEnum::TWOPHASE:
    {
      // Latent heat of vaporization
      const DualReal hvl = hv - hl;

      // Vapor quality
      const DualReal X = (h - hl) / hvl;

      // Perturbed saturation temperature to ensure that the correct
      // phase properties are calculated
      const DualReal Tsatl = Tsat - dT;
      const DualReal Tsatv = Tsat + dT;

      // Density
      const DualReal rhol = _water_fp.rho_from_p_T(p, Tsatl);
      const DualReal rhov = _water_fp.rho_from_p_T(p, Tsatv);

      // Vapor (gas) saturation
      const DualReal satv = X * rhol / (rhov + X * (rhol - rhov));

      gas.temperature = Tsat;
      gas.density = rhov;
      gas.viscosity = _water_fp.mu_from_p_T(p, Tsatv);
      gas.enthalpy = hv;
      gas.internal_energy = _water_fp.e_from_p_T(p, Tsatv);
      gas.saturation = satv;

      liquid.temperature = Tsat;
      liquid.density = rhol;
      liquid.viscosity = _water_fp.mu_from_p_T(p, Tsatl);
      liquid.enthalpy = hl;
      liquid.internal_energy = _water_fp.e_from_p_T(p, Tsatl);
      liquid.saturation = 1.0 - satv;

      liquid.pressure = p;
      gas.pressure = p + _pc.capillaryPressure(liquid.saturation, qp);

      break;
    }
  }
}
