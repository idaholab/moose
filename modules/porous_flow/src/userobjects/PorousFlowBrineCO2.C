//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowBrineCO2.h"
#include "BrineFluidProperties.h"
#include "SinglePhaseFluidPropertiesPT.h"

template <>
InputParameters
validParams<PorousFlowBrineCO2>()
{
  InputParameters params = validParams<PorousFlowFluidStateBase>();
  params.addRequiredParam<UserObjectName>("brine_fp", "The name of the user object for brine");
  params.addRequiredParam<UserObjectName>("co2_fp", "The name of the user object for CO2");
  params.addClassDescription("Fluid state class for brine and CO2");
  return params;
}

PorousFlowBrineCO2::PorousFlowBrineCO2(const InputParameters & parameters)
  : PorousFlowFluidStateBase(parameters),
    _brine_fp(getUserObject<BrineFluidProperties>("brine_fp")),
    _co2_fp(getUserObject<SinglePhaseFluidPropertiesPT>("co2_fp")),
    _water_fp(_brine_fp.getComponent(BrineFluidProperties::WATER)),
    _Mh2o(_brine_fp.molarMassH2O()),
    _invMh2o(1.0 / _Mh2o),
    _Mco2(_co2_fp.molarMass()),
    _Mnacl(_brine_fp.molarMassNaCl()),
    _Rbar(_R * 10.0)
{
  // Check that the correct FluidProperties UserObjects have been provided
  if (_co2_fp.fluidName() != "co2")
    mooseError("Only a valid CO2 FluidProperties UserObject can be provided in co2_fp");
  if (_brine_fp.fluidName() != "brine")
    mooseError("Only a valid brine FluidProperties UserObject can be provided in brine_fp");
}

std::string
PorousFlowBrineCO2::fluidStateName() const
{
  return "brine-co2";
}

void
PorousFlowBrineCO2::thermophysicalProperties(Real pressure,
                                             Real temperature,
                                             Real xnacl,
                                             Real z,
                                             std::vector<FluidStateProperties> & fsp) const
{
  FluidStateProperties & liquid = fsp[_aqueous_phase_number];
  FluidStateProperties & gas = fsp[_gas_phase_number];

  // Check whether the input temperature is within the region of validity
  checkVariables(pressure, temperature);

  // Clear all of the FluidStateProperties data
  clearFluidStateProperties(fsp);

  FluidStatePhaseEnum phase_state;
  massFractions(pressure, temperature, xnacl, z, phase_state, fsp);

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
      liquidProperties(liquid_pressure, temperature, xnacl, fsp);

      break;
    }

    case FluidStatePhaseEnum::TWOPHASE:
    {
      // Calculate the gas properties
      gasProperties(pressure, temperature, fsp);

      // Calculate the saturation
      saturationTwoPhase(pressure, temperature, xnacl, z, fsp);

      // Calculate the liquid properties
      Real liquid_pressure = pressure - _pc_uo.capillaryPressure(1.0 - gas.saturation);
      liquidProperties(liquid_pressure, temperature, xnacl, fsp);

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
PorousFlowBrineCO2::massFractions(Real pressure,
                                  Real temperature,
                                  Real xnacl,
                                  Real z,
                                  FluidStatePhaseEnum & phase_state,
                                  std::vector<FluidStateProperties> & fsp) const
{
  FluidStateProperties & liquid = fsp[_aqueous_phase_number];
  FluidStateProperties & gas = fsp[_gas_phase_number];

  // Equilibrium mass fraction of CO2 in liquid and H2O in gas phases
  Real Xco2, dXco2_dp, dXco2_dT, Yh2o, dYh2o_dp, dYh2o_dT;
  equilibriumMassFractions(
      pressure, temperature, xnacl, Xco2, dXco2_dp, dXco2_dT, Yh2o, dYh2o_dp, dYh2o_dT);

  Real Yco2 = 1.0 - Yh2o;
  Real dYco2_dp = -dYh2o_dp;
  Real dYco2_dT = -dYh2o_dT;

  // Determine which phases are present based on the value of z
  phaseState(z, Xco2, Yco2, phase_state);

  // The equilibrium mass fractions calculated above are only correct in the two phase
  // state. If only liquid or gas phases are present, the mass fractions are given by
  // the total mass fraction z
  Real Xh2o = 0.0;
  Real dXco2_dz = 0.0, dYco2_dz = 0.0;

  switch (phase_state)
  {
    case FluidStatePhaseEnum::LIQUID:
    {
      Xco2 = z;
      Yco2 = 0.0;
      Xh2o = 1.0 - z;
      Yh2o = 0.0;
      dXco2_dp = 0.0;
      dXco2_dT = 0.0;
      dXco2_dz = 1.0;
      dYco2_dp = 0.0;
      dYco2_dT = 0.0;
      break;
    }

    case FluidStatePhaseEnum::GAS:
    {
      Xco2 = 0.0;
      Yco2 = z;
      Yh2o = 1.0 - z;
      dXco2_dp = 0.0;
      dXco2_dT = 0.0;
      dYco2_dz = 1.0;
      dYco2_dp = 0.0;
      dYco2_dT = 0.0;
      break;
    }

    case FluidStatePhaseEnum::TWOPHASE:
    {
      // Keep equilibrium mass fractions
      Xh2o = 1.0 - Xco2;
      break;
    }
  }

  // Save the mass fractions in the FluidStateMassFractions object
  liquid.mass_fraction[_aqueous_fluid_component] = Xh2o;
  liquid.mass_fraction[_gas_fluid_component] = Xco2;
  gas.mass_fraction[_aqueous_fluid_component] = Yh2o;
  gas.mass_fraction[_gas_fluid_component] = Yco2;

  // Save the derivatives wrt PorousFlow variables
  liquid.dmass_fraction_dp[_aqueous_fluid_component] = -dXco2_dp;
  liquid.dmass_fraction_dp[_gas_fluid_component] = dXco2_dp;
  liquid.dmass_fraction_dT[_aqueous_fluid_component] = -dXco2_dT;
  liquid.dmass_fraction_dT[_gas_fluid_component] = dXco2_dT;
  liquid.dmass_fraction_dz[_aqueous_fluid_component] = -dXco2_dz;
  liquid.dmass_fraction_dz[_gas_fluid_component] = dXco2_dz;

  gas.dmass_fraction_dp[_aqueous_fluid_component] = -dYco2_dp;
  gas.dmass_fraction_dp[_gas_fluid_component] = dYco2_dp;
  gas.dmass_fraction_dT[_aqueous_fluid_component] = -dYco2_dT;
  gas.dmass_fraction_dT[_gas_fluid_component] = dYco2_dT;
  gas.dmass_fraction_dz[_aqueous_fluid_component] = -dYco2_dz;
  gas.dmass_fraction_dz[_gas_fluid_component] = dYco2_dz;
}

void
PorousFlowBrineCO2::gasProperties(Real pressure,
                                  Real temperature,
                                  std::vector<FluidStateProperties> & fsp) const
{
  FluidStateProperties & gas = fsp[_gas_phase_number];

  // Gas density and viscosity are approximated with pure CO2 - no correction due
  // to the small amount of water vapor is made
  Real co2_density, dco2_density_dp, dco2_density_dT;
  Real co2_viscosity, dco2_viscosity_drho, dco2_viscosity_dT;
  _co2_fp.rho_dpT(pressure, temperature, co2_density, dco2_density_dp, dco2_density_dT);
  _co2_fp.mu_drhoT_from_rho_T(co2_density,
                              temperature,
                              dco2_density_dT,
                              co2_viscosity,
                              dco2_viscosity_drho,
                              dco2_viscosity_dT);

  // Save the values to the FluidStateProperties object. Note that derivatives wrt z are 0
  gas.density = co2_density;
  gas.ddensity_dp = dco2_density_dp;
  gas.ddensity_dT = dco2_density_dT;
  gas.ddensity_dz = 0.0;

  gas.viscosity = co2_viscosity;
  gas.dviscosity_dp = dco2_viscosity_drho * dco2_density_dp;
  gas.dviscosity_dT = dco2_viscosity_dT;
  gas.dviscosity_dz = 0.0;
}

void
PorousFlowBrineCO2::liquidProperties(Real pressure,
                                     Real temperature,
                                     Real xnacl,
                                     std::vector<FluidStateProperties> & fsp) const
{
  FluidStateProperties & liquid = fsp[_aqueous_phase_number];

  // The liquid density includes the density increase due to dissolved CO2
  Real brine_density, dbrine_density_dp, dbrine_density_dT, dbrine_density_dx;
  _brine_fp.rho_dpTx(pressure,
                     temperature,
                     xnacl,
                     brine_density,
                     dbrine_density_dp,
                     dbrine_density_dT,
                     dbrine_density_dx);

  // Mass fraction of CO2 in liquid phase
  Real Xco2 = liquid.mass_fraction[_gas_fluid_component];
  Real dXco2_dp = liquid.dmass_fraction_dp[_gas_fluid_component];
  Real dXco2_dT = liquid.dmass_fraction_dT[_gas_fluid_component];
  Real dXco2_dz = liquid.dmass_fraction_dz[_gas_fluid_component];

  // The liquid density
  Real co2_partial_density, dco2_partial_density_dT;
  partialDensityCO2(temperature, co2_partial_density, dco2_partial_density_dT);

  Real liquid_density = 1.0 / (Xco2 / co2_partial_density + (1.0 - Xco2) / brine_density);

  Real dliquid_density_dp =
      (dXco2_dp / brine_density + (1.0 - Xco2) * dbrine_density_dp / brine_density / brine_density -
       dXco2_dp / co2_partial_density) *
      liquid_density * liquid_density;

  Real dliquid_density_dT =
      (dXco2_dT / brine_density + (1.0 - Xco2) * dbrine_density_dT / brine_density / brine_density -
       dXco2_dT / co2_partial_density +
       Xco2 * dco2_partial_density_dT / co2_partial_density / co2_partial_density) *
      liquid_density * liquid_density;

  Real dliquid_density_dz =
      (dXco2_dz / brine_density - dXco2_dz / co2_partial_density) * liquid_density * liquid_density;

  // Assume that liquid viscosity is just the brine viscosity
  // Note: brine viscosity (and derivatives) requires water density (and derivatives)
  Real water_density, dwater_density_dp, dwater_density_dT;
  _water_fp.rho_dpT(pressure, temperature, water_density, dwater_density_dp, dwater_density_dT);

  Real liquid_viscosity, dliquid_viscosity_drho, dliquid_viscosity_dT, dliquid_viscosity_dx;
  _brine_fp.mu_drhoTx(water_density,
                      temperature,
                      xnacl,
                      dwater_density_dT,
                      liquid_viscosity,
                      dliquid_viscosity_drho,
                      dliquid_viscosity_dT,
                      dliquid_viscosity_dx);

  // Save the values to the FluidStateProperties object
  liquid.density = liquid_density;
  liquid.ddensity_dp = dliquid_density_dp;
  liquid.ddensity_dT = dliquid_density_dT;
  liquid.ddensity_dz = dliquid_density_dz;

  liquid.viscosity = liquid_viscosity;
  liquid.dviscosity_dp = dliquid_viscosity_drho * dwater_density_dp;
  liquid.dviscosity_dT = dliquid_viscosity_dT;
  liquid.dviscosity_dz = 0.0;
}

void
PorousFlowBrineCO2::saturationTwoPhase(Real pressure,
                                       Real temperature,
                                       Real xnacl,
                                       Real z,
                                       std::vector<FluidStateProperties> & fsp) const
{
  FluidStateProperties & liquid = fsp[_aqueous_phase_number];
  FluidStateProperties & gas = fsp[_gas_phase_number];

  // Approximate liquid density as saturation isn't known yet
  Real brine_density, dbrine_density_dp, dbrine_density_dT, dbrine_density_dx;
  _brine_fp.rho_dpTx(pressure,
                     temperature,
                     xnacl,
                     brine_density,
                     dbrine_density_dp,
                     dbrine_density_dT,
                     dbrine_density_dx);

  // Mass fraction of CO2 in liquid phase
  Real Xco2 = liquid.mass_fraction[_gas_fluid_component];
  Real dXco2_dp = liquid.dmass_fraction_dp[_gas_fluid_component];
  Real dXco2_dT = liquid.dmass_fraction_dT[_gas_fluid_component];

  // The liquid density
  Real co2_partial_density, dco2_partial_density_dT;
  partialDensityCO2(temperature, co2_partial_density, dco2_partial_density_dT);

  Real liquid_density = 1.0 / (Xco2 / co2_partial_density + (1.0 - Xco2) / brine_density);

  Real dliquid_density_dp =
      (dXco2_dp / brine_density + (1.0 - Xco2) * dbrine_density_dp / brine_density / brine_density -
       dXco2_dp / co2_partial_density) *
      liquid_density * liquid_density;

  Real dliquid_density_dT =
      (dXco2_dT / brine_density + (1.0 - Xco2) * dbrine_density_dT / brine_density / brine_density -
       dXco2_dT / co2_partial_density +
       Xco2 * dco2_partial_density_dT / co2_partial_density / co2_partial_density) *
      liquid_density * liquid_density;

  Real Yco2 = gas.mass_fraction[_gas_fluid_component];
  Real dYco2_dp = gas.dmass_fraction_dp[_gas_fluid_component];
  Real dYco2_dT = gas.dmass_fraction_dT[_gas_fluid_component];

  // Set mass equilibrium constants used in the calculation of vapor mass fraction
  Real K0 = Yco2 / Xco2;
  Real K1 = (1.0 - Yco2) / (1.0 - Xco2);
  Real vapor_mass_fraction = vaporMassFraction(z, K0, K1);

  // The gas saturation in the two phase case
  gas.saturation = vapor_mass_fraction * liquid_density /
                   (gas.density + vapor_mass_fraction * (liquid_density - gas.density));

  Real dv_dz = (K1 - K0) / ((K0 - 1.0) * (K1 - 1.0));
  Real denominator = (gas.density + vapor_mass_fraction * (liquid_density - gas.density)) *
                     (gas.density + vapor_mass_fraction * (liquid_density - gas.density));

  Real ds_dz = gas.density * liquid_density * dv_dz / denominator;

  Real dK0_dp = (Xco2 * dYco2_dp - Yco2 * dXco2_dp) / Xco2 / Xco2;
  Real dK0_dT = (Xco2 * dYco2_dT - Yco2 * dXco2_dT) / Xco2 / Xco2;

  Real dK1_dp = ((1.0 - Yco2) * dXco2_dp - (1.0 - Xco2) * dYco2_dp) / (1.0 - Xco2) / (1.0 - Xco2);
  Real dK1_dT = ((1.0 - Yco2) * dXco2_dT - (1.0 - Xco2) * dYco2_dT) / (1.0 - Xco2) / (1.0 - Xco2);

  Real dv_dp = z * dK1_dp / (K1 - 1.0) / (K1 - 1.0) + (1.0 - z) * dK0_dp / (K0 - 1.0) / (K0 - 1.0);

  Real ds_dp = gas.density * liquid_density * dv_dp +
               vapor_mass_fraction * (1.0 - vapor_mass_fraction) *
                   (gas.density * dliquid_density_dp - gas.ddensity_dp * liquid_density);
  ds_dp /= denominator;

  Real dv_dT = z * dK1_dT / (K1 - 1.0) / (K1 - 1.0) + (1.0 - z) * dK0_dT / (K0 - 1.0) / (K0 - 1.0);

  Real ds_dT = gas.density * liquid_density * dv_dT +
               vapor_mass_fraction * (1.0 - vapor_mass_fraction) *
                   (gas.density * dliquid_density_dT - gas.ddensity_dT * liquid_density);
  ds_dT /= denominator;

  gas.dsaturation_dp = ds_dp;
  gas.dsaturation_dT = ds_dT;
  gas.dsaturation_dz = ds_dz;
}

void
PorousFlowBrineCO2::equilibriumMassFractions(Real pressure,
                                             Real temperature,
                                             Real xnacl,
                                             Real & Xco2,
                                             Real & dXco2_dp,
                                             Real & dXco2_dT,
                                             Real & Yh2o,
                                             Real & dYh2o_dp,
                                             Real & dYh2o_dT) const
{
  // Pressure in bar
  Real pbar = pressure * 1.0e-5;
  // Pressure minus 1 bar
  Real delta_pbar = pbar - 1.0;

  // Average partial molar volumes (cm^3/mol) as given by Sypcher, Pruess and Ennis-King (2003)
  const Real vCO2 = 32.6;
  const Real vH2O = 18.1;

  // NaCl molality (mol/kg)
  Real mnacl = xnacl / (1.0 - xnacl) / _Mnacl;

  // Equilibrium constants
  Real K0H2O, dK0H2O_dT, K0CO2, dK0CO2_dT;
  equilibriumConstantH2O(temperature, K0H2O, dK0H2O_dT);
  equilibriumConstantCO2(temperature, K0CO2, dK0CO2_dT);

  // Fugacity coefficients
  Real phiH2O, dphiH2O_dp, dphiH2O_dT;
  Real phiCO2, dphiCO2_dp, dphiCO2_dT;
  fugacityCoefficientCO2(pressure, temperature, phiCO2, dphiCO2_dp, dphiCO2_dT);
  fugacityCoefficientH2O(pressure, temperature, phiH2O, dphiH2O_dp, dphiH2O_dT);

  // Activity coefficient
  Real gamma, dgamma_dp, dgamma_dT;
  activityCoefficient(pressure, temperature, xnacl, gamma, dgamma_dp, dgamma_dT);

  Real A = K0H2O / (phiH2O * pbar) * std::exp(delta_pbar * vH2O / (_Rbar * temperature));
  Real B =
      phiCO2 * pbar / (_invMh2o * K0CO2) * std::exp(-delta_pbar * vCO2 / (_Rbar * temperature));

  Real dA_dp = (-1.0e-5 * K0H2O / pbar + 1.0e-5 * vH2O * K0H2O / (_Rbar * temperature) -
                K0H2O * dphiH2O_dp / phiH2O) *
               std::exp(delta_pbar * vH2O / (_Rbar * temperature)) / (pbar * phiH2O);
  Real dB_dp = (1.0e-5 * phiCO2 + pbar * dphiCO2_dp -
                1.0e-5 * vCO2 * pbar * phiCO2 / (_Rbar * temperature)) *
               std::exp(-delta_pbar * vCO2 / (_Rbar * temperature)) / (_invMh2o * K0CO2);

  Real dA_dT = (dK0H2O_dT - dphiH2O_dT * K0H2O / phiH2O -
                delta_pbar * vH2O * K0H2O / (_Rbar * temperature * temperature)) *
               std::exp(delta_pbar * vH2O / (_Rbar * temperature)) / (pbar * phiH2O);
  Real dB_dT = (-pbar * phiCO2 * dK0CO2_dT / K0CO2 + pbar * dphiCO2_dT +
                delta_pbar * vCO2 * pbar * phiCO2 / (_Rbar * temperature * temperature)) *
               std::exp(-delta_pbar * vCO2 / (_Rbar * temperature)) / (_invMh2o * K0CO2);

  // The mole fraction of H2O in the CO2-rich gas phase is then
  Real yH2O = (1.0 - B) / (1.0 / A - B);
  // The mole fraction of CO2 in the H2O-rich liquid phase is then (note: no salinty effect)
  Real xCO2 = B * (1.0 - yH2O);
  // The molality of CO2 in the H2O-rich liquid phase is (note: no salinty effect)
  Real mCO2 = xCO2 * _invMh2o / (1.0 - xCO2);

  // The molality of CO2 in brine is then given by
  Real mCO2b = mCO2 / gamma;
  // The mole fraction of CO2 in brine is then
  Real xCO2b = mCO2b / (2.0 * mnacl + _invMh2o + mCO2b);
  // The mole fraction of H2O in the CO2-rich gas phase corrected for NaCl mole fraction is
  Real yH2Ob = A * (1.0 - xCO2b - 2.0 * mnacl / (2.0 * mnacl + _invMh2o + mCO2b));

  // Convert the mole fractions to mass fractions and then update referenced values
  // The mass fraction of H2O in gas (assume no salt in gas phase)
  Yh2o = yH2Ob * _Mh2o / (yH2Ob * _Mh2o + (1.0 - yH2Ob) * _Mco2);

  // The number of moles of CO2 in 1kg of H2O
  Real nco2 = xCO2b * (2.0 * mnacl + _invMh2o) / (1.0 - xCO2b);

  // The mass fraction of CO2 in liquid
  Xco2 = nco2 * _Mco2 / (1.0 + mnacl * _Mnacl + nco2 * _Mco2);

  // The derivatives of the mass fractions wrt pressure
  Real dyH2O_dp = ((1.0 - B) * dA_dp + (A - 1.0) * A * dB_dp) / (1.0 - A * B) / (1.0 - A * B);
  Real dxCO2_dp = dB_dp * (1.0 - yH2O) - B * dyH2O_dp;

  Real dmCO2_dp = _invMh2o * dxCO2_dp / (1.0 - xCO2) / (1.0 - xCO2);
  Real dmCO2b_dp = dmCO2_dp / gamma - mCO2 * dgamma_dp / gamma / gamma;
  Real dxCO2b_dp = (2.0 * mnacl + _invMh2o) * dmCO2b_dp / (2.0 * mnacl + _invMh2o + mCO2b) /
                   (2.0 * mnacl + _invMh2o + mCO2b);

  Real dyH2Ob_dp = (1.0 - xCO2b - 2.0 * mnacl / (2.0 * mnacl + _invMh2o + mCO2b)) * dA_dp -
                   A * dxCO2b_dp +
                   2.0 * A * mnacl * dmCO2b_dp / (2.0 * mnacl + _invMh2o + mCO2b) /
                       (2.0 * mnacl + _invMh2o + mCO2b);

  dYh2o_dp = _Mco2 * _Mh2o * dyH2Ob_dp / (yH2Ob * _Mh2o + (1.0 - yH2Ob) * _Mco2) /
             (yH2Ob * _Mh2o + (1.0 - yH2Ob) * _Mco2);

  Real dnco2_dp = dxCO2b_dp * (2.0 * mnacl + _invMh2o) / (1.0 - xCO2b) / (1.0 - xCO2b);

  dXco2_dp = (1.0 + mnacl * _Mnacl) * _Mco2 * dnco2_dp / (1.0 + mnacl * _Mnacl + nco2 * _Mco2) /
             (1.0 + mnacl * _Mnacl + nco2 * _Mco2);

  // The derivatives of the mass fractions wrt temperature
  Real dyH2O_dT = ((1.0 - B) * dA_dT + (A - 1.0) * A * dB_dT) / (1.0 - A * B) / (1.0 - A * B);
  Real dxCO2_dT = dB_dT * (1.0 - yH2O) - B * dyH2O_dT;

  Real dmCO2_dT = _invMh2o * dxCO2_dT / (1.0 - xCO2) / (1.0 - xCO2);
  Real dmCO2b_dT = dmCO2_dT / gamma - mCO2 * dgamma_dT / gamma / gamma;
  Real dxCO2b_dT = (2.0 * mnacl + _invMh2o) * dmCO2b_dT / (2.0 * mnacl + _invMh2o + mCO2b) /
                   (2.0 * mnacl + _invMh2o + mCO2b);

  Real dyH2Ob_dT = (1.0 - xCO2b - 2.0 * mnacl / (2.0 * mnacl + _invMh2o + mCO2b)) * dA_dT -
                   A * dxCO2b_dT +
                   2.0 * A * mnacl * dmCO2b_dT / (2.0 * mnacl + _invMh2o + mCO2b) /
                       (2.0 * mnacl + _invMh2o + mCO2b);

  dYh2o_dT = _Mco2 * _Mh2o * dyH2Ob_dT / (yH2Ob * _Mh2o + (1.0 - yH2Ob) * _Mco2) /
             (yH2Ob * _Mh2o + (1.0 - yH2Ob) * _Mco2);

  Real dnco2_dT = dxCO2b_dT * (2.0 * mnacl + _invMh2o) / (1.0 - xCO2b) / (1.0 - xCO2b);

  dXco2_dT = (1.0 + mnacl * _Mnacl) * _Mco2 * dnco2_dT / (1.0 + mnacl * _Mnacl + nco2 * _Mco2) /
             (1.0 + mnacl * _Mnacl + nco2 * _Mco2);
}

void
PorousFlowBrineCO2::fugacityCoefficientCO2(
    Real pressure, Real temperature, Real & fco2, Real & dfco2_dp, Real & dfco2_dT) const
{
  // Need pressure in bar
  Real pbar = pressure * 1.0e-5;
  // CO2 density and derivatives wrt pressure and temperature
  Real gas_density, dgas_density_dp, dgas_density_dT;
  _co2_fp.rho_dpT(pressure, temperature, gas_density, dgas_density_dp, dgas_density_dT);
  // Molar volume in cm^3/mol
  Real V = _Mco2 / gas_density * 1.0e6;

  // Redlich-Kwong parameters
  Real aCO2 = 7.54e7 - 4.13e4 * temperature;
  Real bCO2 = 27.8;

  Real term1 = std::log(V / (V - bCO2)) + bCO2 / (V - bCO2) -
               2.0 * aCO2 / (_Rbar * std::pow(temperature, 1.5) * bCO2) * std::log((V + bCO2) / V) +
               aCO2 / (_Rbar * std::pow(temperature, 1.5) * bCO2) *
                   (std::log((V + bCO2) / V) - bCO2 / (V + bCO2));

  Real lnPhiCO2 = term1 - std::log(pbar * V / (_Rbar * temperature));
  fco2 = std::exp(lnPhiCO2);

  // The derivative of the fugacity coefficient wrt pressure
  Real dV_dp = -_Mco2 / gas_density / gas_density * dgas_density_dp * 1.0e6;
  Real dterm1_dV =
      (bCO2 * (bCO2 - 2.0 * V) / (bCO2 - V) / (bCO2 - V) +
       aCO2 * (bCO2 + 2.0 * V) / (_Rbar * std::pow(temperature, 1.5) * (bCO2 + V) * (bCO2 + V))) /
      V;
  dfco2_dp = (dterm1_dV * dV_dp - 1.0e-5 / pbar - 1.0 / V * dV_dp) * fco2;

  // The derivative of the fugacity coefficient wrt temperature
  Real dV_dT = -_Mco2 / gas_density / gas_density * dgas_density_dT * 1.0e6;
  Real dterm1_dT = 3.0 * aCO2 * _Rbar * std::sqrt(temperature) * bCO2 * std::log((V + bCO2) / V) /
                       (_Rbar * std::pow(temperature, 1.5) * bCO2) /
                       (_Rbar * std::pow(temperature, 1.5) * bCO2) +
                   8.26e4 / (_Rbar * std::pow(temperature, 1.5) * bCO2) * std::log((V + bCO2) / V) -
                   1.5 * aCO2 * _Rbar * std::sqrt(temperature) * bCO2 *
                       (std::log((V + bCO2) / V) - bCO2 / (V + bCO2)) /
                       (_Rbar * std::pow(temperature, 1.5) * bCO2) /
                       (_Rbar * std::pow(temperature, 1.5) * bCO2) -
                   4.13e4 / (_Rbar * std::pow(temperature, 1.5) * bCO2) *
                       (std::log((V + bCO2) / V) - bCO2 / (V + bCO2));
  dfco2_dT = (dterm1_dT + dterm1_dV * dV_dT - dV_dT / V + 1.0 / temperature) * fco2;
}

void
PorousFlowBrineCO2::fugacityCoefficientH2O(
    Real pressure, Real temperature, Real & fh2o, Real & dfh2o_dp, Real & dfh2o_dT) const
{
  // Need pressure in bar
  Real pbar = pressure * 1.0e-5;
  // CO2 density and derivatives wrt pressure and temperature
  Real gas_density, dgas_density_dp, dgas_density_dT;
  _co2_fp.rho_dpT(pressure, temperature, gas_density, dgas_density_dp, dgas_density_dT);
  // Molar volume in cm^3/mol
  Real V = _Mco2 / gas_density * 1.0e6;

  // Redlich-Kwong parameters
  Real aCO2 = 7.54e7 - 4.13e4 * temperature;
  Real bCO2 = 27.8;
  Real aCO2H2O = 7.89e7;
  Real bH2O = 18.18;

  Real term1 =
      std::log(V / (V - bCO2)) + bH2O / (V - bCO2) -
      2.0 * aCO2H2O / (_Rbar * std::pow(temperature, 1.5) * bCO2) * std::log((V + bCO2) / V) +
      aCO2 * bH2O / (_Rbar * std::pow(temperature, 1.5) * bCO2 * bCO2) *
          (std::log((V + bCO2) / V) - bCO2 / (V + bCO2));

  Real lnPhiH2O = term1 - std::log(pbar * V / (_Rbar * temperature));
  fh2o = std::exp(lnPhiH2O);

  // The derivative of the fugacity coefficient wrt pressure
  Real dV_dp = -_Mco2 / gas_density / gas_density * dgas_density_dp * 1.0e6;
  Real dterm1_dV = ((bCO2 * bCO2 - (bCO2 + bH2O) * V) / (bCO2 - V) / (bCO2 - V) +
                    (2.0 * aCO2H2O * (bCO2 + V) - aCO2 * bH2O) /
                        (_Rbar * std::pow(temperature, 1.5) * (bCO2 + V) * (bCO2 + V))) /
                   V;
  dfh2o_dp = (dterm1_dV * dV_dp - 1.0e-5 / pbar - dV_dp / V) * fh2o;

  // The derivative of the fugacity coefficient wrt temperature
  Real dV_dT = -_Mco2 / gas_density / gas_density * dgas_density_dT * 1.0e6;
  Real dterm1_dT = 3.0 * _Rbar * std::sqrt(temperature) * bCO2 * aCO2H2O *
                       std::log((V + bCO2) / V) / (_Rbar * std::pow(temperature, 1.5) * bCO2) /
                       (_Rbar * std::pow(temperature, 1.5) * bCO2) -
                   1.5 * aCO2 * bH2O * _Rbar * std::sqrt(temperature) * bCO2 * bCO2 *
                       (std::log((V + bCO2) / V) - bCO2 / (V + bCO2)) /
                       (_Rbar * std::pow(temperature, 1.5) * bCO2 * bCO2) /
                       (_Rbar * std::pow(temperature, 1.5) * bCO2 * bCO2) -
                   4.13e4 * bH2O * (std::log((V + bCO2) / V) - bCO2 / (V + bCO2)) /
                       (_Rbar * std::pow(temperature, 1.5) * bCO2 * bCO2);
  dfh2o_dT = (dterm1_dT + dterm1_dV * dV_dT - dV_dT / V + 1.0 / temperature) * fh2o;
}

void
PorousFlowBrineCO2::activityCoefficient(Real pressure,
                                        Real temperature,
                                        Real xnacl,
                                        Real & gamma,
                                        Real & dgamma_dp,
                                        Real & dgamma_dT) const
{
  // Need pressure in bar
  Real pbar = pressure * 1.0e-5;
  // Need NaCl molality (mol/kg)
  Real mnacl = xnacl / (1.0 - xnacl) / _Mnacl;

  Real lambda = -0.411370585 + 6.07632013e-4 * temperature + 97.5347708 / temperature -
                0.0237622469 * pbar / temperature + 0.0170656236 * pbar / (630.0 - temperature) +
                1.41335834e-5 * temperature * std::log(pbar);

  Real xi = 3.36389723e-4 - 1.9829898e-5 * temperature + 2.12220830e-3 * pbar / temperature -
            5.24873303e-3 * pbar / (630.0 - temperature);

  gamma = std::exp(2.0 * lambda * mnacl + xi * mnacl * mnacl);

  // Derivative wrt pressure
  Real dlambda_dp, dxi_dp;
  dlambda_dp = -0.0237622469 / temperature + 0.0170656236 / (630.0 - temperature) +
               1.41335834e-5 * temperature / pbar;
  dxi_dp = 2.12220830e-3 / temperature - 5.24873303e-3 / (630.0 - temperature);
  dgamma_dp = (2.0 * mnacl * dlambda_dp + mnacl * mnacl * dxi_dp) *
              std::exp(2.0 * lambda * mnacl + xi * mnacl * mnacl) * 1.0e-5;

  // Derivative wrt temperature
  Real dlambda_dT, dxi_dT;
  dlambda_dT = 6.07632013e-4 - 97.5347708 / temperature / temperature +
               0.0237622469 * pbar / temperature / temperature +
               0.0170656236 * pbar / (630.0 - temperature) / (630.0 - temperature) +
               1.41335834e-5 * std::log(pbar);
  dxi_dT = -1.9829898e-5 - 2.12220830e-3 * pbar / temperature / temperature -
           5.24873303e-3 * pbar / (630.0 - temperature) / (630.0 - temperature);
  dgamma_dT = (2.0 * mnacl * dlambda_dT + mnacl * mnacl * dxi_dT) *
              std::exp(2.0 * lambda * mnacl + xi * mnacl * mnacl);
}

void
PorousFlowBrineCO2::equilibriumConstantH2O(Real temperature, Real & kh2o, Real & dkh2o_dT) const
{
  // Uses temperature in Celcius
  Real Tc = temperature - _T_c2k;

  Real logK0H2O = -2.209 + 3.097e-2 * Tc - 1.098e-4 * Tc * Tc + 2.048e-7 * Tc * Tc * Tc;
  Real dlogK0H2O = 3.097e-2 - 2.196e-4 * Tc + 6.144e-7 * Tc * Tc;

  kh2o = std::pow(10.0, logK0H2O);
  dkh2o_dT = std::log(10.0) * dlogK0H2O * kh2o;
}

void
PorousFlowBrineCO2::equilibriumConstantCO2(Real temperature, Real & kco2, Real & dkco2_dT) const
{
  // Uses temperature in Celcius
  Real Tc = temperature - _T_c2k;

  Real logK0CO2 = 1.189 + 1.304e-2 * Tc - 5.446e-5 * Tc * Tc;
  Real dlogK0CO2 = 1.304e-2 - 1.0892e-4 * Tc;

  kco2 = std::pow(10.0, logK0CO2);
  dkco2_dT = std::log(10.0) * dlogK0CO2 * kco2;
}

void
PorousFlowBrineCO2::partialDensityCO2(Real temperature,
                                      Real & partial_density,
                                      Real & dpartial_density_dT) const
{
  // This correlation uses temperature in C
  Real Tc = temperature - _T_c2k;
  // The parial molar volume
  Real V = 37.51 - 9.585e-2 * Tc + 8.74e-4 * Tc * Tc - 5.044e-7 * Tc * Tc * Tc;
  Real dV_dT = -9.585e-2 + 1.748e-3 * Tc - 1.5132e-6 * Tc * Tc;

  partial_density = 1.0e6 * _Mco2 / V;
  dpartial_density_dT = -1.0e6 * _Mco2 * dV_dT / V / V;
}

Real
PorousFlowBrineCO2::totalMassFraction(Real pressure,
                                      Real temperature,
                                      Real xnacl,
                                      Real saturation) const
{
  // Check whether the input pressure and temperature are within the region of validity
  checkVariables(pressure, temperature);

  // FluidStateProperties data structure
  std::vector<FluidStateProperties> fsp(_num_phases, FluidStateProperties(_num_components));
  FluidStateProperties & liquid = fsp[_aqueous_phase_number];
  FluidStateProperties & gas = fsp[_gas_phase_number];

  // Calculate equilibrium mass fractions in the two-phase state
  Real Xco2, dXco2_dp, dXco2_dT, Yh2o, dYh2o_dp, dYh2o_dT;
  equilibriumMassFractions(
      pressure, temperature, xnacl, Xco2, dXco2_dp, dXco2_dT, Yh2o, dYh2o_dp, dYh2o_dT);

  // Save the mass fractions in the FluidStateMassFractions object
  Real Yco2 = 1.0 - Yh2o;
  liquid.mass_fraction[_aqueous_fluid_component] = 1.0 - Xco2;
  liquid.mass_fraction[_gas_fluid_component] = Xco2;
  gas.mass_fraction[_aqueous_fluid_component] = Yh2o;
  gas.mass_fraction[_gas_fluid_component] = Yco2;

  // Gas properties
  gasProperties(pressure, temperature, fsp);

  // Liquid properties
  Real liquid_saturation = 1.0 - saturation;
  Real liquid_pressure = pressure - _pc_uo.capillaryPressure(liquid_saturation);
  liquidProperties(liquid_pressure, temperature, xnacl, fsp);

  // The total mass fraction of ncg (z) can now be calculated
  Real z = (saturation * gas.density * Yco2 + liquid_saturation * liquid.density * Xco2) /
           (saturation * gas.density + liquid_saturation * liquid.density);

  return z;
}

void
PorousFlowBrineCO2::checkVariables(Real pressure, Real temperature) const
{
  // The calculation of mass fractions is valid from 12C <= T <= 100C, and
  // pressure less than 60 MPa
  if (temperature < 285.15 || temperature > 373.15)
    mooseError("PorousFlowBrineCO2: Temperature is outside range 285.15 K <= T "
               "<= 373.15 K");

  if (pressure > 6.0e7)
    mooseError("PorousFlowBrineCO2: Pressure must be less than 60 MPa");
}
