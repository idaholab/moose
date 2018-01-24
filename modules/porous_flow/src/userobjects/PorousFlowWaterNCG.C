//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowWaterNCG.h"
#include "Water97FluidProperties.h"

template <>
InputParameters
validParams<PorousFlowWaterNCG>()
{
  InputParameters params = validParams<PorousFlowFluidStateBase>();
  params.addRequiredParam<UserObjectName>("water_fp", "The name of the user object for water");
  params.addRequiredParam<UserObjectName>(
      "gas_fp", "The name of the user object for the non-condensable gas");
  params.addClassDescription("Fluid state class for water and non-condensable gas");
  return params;
}

PorousFlowWaterNCG::PorousFlowWaterNCG(const InputParameters & parameters)
  : PorousFlowFluidStateBase(parameters),
    _water_fp(getUserObject<Water97FluidProperties>("water_fp")),
    _ncg_fp(getUserObject<SinglePhaseFluidPropertiesPT>("gas_fp")),
    _Mh2o(_water_fp.molarMass()),
    _Mncg(_ncg_fp.molarMass()),
    _water_triple_temperature(_water_fp.triplePointTemperature()),
    _water_critical_temperature(_water_fp.criticalTemperature())
{
  // Check that the correct FluidProperties UserObjects have been provided
  if (_water_fp.fluidName() != "water")
    mooseError("Only a valid water FluidProperties UserObject can be provided in water_fp");
}

std::string
PorousFlowWaterNCG::fluidStateName() const
{
  return "water-ncg";
}

void
PorousFlowWaterNCG::thermophysicalProperties(Real pressure,
                                             Real temperature,
                                             Real z,
                                             std::vector<FluidStateProperties> & fsp) const
{
  FluidStateProperties & liquid = fsp[_aqueous_phase_number];
  FluidStateProperties & gas = fsp[_gas_phase_number];

  // Check whether the input temperature is within the region of validity
  checkVariables(temperature);

  // Clear all of the FluidStateProperties data
  clearFluidStateProperties(fsp);

  FluidStatePhaseEnum phase_state;
  massFractions(pressure, temperature, z, phase_state, fsp);

  switch (phase_state)
  {
    case FluidStatePhaseEnum::GAS:
    {
      // Set the gas saturations
      gas.saturation = 1.0;

      // Calculate gas properties
      gasProperties(pressure, temperature, fsp);

      break;
    }

    case FluidStatePhaseEnum::LIQUID:
    {
      // Calculate the liquid properties
      Real liquid_pressure = pressure - _pc_uo.capillaryPressure(1.0);
      liquidProperties(liquid_pressure, temperature, fsp);

      break;
    }

    case FluidStatePhaseEnum::TWOPHASE:
    {
      // Calculate the gas properties
      gasProperties(pressure, temperature, fsp);

      // Calculate the saturation
      saturationTwoPhase(pressure, temperature, z, fsp);

      // Calculate the liquid properties
      Real liquid_pressure = pressure - _pc_uo.capillaryPressure(1.0 - gas.saturation);
      liquidProperties(liquid_pressure, temperature, fsp);

      break;
    }
  }

  // Liquid saturations can now be set
  liquid.saturation = 1.0 - gas.saturation;
  liquid.dsaturation_dp = -gas.dsaturation_dp;
  liquid.dsaturation_dT = -gas.dsaturation_dT;
  liquid.dsaturation_dz = -gas.dsaturation_dz;

  // Save pressures to FluidStateProperties object
  gas.pressure = pressure;
  liquid.pressure = pressure - _pc_uo.capillaryPressure(liquid.saturation);
}

void
PorousFlowWaterNCG::massFractions(Real pressure,
                                  Real temperature,
                                  Real z,
                                  FluidStatePhaseEnum & phase_state,
                                  std::vector<FluidStateProperties> & fsp) const
{
  FluidStateProperties & liquid = fsp[_aqueous_phase_number];
  FluidStateProperties & gas = fsp[_gas_phase_number];

  // Equilibrium mass fraction of NCG in liquid and H2O in gas phases
  Real Xncg, dXncg_dp, dXncg_dT, Yh2o, dYh2o_dp, dYh2o_dT;
  equilibriumMassFractions(
      pressure, temperature, Xncg, dXncg_dp, dXncg_dT, Yh2o, dYh2o_dp, dYh2o_dT);

  Real Yncg = 1.0 - Yh2o;
  Real dYncg_dp = -dYh2o_dp;
  Real dYncg_dT = -dYh2o_dT;

  // Determine which phases are present based on the value of z
  phaseState(z, Xncg, Yncg, phase_state);

  // The equilibrium mass fractions calculated above are only correct in the two phase
  // state. If only liquid or gas phases are present, the mass fractions are given by
  // the total mass fraction z.
  Real Xh2o = 0.0;
  Real dXncg_dz = 0.0, dYncg_dz = 0.0;

  switch (phase_state)
  {
    case FluidStatePhaseEnum::LIQUID:
    {
      Xncg = z;
      Yncg = 0.0;
      Xh2o = 1.0 - z;
      Yh2o = 0.0;
      dXncg_dp = 0.0;
      dXncg_dT = 0.0;
      dXncg_dz = 1.0;
      dYncg_dp = 0.0;
      dYncg_dT = 0.0;
      break;
    }

    case FluidStatePhaseEnum::GAS:
    {
      Xncg = 0.0;
      Yncg = z;
      Yh2o = 1.0 - z;
      dXncg_dp = 0.0;
      dXncg_dT = 0.0;
      dYncg_dz = 1.0;
      dYncg_dp = 0.0;
      dYncg_dT = 0.0;
      break;
    }

    case FluidStatePhaseEnum::TWOPHASE:
    {
      // Keep equilibrium mass fractions
      Xh2o = 1.0 - Xncg;
      break;
    }
  }

  // Save the mass fractions in the FluidStateMassFractions object
  liquid.mass_fraction[_aqueous_fluid_component] = Xh2o;
  liquid.mass_fraction[_gas_fluid_component] = Xncg;
  gas.mass_fraction[_aqueous_fluid_component] = Yh2o;
  gas.mass_fraction[_gas_fluid_component] = Yncg;

  // Save the derivatives wrt PorousFlow variables
  liquid.dmass_fraction_dp[_aqueous_fluid_component] = -dXncg_dp;
  liquid.dmass_fraction_dp[_gas_fluid_component] = dXncg_dp;
  liquid.dmass_fraction_dT[_aqueous_fluid_component] = -dXncg_dT;
  liquid.dmass_fraction_dT[_gas_fluid_component] = dXncg_dT;
  liquid.dmass_fraction_dz[_aqueous_fluid_component] = -dXncg_dz;
  liquid.dmass_fraction_dz[_gas_fluid_component] = dXncg_dz;

  gas.dmass_fraction_dp[_aqueous_fluid_component] = -dYncg_dp;
  gas.dmass_fraction_dp[_gas_fluid_component] = dYncg_dp;
  gas.dmass_fraction_dT[_aqueous_fluid_component] = -dYncg_dT;
  gas.dmass_fraction_dT[_gas_fluid_component] = dYncg_dT;
  gas.dmass_fraction_dz[_aqueous_fluid_component] = -dYncg_dz;
  gas.dmass_fraction_dz[_gas_fluid_component] = dYncg_dz;
}

void
PorousFlowWaterNCG::gasProperties(Real pressure,
                                  Real temperature,
                                  std::vector<FluidStateProperties> & fsp) const
{
  FluidStateProperties & liquid = fsp[_aqueous_phase_number];
  FluidStateProperties & gas = fsp[_gas_phase_number];

  Real psat, dpsat_dT;
  _water_fp.vaporPressure_dT(temperature, psat, dpsat_dT);

  Real Yncg = gas.mass_fraction[_gas_fluid_component];
  Real dYncg_dp = gas.dmass_fraction_dp[_gas_fluid_component];
  Real dYncg_dT = gas.dmass_fraction_dT[_gas_fluid_component];
  Real dYncg_dz = gas.dmass_fraction_dz[_gas_fluid_component];

  Real Xncg = liquid.mass_fraction[_gas_fluid_component];
  Real dXncg_dp = liquid.dmass_fraction_dp[_gas_fluid_component];
  Real dXncg_dT = liquid.dmass_fraction_dT[_gas_fluid_component];
  Real dXncg_dz = liquid.dmass_fraction_dz[_gas_fluid_component];

  Real ncg_density, dncg_density_dp0, dncg_density_dT;
  Real vapor_density, dvapor_density_dp0, dvapor_density_dT;
  // NCG density calculated using partial pressure Yncg * gas_poreressure (Dalton's law)
  _ncg_fp.rho_dpT(Yncg * pressure, temperature, ncg_density, dncg_density_dp0, dncg_density_dT);
  // Vapor density calculated using partial pressure X1 * psat (Raoult's law)
  _water_fp.rho_dpT(
      (1.0 - Xncg) * psat, temperature, vapor_density, dvapor_density_dp0, dvapor_density_dT);

  // Derivative wrt z by the chain rule
  Real dncg_density_dz = dYncg_dz * pressure * dncg_density_dp0;
  Real dvapor_density_dz = -dXncg_dz * psat * dvapor_density_dp0;
  // The derivatives wrt pressure and temperature above also depend on the derivative
  // of the pressure variable using the chain rule
  Real dncg_density_dp = (Yncg + dYncg_dp * pressure) * dncg_density_dp0;
  dncg_density_dT += dYncg_dT * pressure * dncg_density_dp0;
  Real dvapor_density_dp = -dXncg_dp * psat * dvapor_density_dp0;
  dvapor_density_dT += ((1.0 - Xncg) * dpsat_dT - dXncg_dT * psat) * dvapor_density_dp0;

  gas.density = ncg_density + vapor_density;
  gas.ddensity_dp = dncg_density_dp + dvapor_density_dp;
  gas.ddensity_dT = dncg_density_dT + dvapor_density_dT;
  gas.ddensity_dz = dncg_density_dz + dvapor_density_dz;

  // Viscosity
  Real ncg_viscosity, dncg_viscosity_drho, dncg_viscosity_dT;
  Real vapor_viscosity, dvapor_viscosity_drho, dvapor_viscosity_dT;
  _ncg_fp.mu_drhoT_from_rho_T(ncg_density,
                              temperature,
                              dncg_density_dT,
                              ncg_viscosity,
                              dncg_viscosity_drho,
                              dncg_viscosity_dT);
  _water_fp.mu_drhoT_from_rho_T(vapor_density,
                                temperature,
                                dvapor_density_dT,
                                vapor_viscosity,
                                dvapor_viscosity_drho,
                                dvapor_viscosity_dT);

  // Assume that the viscosity of the gas phase is a weighted sum of the
  // individual viscosities
  gas.viscosity = Yncg * ncg_viscosity + (1.0 - Yncg) * vapor_viscosity;
  gas.dviscosity_dp = dYncg_dp * (ncg_viscosity - vapor_viscosity) +
                      Yncg * dncg_viscosity_drho * dncg_density_dp +
                      (1.0 - Yncg) * dvapor_viscosity_drho * dvapor_density_dp;
  gas.dviscosity_dT = dYncg_dT * (ncg_viscosity - vapor_viscosity) + Yncg * dncg_viscosity_dT +
                      (1.0 - Yncg) * dvapor_viscosity_dT;
  gas.dviscosity_dz = dYncg_dz * (ncg_viscosity - vapor_viscosity) +
                      Yncg * dncg_viscosity_drho * dncg_density_dz +
                      (1.0 - Yncg) * dvapor_viscosity_drho * dvapor_density_dz;
}

void
PorousFlowWaterNCG::liquidProperties(Real pressure,
                                     Real temperature,
                                     std::vector<FluidStateProperties> & fsp) const
{
  FluidStateProperties & liquid = fsp[_aqueous_phase_number];

  // Calculate liquid density and viscosity if in the two phase or single phase
  // liquid region, assuming they are not affected by the presence of dissolved
  // NCG. Note: the (small) contribution due to derivative of capillary pressure
  // wrt pressure (using the chain rule) is not implemented.
  Real liquid_density, dliquid_density_dp, dliquid_density_dT;
  _water_fp.rho_dpT(pressure, temperature, liquid_density, dliquid_density_dp, dliquid_density_dT);

  liquid.density = liquid_density;
  liquid.ddensity_dp = dliquid_density_dp;
  liquid.ddensity_dT = dliquid_density_dT;
  liquid.ddensity_dz = 0.0;

  Real liquid_viscosity, dliquid_viscosity_drho, dliquid_viscosity_dT;
  _water_fp.mu_drhoT_from_rho_T(liquid_density,
                                temperature,
                                dliquid_density_dT,
                                liquid_viscosity,
                                dliquid_viscosity_drho,
                                dliquid_viscosity_dT);

  // The derivative of viscosity wrt pressure is given by the chain rule
  liquid.viscosity = liquid_viscosity;
  liquid.dviscosity_dp = dliquid_viscosity_drho * dliquid_density_dp;
  liquid.dviscosity_dT = dliquid_viscosity_dT;
  liquid.dviscosity_dz = 0.0;
}

void
PorousFlowWaterNCG::saturationTwoPhase(Real pressure,
                                       Real temperature,
                                       Real z,
                                       std::vector<FluidStateProperties> & fsp) const
{
  FluidStateProperties & liquid = fsp[_aqueous_phase_number];
  FluidStateProperties & gas = fsp[_gas_phase_number];

  // Mass fractions
  Real Xncg = liquid.mass_fraction[_gas_fluid_component];
  Real dXncg_dp = liquid.dmass_fraction_dp[_gas_fluid_component];
  Real dXncg_dT = liquid.dmass_fraction_dT[_gas_fluid_component];

  Real Yncg = gas.mass_fraction[_gas_fluid_component];
  Real dYncg_dp = gas.dmass_fraction_dp[_gas_fluid_component];
  Real dYncg_dT = gas.dmass_fraction_dT[_gas_fluid_component];

  // Calculate the vapor mass fraction
  Real K0 = Yncg / Xncg;
  Real K1 = (1.0 - Yncg) / (1.0 - Xncg);
  Real vapor_mass_fraction = vaporMassFraction(z, K0, K1);

  // Liquid density is a function of gas saturation through the capillary pressure
  // curve, so approximate liquid pressure as gas pressure
  Real liquid_pressure = pressure;

  Real liquid_density, liquid_ddensity_dp, liquid_ddensity_dT;
  _water_fp.rho_dpT(
      liquid_pressure, temperature, liquid_density, liquid_ddensity_dp, liquid_ddensity_dT);

  // The gas saturation in the two phase case
  gas.saturation = vapor_mass_fraction * liquid_density /
                   (gas.density + vapor_mass_fraction * (liquid_density - gas.density));

  Real dv_dz = (K1 - K0) / ((K0 - 1.0) * (K1 - 1.0));
  Real denominator = (gas.density + vapor_mass_fraction * (liquid_density - gas.density)) *
                     (gas.density + vapor_mass_fraction * (liquid_density - gas.density));

  Real ds_dz = gas.density * liquid_density * dv_dz / denominator;

  Real dK0_dp = (Xncg * dYncg_dp - Yncg * dXncg_dp) / Xncg / Xncg;
  Real dK0_dT = (Xncg * dYncg_dT - Yncg * dXncg_dT) / Xncg / Xncg;

  Real dK1_dp = ((1.0 - Yncg) * dXncg_dp - (1.0 - Xncg) * dYncg_dp) / (1.0 - Xncg) / (1.0 - Xncg);
  Real dK1_dT = ((1.0 - Yncg) * dXncg_dT - (1.0 - Xncg) * dYncg_dT) / (1.0 - Xncg) / (1.0 - Xncg);

  Real dv_dp = z * dK1_dp / (K1 - 1.0) / (K1 - 1.0) + (1.0 - z) * dK0_dp / (K0 - 1.0) / (K0 - 1.0);

  Real ds_dp = gas.density * liquid_density * dv_dp +
               vapor_mass_fraction * (1.0 - vapor_mass_fraction) *
                   (gas.density * liquid_ddensity_dp - gas.ddensity_dp * liquid_density);
  ds_dp /= denominator;

  Real dv_dT = z * dK1_dT / (K1 - 1.0) / (K1 - 1.0) + (1.0 - z) * dK0_dT / (K0 - 1.0) / (K0 - 1.0);

  Real ds_dT = gas.density * liquid_density * dv_dT +
               vapor_mass_fraction * (1.0 - vapor_mass_fraction) *
                   (gas.density * liquid_ddensity_dT - gas.ddensity_dT * liquid_density);
  ds_dT /= denominator;

  gas.dsaturation_dp = ds_dp;
  gas.dsaturation_dT = ds_dT;
  gas.dsaturation_dz = ds_dz;
}

void
PorousFlowWaterNCG::equilibriumMassFractions(Real pressure,
                                             Real temperature,
                                             Real & Xncg,
                                             Real & dXncg_dp,
                                             Real & dXncg_dT,
                                             Real & Yh2o,
                                             Real & dYh2o_dp,
                                             Real & dYh2o_dT) const
{
  // Equilibrium constants for each component (Henry's law for the NCG
  // component, and Raoult's law for water).
  Real Kh, dKh_dT, psat, dpsat_dT;
  _ncg_fp.henryConstant_dT(temperature, Kh, dKh_dT);
  _water_fp.vaporPressure_dT(temperature, psat, dpsat_dT);
  Real Kncg = Kh / pressure;
  Real Kh2o = psat / pressure;

  // The mole fractions for the NCG component in the two component
  // case can be expressed in terms of the equilibrium constants only
  Real xncg = (1.0 - Kh2o) / (Kncg - Kh2o);
  Real yncg = Kncg * xncg;

  // Convert mole fractions to mass fractions
  Xncg = moleFractionToMassFraction(xncg);
  Yh2o = 1.0 - moleFractionToMassFraction(yncg);

  // Derivatives of mass fractions wrt PorousFlow variables
  Real dKncg_dp = -Kh / pressure / pressure;
  Real dKncg_dT = dKh_dT / pressure;

  Real dKh2o_dp = -psat / pressure / pressure;
  Real dKh2o_dT = dpsat_dT / pressure;

  Real dxncg_dp = ((Kh2o - 1.0) * dKncg_dp + (1 - Kncg) * dKh2o_dp) / (Kncg - Kh2o) / (Kncg - Kh2o);
  Real dyncg_dp = xncg * dKncg_dp + Kncg * dxncg_dp;
  Real dxncg_dT = ((Kh2o - 1.0) * dKncg_dT + (1 - Kncg) * dKh2o_dT) / (Kncg - Kh2o) / (Kncg - Kh2o);
  Real dyncg_dT = xncg * dKncg_dT + Kncg * dxncg_dT;

  Real dXncg_dxncg =
      _Mncg * _Mh2o / (xncg * _Mncg + (1.0 - xncg) * _Mh2o) / (xncg * _Mncg + (1.0 - xncg) * _Mh2o);
  Real dYncg_dyncg =
      _Mncg * _Mh2o / (yncg * _Mncg + (1.0 - yncg) * _Mh2o) / (yncg * _Mncg + (1.0 - yncg) * _Mh2o);

  dXncg_dp = dXncg_dxncg * dxncg_dp;
  dXncg_dT = dXncg_dxncg * dxncg_dT;
  dYh2o_dp = -dYncg_dyncg * dyncg_dp;
  dYh2o_dT = -dYncg_dyncg * dyncg_dT;
}

Real
PorousFlowWaterNCG::moleFractionToMassFraction(Real xmol) const
{
  return xmol * _Mncg / (xmol * _Mncg + (1.0 - xmol) * _Mh2o);
}

void
PorousFlowWaterNCG::checkVariables(Real temperature) const
{
  // Check whether the input temperature is within the region of validity of this equation
  // of state (T_triple <= T <= T_critical)
  if (temperature < _water_triple_temperature || temperature > _water_critical_temperature)
    mooseError("PorousFlowWaterNCG: Temperature is outside range 273.16 K <= T "
               "<= 647.096 K");
}

Real
PorousFlowWaterNCG::enthalpyOfDissolution(Real temperature, Real Kh, Real dKh_dT) const
{
  return -_R * temperature * temperature * _Mncg * dKh_dT / Kh;
}

Real
PorousFlowWaterNCG::totalMassFraction(Real pressure, Real temperature, Real saturation) const
{
  // Check whether the input temperature is within the region of validity
  checkVariables(temperature);

  // FluidStateProperties data structure
  std::vector<FluidStateProperties> fsp(_num_phases, FluidStateProperties(_num_components));
  FluidStateProperties & liquid = fsp[_aqueous_phase_number];
  FluidStateProperties & gas = fsp[_gas_phase_number];

  // Calculate equilibrium mass fractions in the two-phase state
  Real Xncg, dXncg_dp, dXncg_dT, Yh2o, dYh2o_dp, dYh2o_dT;
  equilibriumMassFractions(
      pressure, temperature, Xncg, dXncg_dp, dXncg_dT, Yh2o, dYh2o_dp, dYh2o_dT);

  // Save the mass fractions in the FluidStateMassFractions object
  Real Yncg = 1.0 - Yh2o;
  liquid.mass_fraction[_aqueous_fluid_component] = 1.0 - Xncg;
  liquid.mass_fraction[_gas_fluid_component] = Xncg;
  gas.mass_fraction[_aqueous_fluid_component] = Yh2o;
  gas.mass_fraction[_gas_fluid_component] = Yncg;

  // Gas properties
  gasProperties(pressure, temperature, fsp);

  // Liquid properties
  Real liquid_saturation = 1.0 - saturation;
  Real liquid_pressure = pressure - _pc_uo.capillaryPressure(liquid_saturation);
  liquidProperties(liquid_pressure, temperature, fsp);

  // The total mass fraction of ncg (z) can now be calculated
  Real z = (saturation * gas.density * Yncg + liquid_saturation * liquid.density * Xncg) /
           (saturation * gas.density + liquid_saturation * liquid.density);

  return z;
}
