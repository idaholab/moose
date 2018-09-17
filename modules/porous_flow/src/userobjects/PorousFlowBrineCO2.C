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
#include "MathUtils.h"
#include "Conversion.h"

registerMooseObject("PorousFlowApp", PorousFlowBrineCO2);

template <>
InputParameters
validParams<PorousFlowBrineCO2>()
{
  InputParameters params = validParams<PorousFlowFluidStateBase>();
  params.addRequiredParam<UserObjectName>("brine_fp", "The name of the user object for brine");
  params.addRequiredParam<UserObjectName>("co2_fp", "The name of the user object for CO2");
  params.addParam<unsigned int>("salt_component", 2, "The component number of salt");
  params.addClassDescription("Fluid state class for brine and CO2");
  return params;
}

PorousFlowBrineCO2::PorousFlowBrineCO2(const InputParameters & parameters)
  : PorousFlowFluidStateBase(parameters),
    _salt_component(getParam<unsigned int>("salt_component")),
    _brine_fp(getUserObject<BrineFluidProperties>("brine_fp")),
    _co2_fp(getUserObject<SinglePhaseFluidPropertiesPT>("co2_fp")),
    _water_fp(_brine_fp.getComponent(BrineFluidProperties::WATER)),
    _Mh2o(_brine_fp.molarMassH2O()),
    _invMh2o(1.0 / _Mh2o),
    _Mco2(_co2_fp.molarMass()),
    _Mnacl(_brine_fp.molarMassNaCl()),
    _Rbar(_R * 10.0),
    _Tlower(372.15),
    _Tupper(382.15)
{
  // Check that the correct FluidProperties UserObjects have been provided
  if (_co2_fp.fluidName() != "co2")
    paramError("co2_fp", "A valid CO2 FluidProperties UserObject must be provided");

  if (_brine_fp.fluidName() != "brine")
    paramError("brine_fp", "A valid Brine FluidProperties UserObject must be provided");

  // Set the number of phases and components, and their indexes
  _num_phases = 2;
  _num_components = 3;
  _gas_phase_number = 1 - _aqueous_phase_number;
  _gas_fluid_component = 3 - _aqueous_fluid_component - _salt_component;

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

  // Check that the salt component index is not identical to the liquid fluid component
  if (_salt_component == _aqueous_fluid_component)
    paramError(
        "salt_component",
        "The value provided must be different from the value entered in liquid_fluid_component");

  // Check that _salt_component is <= total number of fluid components
  if (_salt_component >= _num_components)
    paramError("salt_component",
               "The value provided is larger than the possible number of fluid components",
               _num_components);
}

std::string
PorousFlowBrineCO2::fluidStateName() const
{
  return "brine-co2";
}

void
PorousFlowBrineCO2::thermophysicalProperties(Real pressure,
                                             Real temperature,
                                             Real Xnacl,
                                             Real Z,
                                             unsigned qp,
                                             std::vector<FluidStateProperties> & fsp) const
{
  FluidStateProperties & liquid = fsp[_aqueous_phase_number];
  FluidStateProperties & gas = fsp[_gas_phase_number];

  // Check whether the input temperature is within the region of validity
  checkVariables(pressure, temperature);

  // Clear all of the FluidStateProperties data
  clearFluidStateProperties(fsp);

  FluidStatePhaseEnum phase_state;
  massFractions(pressure, temperature, Xnacl, Z, phase_state, fsp);

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
      Real liquid_pressure = pressure - _pc_uo.capillaryPressure(1.0, qp);
      liquidProperties(liquid_pressure, temperature, Xnacl, fsp);

      break;
    }

    case FluidStatePhaseEnum::TWOPHASE:
    {
      // Calculate the gas properties
      gasProperties(pressure, temperature, fsp);

      // Calculate the saturation
      saturationTwoPhase(pressure, temperature, Xnacl, Z, fsp);

      // Calculate the liquid properties
      Real liquid_pressure = pressure - _pc_uo.capillaryPressure(1.0 - gas.saturation, qp);
      liquidProperties(liquid_pressure, temperature, Xnacl, fsp);

      break;
    }
  }

  // Liquid saturations can now be set
  liquid.saturation = 1.0 - gas.saturation;
  liquid.dsaturation_dp = -gas.dsaturation_dp;
  liquid.dsaturation_dT = -gas.dsaturation_dT;
  liquid.dsaturation_dX = -gas.dsaturation_dX;
  liquid.dsaturation_dZ = -gas.dsaturation_dZ;

  // Save pressures to FluidStateProperties object
  gas.pressure = pressure;
  liquid.pressure = pressure - _pc_uo.capillaryPressure(liquid.saturation, qp);
}

void
PorousFlowBrineCO2::massFractions(Real pressure,
                                  Real temperature,
                                  Real Xnacl,
                                  Real Z,
                                  FluidStatePhaseEnum & phase_state,
                                  std::vector<FluidStateProperties> & fsp) const
{
  FluidStateProperties & liquid = fsp[_aqueous_phase_number];
  FluidStateProperties & gas = fsp[_gas_phase_number];

  // Equilibrium mass fraction of CO2 in liquid and H2O in gas phases
  Real Xco2, dXco2_dp, dXco2_dT, dXco2_dX, Yh2o, dYh2o_dp, dYh2o_dT, dYh2o_dX;
  equilibriumMassFractions(pressure,
                           temperature,
                           Xnacl,
                           Xco2,
                           dXco2_dp,
                           dXco2_dT,
                           dXco2_dX,
                           Yh2o,
                           dYh2o_dp,
                           dYh2o_dT,
                           dYh2o_dX);

  Real Yco2 = 1.0 - Yh2o;
  Real dYco2_dp = -dYh2o_dp;
  Real dYco2_dT = -dYh2o_dT;
  Real dYco2_dX = -dYh2o_dX;

  // Determine which phases are present based on the value of z
  phaseState(Z, Xco2, Yco2, phase_state);

  // The equilibrium mass fractions calculated above are only correct in the two phase
  // state. If only liquid or gas phases are present, the mass fractions are given by
  // the total mass fraction z
  Real Xh2o = 0.0;
  Real dXco2_dZ = 0.0, dYco2_dZ = 0.0;

  switch (phase_state)
  {
    case FluidStatePhaseEnum::LIQUID:
    {
      Xco2 = Z;
      Yco2 = 0.0;
      Xh2o = 1.0 - Z;
      Yh2o = 0.0;
      dXco2_dp = 0.0;
      dXco2_dT = 0.0;
      dXco2_dX = 0.0;
      dXco2_dZ = 1.0;
      dYco2_dp = 0.0;
      dYco2_dT = 0.0;
      dYco2_dX = 0.0;
      break;
    }

    case FluidStatePhaseEnum::GAS:
    {
      Xco2 = 0.0;
      Yco2 = Z;
      Yh2o = 1.0 - Z;
      dXco2_dp = 0.0;
      dXco2_dT = 0.0;
      dXco2_dX = 0.0;
      dYco2_dZ = 1.0;
      dYco2_dp = 0.0;
      dYco2_dT = 0.0;
      dYco2_dX = 0.0;
      break;
    }

    case FluidStatePhaseEnum::TWOPHASE:
    {
      // Keep equilibrium mass fractions
      Xh2o = 1.0 - Xco2;
      break;
    }
  }

  // Save the mass fractions in the FluidStateProperties object
  liquid.mass_fraction[_aqueous_fluid_component] = Xh2o;
  liquid.mass_fraction[_gas_fluid_component] = Xco2;
  liquid.mass_fraction[_salt_component] = Xnacl;
  gas.mass_fraction[_aqueous_fluid_component] = Yh2o;
  gas.mass_fraction[_gas_fluid_component] = Yco2;

  // Save the derivatives wrt PorousFlow variables
  liquid.dmass_fraction_dp[_aqueous_fluid_component] = -dXco2_dp;
  liquid.dmass_fraction_dp[_gas_fluid_component] = dXco2_dp;
  liquid.dmass_fraction_dT[_aqueous_fluid_component] = -dXco2_dT;
  liquid.dmass_fraction_dT[_gas_fluid_component] = dXco2_dT;
  liquid.dmass_fraction_dX[_aqueous_fluid_component] = -dXco2_dX;
  liquid.dmass_fraction_dX[_gas_fluid_component] = dXco2_dX;
  liquid.dmass_fraction_dX[_salt_component] = 1.0;
  liquid.dmass_fraction_dZ[_aqueous_fluid_component] = -dXco2_dZ;
  liquid.dmass_fraction_dZ[_gas_fluid_component] = dXco2_dZ;

  gas.dmass_fraction_dp[_aqueous_fluid_component] = -dYco2_dp;
  gas.dmass_fraction_dp[_gas_fluid_component] = dYco2_dp;
  gas.dmass_fraction_dT[_aqueous_fluid_component] = -dYco2_dT;
  gas.dmass_fraction_dT[_gas_fluid_component] = dYco2_dT;
  gas.dmass_fraction_dX[_aqueous_fluid_component] = -dYco2_dX;
  gas.dmass_fraction_dX[_gas_fluid_component] = dYco2_dX;
  gas.dmass_fraction_dZ[_aqueous_fluid_component] = -dYco2_dZ;
  gas.dmass_fraction_dZ[_gas_fluid_component] = dYco2_dZ;
}

void
PorousFlowBrineCO2::gasProperties(Real pressure,
                                  Real temperature,
                                  std::vector<FluidStateProperties> & fsp) const
{
  FluidStateProperties & gas = fsp[_gas_phase_number];

  // Gas density, viscosity and enthalpy are approximated with pure CO2 - no correction due
  // to the small amount of water vapor is made
  Real co2_density, dco2_density_dp, dco2_density_dT;
  Real co2_viscosity, dco2_viscosity_dp, dco2_viscosity_dT;
  Real co2_enthalpy, dco2_enthalpy_dp, dco2_enthalpy_dT;
  _co2_fp.rho_mu_dpT(pressure,
                     temperature,
                     co2_density,
                     dco2_density_dp,
                     dco2_density_dT,
                     co2_viscosity,
                     dco2_viscosity_dp,
                     dco2_viscosity_dT);

  _co2_fp.h_from_p_T(pressure, temperature, co2_enthalpy, dco2_enthalpy_dp, dco2_enthalpy_dT);

  // Save the values to the FluidStateProperties object. Note that derivatives wrt z are 0
  gas.density = co2_density;
  gas.ddensity_dp = dco2_density_dp;
  gas.ddensity_dT = dco2_density_dT;
  gas.ddensity_dZ = 0.0;

  gas.viscosity = co2_viscosity;
  gas.dviscosity_dp = dco2_viscosity_dp;
  gas.dviscosity_dT = dco2_viscosity_dT;
  gas.dviscosity_dZ = 0.0;

  gas.enthalpy = co2_enthalpy;
  gas.denthalpy_dp = dco2_enthalpy_dp;
  gas.denthalpy_dT = dco2_enthalpy_dT;
  gas.denthalpy_dZ = 0.0;
}

void
PorousFlowBrineCO2::liquidProperties(Real pressure,
                                     Real temperature,
                                     Real Xnacl,
                                     std::vector<FluidStateProperties> & fsp) const
{
  FluidStateProperties & liquid = fsp[_aqueous_phase_number];

  // The liquid density includes the density increase due to dissolved CO2
  Real brine_density, dbrine_density_dp, dbrine_density_dT, dbrine_density_dX;
  _brine_fp.rho_dpTx(pressure,
                     temperature,
                     Xnacl,
                     brine_density,
                     dbrine_density_dp,
                     dbrine_density_dT,
                     dbrine_density_dX);

  // Mass fraction of CO2 in liquid phase
  const Real Xco2 = liquid.mass_fraction[_gas_fluid_component];
  const Real dXco2_dp = liquid.dmass_fraction_dp[_gas_fluid_component];
  const Real dXco2_dT = liquid.dmass_fraction_dT[_gas_fluid_component];
  const Real dXco2_dZ = liquid.dmass_fraction_dZ[_gas_fluid_component];
  const Real dXco2_dX = liquid.dmass_fraction_dX[_gas_fluid_component];

  // The liquid density
  Real co2_partial_density, dco2_partial_density_dT;
  partialDensityCO2(temperature, co2_partial_density, dco2_partial_density_dT);

  const Real liquid_density = 1.0 / (Xco2 / co2_partial_density + (1.0 - Xco2) / brine_density);

  const Real dliquid_density_dp =
      (dXco2_dp / brine_density + (1.0 - Xco2) * dbrine_density_dp / brine_density / brine_density -
       dXco2_dp / co2_partial_density) *
      liquid_density * liquid_density;

  const Real dliquid_density_dT =
      (dXco2_dT / brine_density + (1.0 - Xco2) * dbrine_density_dT / brine_density / brine_density -
       dXco2_dT / co2_partial_density +
       Xco2 * dco2_partial_density_dT / co2_partial_density / co2_partial_density) *
      liquid_density * liquid_density;

  const Real dliquid_density_dZ =
      (dXco2_dZ / brine_density - dXco2_dZ / co2_partial_density) * liquid_density * liquid_density;

  const Real dliquid_density_dX =
      (dXco2_dX / brine_density + (1.0 - Xco2) * dbrine_density_dX / brine_density / brine_density -
       dXco2_dX / co2_partial_density) *
      liquid_density * liquid_density;

  // Assume that liquid viscosity is just the brine viscosity
  Real liquid_viscosity, dliquid_viscosity_dp, dliquid_viscosity_dT, dliquid_viscosity_dX;
  _brine_fp.mu_dpTx(pressure,
                    temperature,
                    Xnacl,
                    liquid_viscosity,
                    dliquid_viscosity_dp,
                    dliquid_viscosity_dT,
                    dliquid_viscosity_dX);

  // Liquid enthalpy (including contribution due to the enthalpy of dissolution)
  Real brine_enthalpy, dbrine_enthalpy_dp, dbrine_enthalpy_dT, dbrine_enthalpy_dX;
  _brine_fp.h_dpTx(pressure,
                   temperature,
                   Xnacl,
                   brine_enthalpy,
                   dbrine_enthalpy_dp,
                   dbrine_enthalpy_dT,
                   dbrine_enthalpy_dX);

  // Enthalpy of CO2
  Real co2_enthalpy, dco2_enthalpy_dp, dco2_enthalpy_dT;
  _co2_fp.h_from_p_T(pressure, temperature, co2_enthalpy, dco2_enthalpy_dp, dco2_enthalpy_dT);

  // Enthalpy of dissolution
  Real hdis, dhdis_dT;
  enthalpyOfDissolution(temperature, hdis, dhdis_dT);

  const Real liquid_enthalpy = (1.0 - Xco2) * brine_enthalpy + Xco2 * (co2_enthalpy + hdis);
  const Real dliquid_enthalpy_dp = (1.0 - Xco2) * dbrine_enthalpy_dp + Xco2 * dco2_enthalpy_dp +
                                   dXco2_dp * (co2_enthalpy + hdis - brine_enthalpy);
  const Real dliquid_enthalpy_dT = (1.0 - Xco2) * dbrine_enthalpy_dT +
                                   Xco2 * (dco2_enthalpy_dT + dhdis_dT) +
                                   dXco2_dT * (co2_enthalpy + hdis - brine_enthalpy);
  const Real dliquid_enthalpy_dZ = dXco2_dZ * (co2_enthalpy + hdis - brine_enthalpy);
  const Real dliquid_enthalpy_dX =
      (1.0 - Xco2) * dbrine_enthalpy_dX + dXco2_dX * (co2_enthalpy + hdis - brine_enthalpy);

  // Save the values to the FluidStateProperties object
  liquid.density = liquid_density;
  liquid.ddensity_dp = dliquid_density_dp;
  liquid.ddensity_dT = dliquid_density_dT;
  liquid.ddensity_dZ = dliquid_density_dZ;
  liquid.ddensity_dX = dliquid_density_dX;

  liquid.viscosity = liquid_viscosity;
  liquid.dviscosity_dp = dliquid_viscosity_dp;
  liquid.dviscosity_dT = dliquid_viscosity_dT;
  liquid.dviscosity_dZ = 0.0;
  liquid.dviscosity_dX = dliquid_viscosity_dX;

  liquid.enthalpy = liquid_enthalpy;
  liquid.denthalpy_dp = dliquid_enthalpy_dp;
  liquid.denthalpy_dT = dliquid_enthalpy_dT;
  liquid.denthalpy_dZ = dliquid_enthalpy_dZ;
  liquid.denthalpy_dX = dliquid_enthalpy_dX;
}

void
PorousFlowBrineCO2::saturationTwoPhase(Real pressure,
                                       Real temperature,
                                       Real Xnacl,
                                       Real Z,
                                       std::vector<FluidStateProperties> & fsp) const
{
  FluidStateProperties & liquid = fsp[_aqueous_phase_number];
  FluidStateProperties & gas = fsp[_gas_phase_number];

  // Approximate liquid density as saturation isn't known yet
  Real brine_density, dbrine_density_dp, dbrine_density_dT, dbrine_density_dX;
  _brine_fp.rho_dpTx(pressure,
                     temperature,
                     Xnacl,
                     brine_density,
                     dbrine_density_dp,
                     dbrine_density_dT,
                     dbrine_density_dX);

  // Mass fraction of CO2 in liquid phase
  const Real Xco2 = liquid.mass_fraction[_gas_fluid_component];
  const Real dXco2_dp = liquid.dmass_fraction_dp[_gas_fluid_component];
  const Real dXco2_dT = liquid.dmass_fraction_dT[_gas_fluid_component];
  const Real dXco2_dX = liquid.dmass_fraction_dX[_gas_fluid_component];

  // The liquid density
  Real co2_partial_density, dco2_partial_density_dT;
  partialDensityCO2(temperature, co2_partial_density, dco2_partial_density_dT);

  const Real liquid_density = 1.0 / (Xco2 / co2_partial_density + (1.0 - Xco2) / brine_density);

  const Real dliquid_density_dp =
      (dXco2_dp / brine_density + (1.0 - Xco2) * dbrine_density_dp / brine_density / brine_density -
       dXco2_dp / co2_partial_density) *
      liquid_density * liquid_density;

  const Real dliquid_density_dT =
      (dXco2_dT / brine_density + (1.0 - Xco2) * dbrine_density_dT / brine_density / brine_density -
       dXco2_dT / co2_partial_density +
       Xco2 * dco2_partial_density_dT / co2_partial_density / co2_partial_density) *
      liquid_density * liquid_density;

  const Real dliquid_density_dX =
      (dXco2_dX / brine_density + (1.0 - Xco2) * dbrine_density_dX / brine_density / brine_density -
       dXco2_dX / co2_partial_density) *
      liquid_density * liquid_density;

  const Real Yco2 = gas.mass_fraction[_gas_fluid_component];
  const Real dYco2_dp = gas.dmass_fraction_dp[_gas_fluid_component];
  const Real dYco2_dT = gas.dmass_fraction_dT[_gas_fluid_component];
  const Real dYco2_dX = gas.dmass_fraction_dX[_gas_fluid_component];

  // Set mass equilibrium constants used in the calculation of vapor mass fraction
  const Real K0 = Yco2 / Xco2;
  const Real K1 = (1.0 - Yco2) / (1.0 - Xco2);
  const Real vapor_mass_fraction = vaporMassFraction(Z, K0, K1);

  // The gas saturation in the two phase case
  gas.saturation = vapor_mass_fraction * liquid_density /
                   (gas.density + vapor_mass_fraction * (liquid_density - gas.density));

  const Real dv_dZ = (K1 - K0) / ((K0 - 1.0) * (K1 - 1.0));
  const Real denominator = (gas.density + vapor_mass_fraction * (liquid_density - gas.density)) *
                           (gas.density + vapor_mass_fraction * (liquid_density - gas.density));

  const Real ds_dZ = gas.density * liquid_density * dv_dZ / denominator;

  const Real dK0_dp = (Xco2 * dYco2_dp - Yco2 * dXco2_dp) / Xco2 / Xco2;
  const Real dK0_dT = (Xco2 * dYco2_dT - Yco2 * dXco2_dT) / Xco2 / Xco2;
  const Real dK0_dX = (Xco2 * dYco2_dX - Yco2 * dXco2_dX) / Xco2 / Xco2;

  const Real dK1_dp =
      ((1.0 - Yco2) * dXco2_dp - (1.0 - Xco2) * dYco2_dp) / (1.0 - Xco2) / (1.0 - Xco2);
  const Real dK1_dT =
      ((1.0 - Yco2) * dXco2_dT - (1.0 - Xco2) * dYco2_dT) / (1.0 - Xco2) / (1.0 - Xco2);
  const Real dK1_dX =
      ((1.0 - Yco2) * dXco2_dX - (1.0 - Xco2) * dYco2_dX) / (1.0 - Xco2) / (1.0 - Xco2);

  const Real dv_dp =
      Z * dK1_dp / (K1 - 1.0) / (K1 - 1.0) + (1.0 - Z) * dK0_dp / (K0 - 1.0) / (K0 - 1.0);

  Real ds_dp = gas.density * liquid_density * dv_dp +
               vapor_mass_fraction * (1.0 - vapor_mass_fraction) *
                   (gas.density * dliquid_density_dp - gas.ddensity_dp * liquid_density);
  ds_dp /= denominator;

  const Real dv_dT =
      Z * dK1_dT / (K1 - 1.0) / (K1 - 1.0) + (1.0 - Z) * dK0_dT / (K0 - 1.0) / (K0 - 1.0);

  Real ds_dT = gas.density * liquid_density * dv_dT +
               vapor_mass_fraction * (1.0 - vapor_mass_fraction) *
                   (gas.density * dliquid_density_dT - gas.ddensity_dT * liquid_density);
  ds_dT /= denominator;

  const Real dv_dX =
      Z * dK1_dX / (K1 - 1.0) / (K1 - 1.0) + (1.0 - Z) * dK0_dX / (K0 - 1.0) / (K0 - 1.0);

  Real ds_dX = gas.density * liquid_density * dv_dX + vapor_mass_fraction *
                                                          (1.0 - vapor_mass_fraction) *
                                                          (gas.density * dliquid_density_dX);
  ds_dX /= denominator;

  gas.dsaturation_dp = ds_dp;
  gas.dsaturation_dT = ds_dT;
  gas.dsaturation_dZ = ds_dZ;
  gas.dsaturation_dX = ds_dX;
}

void
PorousFlowBrineCO2::equilibriumMassFractions(Real pressure,
                                             Real temperature,
                                             Real Xnacl,
                                             Real & Xco2,
                                             Real & dXco2_dp,
                                             Real & dXco2_dT,
                                             Real & dXco2_dX,
                                             Real & Yh2o,
                                             Real & dYh2o_dp,
                                             Real & dYh2o_dT,
                                             Real & dYh2o_dX) const
{
  Real co2_density, dco2_density_dp, dco2_density_dT;
  _co2_fp.rho_from_p_T(pressure, temperature, co2_density, dco2_density_dp, dco2_density_dT);

  // Mole fractions at equilibrium
  Real xco2, dxco2_dp, dxco2_dT, dxco2_dX, yh2o, dyh2o_dp, dyh2o_dT, dyh2o_dX;
  equilibriumMoleFractions(pressure,
                           temperature,
                           Xnacl,
                           xco2,
                           dxco2_dp,
                           dxco2_dT,
                           dxco2_dX,
                           yh2o,
                           dyh2o_dp,
                           dyh2o_dT,
                           dyh2o_dX);

  // The mass fraction of H2O in gas (assume no salt in gas phase) and derivatives
  // wrt p, T, and X
  Yh2o = yh2o * _Mh2o / (yh2o * _Mh2o + (1.0 - yh2o) * _Mco2);
  dYh2o_dp = _Mco2 * _Mh2o * dyh2o_dp / (yh2o * _Mh2o + (1.0 - yh2o) * _Mco2) /
             (yh2o * _Mh2o + (1.0 - yh2o) * _Mco2);
  dYh2o_dT = _Mco2 * _Mh2o * dyh2o_dT / (yh2o * _Mh2o + (1.0 - yh2o) * _Mco2) /
             (yh2o * _Mh2o + (1.0 - yh2o) * _Mco2);
  dYh2o_dX = dyh2o_dX * _Mh2o * _Mco2 / (yh2o * _Mh2o + (1.0 - yh2o) * _Mco2) /
             (yh2o * _Mh2o + (1.0 - yh2o) * _Mco2);

  // NaCl molality (mol/kg)
  const Real mnacl = Xnacl / (1.0 - Xnacl) / _Mnacl;
  const Real dmnacl_dX = 1.0 / (1.0 - Xnacl) / (1.0 - Xnacl) / _Mnacl;

  // The molality of CO2 in 1kg of H2O
  const Real mco2 = xco2 * (2.0 * mnacl + _invMh2o) / (1.0 - xco2);
  // The mass fraction of CO2 in brine is then
  const Real denominator = (1.0 + mnacl * _Mnacl + mco2 * _Mco2);
  Xco2 = mco2 * _Mco2 / denominator;

  // Derivatives of Xco2 wrt p, T and X
  const Real denominator2 = denominator * denominator;
  const Real dmco2_dp = dxco2_dp * (2.0 * mnacl + _invMh2o) / (1.0 - xco2) / (1.0 - xco2);
  dXco2_dp = (1.0 + mnacl * _Mnacl) * _Mco2 * dmco2_dp / denominator2;

  const Real dmco2_dT = dxco2_dT * (2.0 * mnacl + _invMh2o) / (1.0 - xco2) / (1.0 - xco2);
  dXco2_dT = (1.0 + mnacl * _Mnacl) * _Mco2 * dmco2_dT / denominator2;

  const Real dmco2_dX =
      (dxco2_dX * (2.0 * mnacl + _invMh2o) + 2.0 * (1.0 - xco2) * xco2 * dmnacl_dX) / (1.0 - xco2) /
      (1.0 - xco2);
  dXco2_dX = _Mco2 * ((1.0 + mnacl * _Mnacl) * dmco2_dX - dmnacl_dX * mco2 * _Mnacl) / denominator2;
}

void
PorousFlowBrineCO2::fugacityCoefficientsLowTemp(Real pressure,
                                                Real temperature,
                                                Real co2_density,
                                                Real dco2_density_dp,
                                                Real dco2_density_dT,
                                                Real & fco2,
                                                Real & dfco2_dp,
                                                Real & dfco2_dT,
                                                Real & fh2o,
                                                Real & dfh2o_dp,
                                                Real & dfh2o_dT) const
{
  if (temperature > 373.15)
    mooseError(name(),
               ": fugacityCoefficientsLowTemp() is not valid for T > 373.15K. Use "
               "fugacityCoefficientsHighTemp() instead");

  // Need pressure in bar
  const Real pbar = pressure * 1.0e-5;

  // Molar volume in cm^3/mol
  const Real V = _Mco2 / co2_density * 1.0e6;
  const Real dV_dp = -V / co2_density * dco2_density_dp;
  const Real dV_dT = -V / co2_density * dco2_density_dT;

  // Redlich-Kwong parameters
  const Real aCO2 = 7.54e7 - 4.13e4 * temperature;
  const Real daCO2_dT = -4.13e4;
  const Real bCO2 = 27.8;
  const Real aCO2H2O = 7.89e7;
  const Real bH2O = 18.18;

  const Real t15 = std::pow(temperature, 1.5);

  // The fugacity coefficients for H2O and CO2
  auto lnPhi = [V, aCO2, bCO2, t15, this](Real a, Real b) {
    return std::log(V / (V - bCO2)) + b / (V - bCO2) -
           2.0 * a / (_Rbar * t15 * bCO2) * std::log((V + bCO2) / V) +
           aCO2 * b / (_Rbar * t15 * bCO2 * bCO2) * (std::log((V + bCO2) / V) - bCO2 / (V + bCO2));
  };

  const Real lnPhiH2O = lnPhi(aCO2H2O, bH2O) - std::log(pbar * V / (_Rbar * temperature));
  const Real lnPhiCO2 = lnPhi(aCO2, bCO2) - std::log(pbar * V / (_Rbar * temperature));

  fh2o = std::exp(lnPhiH2O);
  fco2 = std::exp(lnPhiCO2);

  // The derivative of the fugacity coefficients wrt pressure
  auto dlnPhi_dV = [V, aCO2, bCO2, t15, this](Real a, Real b) {
    return (bCO2 * bCO2 - (bCO2 + b) * V) / V / (V - bCO2) / (V - bCO2) +
           (2.0 * a * (V + bCO2) - aCO2 * b) / (_Rbar * t15 * V * (V + bCO2) * (V + bCO2));
  };

  dfh2o_dp = (dlnPhi_dV(aCO2H2O, bH2O) * dV_dp - 1.0e-5 / pbar - dV_dp / V) * fh2o;
  dfco2_dp = (dlnPhi_dV(aCO2, bCO2) * dV_dp - 1.0e-5 / pbar - dV_dp / V) * fco2;

  // The derivative of the fugacity coefficient wrt temperature
  auto dlnPhi_dT = [V, aCO2, daCO2_dT, bCO2, t15, temperature, this](Real a, Real b, Real da_dT) {
    return (3.0 * bCO2 * a * std::log((V + bCO2) / V) +
            1.5 * aCO2 * b * (bCO2 / (V + bCO2) - std::log((V + bCO2) / V)) -
            (2.0 * da_dT * bCO2 * std::log((V + bCO2) / V) -
             daCO2_dT * b * (std::log((V + bCO2) / V) - bCO2 / (V + bCO2))) *
                temperature) /
           (temperature * t15 * _Rbar * bCO2 * bCO2);
  };

  dfh2o_dT = (dlnPhi_dT(aCO2H2O, bH2O, 0) + dlnPhi_dV(aCO2H2O, bH2O) * dV_dT - dV_dT / V +
              1.0 / temperature) *
             fh2o;
  dfco2_dT = (dlnPhi_dT(aCO2, bCO2, daCO2_dT) + dlnPhi_dV(aCO2, bCO2) * dV_dT - dV_dT / V +
              1.0 / temperature) *
             fco2;
}

void
PorousFlowBrineCO2::fugacityCoefficientsHighTemp(Real pressure,
                                                 Real temperature,
                                                 Real co2_density,
                                                 Real xco2,
                                                 Real yh2o,
                                                 Real & fco2,
                                                 Real & fh2o) const
{
  if (temperature <= 373.15)
    mooseError(name(),
               ": fugacityCoefficientsHighTemp() is not valid for T <= 373.15K. Use "
               "fugacityCoefficientsLowTemp() instead");

  fh2o = fugacityCoefficientH2OHighTemp(pressure, temperature, co2_density, xco2, yh2o);
  fco2 = fugacityCoefficientCO2HighTemp(pressure, temperature, co2_density, xco2, yh2o);
}

void
PorousFlowBrineCO2::fugacityCoefficientsHighTemp(Real pressure,
                                                 Real temperature,
                                                 Real co2_density,
                                                 Real xco2,
                                                 Real yh2o,
                                                 Real & fco2,
                                                 Real & dfco2_dp,
                                                 Real & dfco2_dT,
                                                 Real & fh2o,
                                                 Real & dfh2o_dp,
                                                 Real & dfh2o_dT) const
{
  if (temperature <= 373.15)
    mooseError(name(),
               ": fugacityCoefficientsHighTemp() is not valid for T <= 373.15K. Use "
               "fugacityCoefficientsLowTemp() instead");

  fh2o = fugacityCoefficientH2OHighTemp(pressure, temperature, co2_density, xco2, yh2o);
  fco2 = fugacityCoefficientCO2HighTemp(pressure, temperature, co2_density, xco2, yh2o);

  // Derivatives by finite difference
  const Real dp = 1.0e-2;
  const Real dT = 1.0e-4;

  Real fh2o2 = fugacityCoefficientH2OHighTemp(pressure + dp, temperature, co2_density, xco2, yh2o);
  Real fco22 = fugacityCoefficientCO2HighTemp(pressure + dp, temperature, xco2, co2_density, yh2o);

  dfh2o_dp = (fh2o2 - fh2o) / dp;
  dfco2_dp = (fco22 - fco2) / dp;

  fh2o2 = fugacityCoefficientH2OHighTemp(pressure, temperature + dT, co2_density, xco2, yh2o);
  fco22 = fugacityCoefficientCO2HighTemp(pressure, temperature + dT, co2_density, xco2, yh2o);

  dfh2o_dT = (fh2o2 - fh2o) / dT;
  dfco2_dT = (fco22 - fco2) / dT;
}

Real
PorousFlowBrineCO2::fugacityCoefficientH2OHighTemp(
    Real pressure, Real temperature, Real co2_density, Real xco2, Real yh2o) const
{
  // Need pressure in bar
  const Real pbar = pressure * 1.0e-5;
  // Molar volume in cm^3/mol
  const Real V = _Mco2 / co2_density * 1.0e6;

  // Redlich-Kwong parameters
  const Real yco2 = 1.0 - yh2o;
  const Real xh2o = 1.0 - xco2;

  const Real aCO2 = 8.008e7 - 4.984e4 * temperature;
  const Real aH2O = 1.337e8 - 1.4e4 * temperature;
  const Real bCO2 = 28.25;
  const Real bH2O = 15.7;
  const Real KH2OCO2 = 1.427e-2 - 4.037e-4 * temperature;
  const Real KCO2H2O = 0.4228 - 7.422e-4 * temperature;
  const Real kH2OCO2 = KH2OCO2 * yh2o + KCO2H2O * yco2;
  const Real kCO2H2O = KCO2H2O * yh2o + KH2OCO2 * yco2;

  const Real aH2OCO2 = std::sqrt(aCO2 * aH2O) * (1.0 - kH2OCO2);
  const Real aCO2H2O = std::sqrt(aCO2 * aH2O) * (1.0 - kCO2H2O);

  const Real amix = yh2o * yh2o * aH2O + yh2o * yco2 * (aH2OCO2 + aCO2H2O) + yco2 * yco2 * aCO2;
  const Real bmix = yh2o * bH2O + yco2 * bCO2;

  const Real t15 = std::pow(temperature, 1.5);

  Real lnPhiH2O = bH2O / bmix * (pbar * V / (_Rbar * temperature) - 1.0) -
                  std::log(pbar * (V - bmix) / (_Rbar * temperature));
  Real term3 = (2.0 * yh2o * aH2O + yco2 * (aH2OCO2 + aCO2H2O) -
                yh2o * yco2 * std::sqrt(aH2O * aCO2) * (kH2OCO2 - kCO2H2O) * (yh2o - yco2) +
                xh2o * xco2 * std::sqrt(aH2O * aCO2) * (kH2OCO2 - kCO2H2O)) /
               amix;
  term3 -= bH2O / bmix;
  term3 *= amix / (bmix * _Rbar * t15) * std::log(V / (V + bmix));
  lnPhiH2O += term3;

  return std::exp(lnPhiH2O);
}

Real
PorousFlowBrineCO2::fugacityCoefficientCO2HighTemp(
    Real pressure, Real temperature, Real co2_density, Real xco2, Real yh2o) const
{
  // Need pressure in bar
  const Real pbar = pressure * 1.0e-5;
  // Molar volume in cm^3/mol
  const Real V = _Mco2 / co2_density * 1.0e6;

  // Redlich-Kwong parameters
  const Real yco2 = 1.0 - yh2o;
  const Real xh2o = 1.0 - xco2;

  const Real aCO2 = 8.008e7 - 4.984e4 * temperature;
  const Real aH2O = 1.337e8 - 1.4e4 * temperature;
  const Real bCO2 = 28.25;
  const Real bH2O = 15.7;
  const Real KH2OCO2 = 1.427e-2 - 4.037e-4 * temperature;
  const Real KCO2H2O = 0.4228 - 7.422e-4 * temperature;
  const Real kH2OCO2 = KH2OCO2 * yh2o + KCO2H2O * yco2;
  const Real kCO2H2O = KCO2H2O * yh2o + KH2OCO2 * yco2;

  const Real aH2OCO2 = std::sqrt(aCO2 * aH2O) * (1.0 - kH2OCO2);
  const Real aCO2H2O = std::sqrt(aCO2 * aH2O) * (1.0 - kCO2H2O);

  const Real amix = yh2o * yh2o * aH2O + yh2o * yco2 * (aH2OCO2 + aCO2H2O) + yco2 * yco2 * aCO2;
  const Real bmix = yh2o * bH2O + yco2 * bCO2;

  const Real t15 = std::pow(temperature, 1.5);

  Real lnPhiCO2 = bCO2 / bmix * (pbar * V / (_Rbar * temperature) - 1.0) -
                  std::log(pbar * (V - bmix) / (_Rbar * temperature));

  Real term3 = (2.0 * yco2 * aCO2 + yh2o * (aH2OCO2 + aCO2H2O) -
                yh2o * yco2 * std::sqrt(aH2O * aCO2) * (kH2OCO2 - kCO2H2O) * (yh2o - yco2) +
                xh2o * xco2 * std::sqrt(aH2O * aCO2) * (kCO2H2O - kH2OCO2)) /
               amix;

  lnPhiCO2 += (term3 - bCO2 / bmix) * amix / (bmix * _Rbar * t15) * std::log(V / (V + bmix));

  return std::exp(lnPhiCO2);
}

Real
PorousFlowBrineCO2::activityCoefficientH2O(Real temperature, Real xco2) const
{
  if (temperature <= 373.15)
    return 1.0;
  else
  {
    const Real Tref = temperature - 373.15;
    const Real xh2o = 1.0 - xco2;
    const Real Am = -3.084e-2 * Tref + 1.927e-5 * Tref * Tref;

    return std::exp((Am - 2.0 * Am * xh2o) * xco2 * xco2);
  }
}

Real
PorousFlowBrineCO2::activityCoefficientCO2(Real temperature, Real xco2) const
{
  if (temperature <= 373.15)
    return 1.0;
  else
  {
    const Real Tref = temperature - 373.15;
    const Real xh2o = 1.0 - xco2;
    const Real Am = -3.084e-2 * Tref + 1.927e-5 * Tref * Tref;

    return std::exp(2.0 * Am * xco2 * xh2o * xh2o);
  }
}

void
PorousFlowBrineCO2::activityCoefficient(Real pressure,
                                        Real temperature,
                                        Real Xnacl,
                                        Real & gamma,
                                        Real & dgamma_dp,
                                        Real & dgamma_dT,
                                        Real & dgamma_dX) const
{
  // Need pressure in bar
  const Real pbar = pressure * 1.0e-5;
  // Need NaCl molality (mol/kg)
  const Real mnacl = Xnacl / (1.0 - Xnacl) / _Mnacl;
  const Real dmnacl_dX = 1.0 / (1.0 - Xnacl) / (1.0 - Xnacl) / _Mnacl;

  const Real lambda = -0.411370585 + 6.07632013e-4 * temperature + 97.5347708 / temperature -
                      0.0237622469 * pbar / temperature +
                      0.0170656236 * pbar / (630.0 - temperature) +
                      1.41335834e-5 * temperature * std::log(pbar);

  const Real xi = 3.36389723e-4 - 1.9829898e-5 * temperature + 2.12220830e-3 * pbar / temperature -
                  5.24873303e-3 * pbar / (630.0 - temperature);

  gamma = std::exp(2.0 * lambda * mnacl + xi * mnacl * mnacl);

  // Derivative wrt pressure
  const Real dlambda_dp = -0.0237622469 / temperature + 0.0170656236 / (630.0 - temperature) +
                          1.41335834e-5 * temperature / pbar;
  const Real dxi_dp = 2.12220830e-3 / temperature - 5.24873303e-3 / (630.0 - temperature);
  dgamma_dp = (2.0 * mnacl * dlambda_dp + mnacl * mnacl * dxi_dp) * gamma * 1.0e-5;

  // Derivative wrt temperature
  const Real dlambda_dT = 6.07632013e-4 - 97.5347708 / temperature / temperature +
                          0.0237622469 * pbar / temperature / temperature +
                          0.0170656236 * pbar / (630.0 - temperature) / (630.0 - temperature) +
                          1.41335834e-5 * std::log(pbar);
  const Real dxi_dT = -1.9829898e-5 - 2.12220830e-3 * pbar / temperature / temperature -
                      5.24873303e-3 * pbar / (630.0 - temperature) / (630.0 - temperature);
  dgamma_dT = (2.0 * mnacl * dlambda_dT + mnacl * mnacl * dxi_dT) * gamma;

  // Derivative wrt salt mass fraction
  dgamma_dX = (2.0 * lambda * dmnacl_dX + 2.0 * xi * mnacl * dmnacl_dX) * gamma;
}

void
PorousFlowBrineCO2::activityCoefficientHighTemp(
    Real temperature, Real Xnacl, Real & gamma, Real & dgamma_dT, Real & dgamma_dX) const
{
  // Need NaCl molality (mol/kg)
  const Real mnacl = Xnacl / (1.0 - Xnacl) / _Mnacl;
  const Real dmnacl_dX = 1.0 / (1.0 - Xnacl) / (1.0 - Xnacl) / _Mnacl;

  const Real T2 = temperature * temperature;
  const Real T3 = temperature * T2;

  const Real lambda = 2.217e-4 * temperature + 1.074 / temperature + 2648.0 / T2;
  const Real xi = 1.3e-5 * temperature - 20.12 / temperature + 5259.0 / T2;

  gamma = (1.0 - mnacl / _invMh2o) * std::exp(2.0 * lambda * mnacl + xi * mnacl * mnacl);

  // Derivative wrt temperature
  const Real dlambda_dT = 2.217e-4 - 1.074 / T2 - 5296.0 / T3;
  const Real dxi_dT = 1.3e-5 + 20.12 / T2 - 10518.0 / T3;
  dgamma_dT = (2.0 * mnacl * dlambda_dT + mnacl * mnacl * dxi_dT) * gamma;

  // Derivative wrt salt mass fraction
  dgamma_dX = (2.0 * lambda * dmnacl_dX + 2.0 * xi * mnacl * dmnacl_dX) * gamma -
              dmnacl_dX / _invMh2o * std::exp(2.0 * lambda * mnacl + xi * mnacl * mnacl);
}

void
PorousFlowBrineCO2::equilibriumConstantH2O(Real temperature, Real & kh2o, Real & dkh2o_dT) const
{
  // Uses temperature in Celcius
  const Real Tc = temperature - _T_c2k;
  const Real Tc2 = Tc * Tc;
  const Real Tc3 = Tc2 * Tc;
  const Real Tc4 = Tc3 * Tc;

  Real logK0H2O, dlogK0H2O;

  if (Tc <= 99.0)
  {
    logK0H2O = -2.209 + 3.097e-2 * Tc - 1.098e-4 * Tc2 + 2.048e-7 * Tc3;
    dlogK0H2O = 3.097e-2 - 2.196e-4 * Tc + 6.144e-7 * Tc2;
  }
  else if (Tc > 99.0 && Tc < 109.0)
  {
    const Real Tint = (Tc - 99.0) / 10.0;
    const Real Tint2 = Tint * Tint;
    logK0H2O = -0.0204026 + 0.0152513 * Tint + 0.417565 * Tint2 - 0.278636 * Tint * Tint2;
    dlogK0H2O = 0.0152513 + 0.83513 * Tint - 0.835908 * Tint2;
  }
  else // 109 <= Tc <= 300
  {
    logK0H2O = -2.1077 + 2.8127e-2 * Tc - 8.4298e-5 * Tc2 + 1.4969e-7 * Tc3 - 1.1812e-10 * Tc4;
    dlogK0H2O = 2.8127e-2 - 1.68596e-4 * Tc + 4.4907e-7 * Tc2 - 4.7248e-10 * Tc3;
  }

  kh2o = std::pow(10.0, logK0H2O);
  dkh2o_dT = std::log(10.0) * dlogK0H2O * kh2o;
}

void
PorousFlowBrineCO2::equilibriumConstantCO2(Real temperature, Real & kco2, Real & dkco2_dT) const
{
  // Uses temperature in Celcius
  const Real Tc = temperature - _T_c2k;
  const Real Tc2 = Tc * Tc;
  const Real Tc3 = Tc2 * Tc;

  Real logK0CO2, dlogK0CO2;

  if (Tc <= 99.0)
  {
    logK0CO2 = 1.189 + 1.304e-2 * Tc - 5.446e-5 * Tc2;
    dlogK0CO2 = 1.304e-2 - 1.0892e-4 * Tc;
  }
  else if (Tc > 99.0 && Tc < 109.0)
  {
    const Real Tint = (Tc - 99.0) / 10.0;
    const Real Tint2 = Tint * Tint;
    logK0CO2 = 1.9462 + 2.25692e-2 * Tint - 9.49577e-3 * Tint2 - 6.77721e-3 * Tint * Tint2;
    dlogK0CO2 = 2.25692e-2 - 1.899154e-2 * Tint + 2.033163e-2 * Tint2;
  }
  else // 109 <= Tc <= 300
  {
    logK0CO2 = 1.668 + 3.992e-3 * Tc - 1.156e-5 * Tc2 + 1.593e-9 * Tc3;
    dlogK0CO2 = 3.992e-3 - 2.312e-5 * Tc + 4.779e-9 * Tc2;
  }

  kco2 = std::pow(10.0, logK0CO2);
  dkco2_dT = std::log(10.0) * dlogK0CO2 * kco2;
}

void
PorousFlowBrineCO2::equilibriumMoleFractions(Real pressure,
                                             Real temperature,
                                             Real Xnacl,
                                             Real & xco2,
                                             Real & dxco2_dp,
                                             Real & dxco2_dT,
                                             Real & dxco2_dX,
                                             Real & yh2o,
                                             Real & dyh2o_dp,
                                             Real & dyh2o_dT,
                                             Real & dyh2o_dX) const
{
  // CO2 density and derivatives wrt pressure and temperature
  Real co2_density, dco2_density_dp, dco2_density_dT;
  _co2_fp.rho_from_p_T(pressure, temperature, co2_density, dco2_density_dp, dco2_density_dT);

  if (temperature <= _Tlower)
  {
    equilibriumMoleFractionsLowTemp(pressure,
                                    temperature,
                                    Xnacl,
                                    xco2,
                                    dxco2_dp,
                                    dxco2_dT,
                                    dxco2_dX,
                                    yh2o,
                                    dyh2o_dp,
                                    dyh2o_dT,
                                    dyh2o_dX);
  }
  else if (temperature > _Tlower && temperature < _Tupper)
  {
    // Cubic polynomial in this regime
    const Real Tint = (temperature - _Tlower) / 10.0;

    // Equilibrium mole fractions and derivatives at the lower temperature
    Real xco2_lower, dxco2_dT_lower, yh2o_lower, dyh2o_dT_lower;
    equilibriumMoleFractionsLowTemp(pressure,
                                    _Tlower,
                                    Xnacl,
                                    xco2_lower,
                                    dxco2_dp,
                                    dxco2_dT_lower,
                                    dxco2_dX,
                                    yh2o_lower,
                                    dyh2o_dp,
                                    dyh2o_dT_lower,
                                    dyh2o_dX);

    // Equilibrium mole fractions and derivatives at the upper temperature
    Real xco2_upper, yh2o_upper;
    Real co2_density_upper = _co2_fp.rho_from_p_T(pressure, _Tupper);

    solveEquilibriumMoleFractionHighTemp(
        pressure, _Tupper, Xnacl, co2_density_upper, xco2_upper, yh2o_upper);

    Real A, dA_dp, dA_dT, B, dB_dp, dB_dT, dB_dX;
    funcABHighTemp(pressure,
                   _Tupper,
                   Xnacl,
                   co2_density,
                   xco2_upper,
                   yh2o_upper,
                   A,
                   dA_dp,
                   dA_dT,
                   B,
                   dB_dp,
                   dB_dT,
                   dB_dX);

    const Real dyh2o_dT_upper =
        ((1.0 - B) * dA_dT + (A - 1.0) * A * dB_dT) / (1.0 - A * B) / (1.0 - A * B);
    const Real dxco2_dT_upper = dB_dT * (1.0 - yh2o_upper) - B * dyh2o_dT_upper;

    // The mole fractions in this regime are then found by interpolation
    Real dxco2_dT, dyh2o_dT;
    smoothCubicInterpolation(
        Tint, xco2_lower, dxco2_dT_lower, xco2_upper, dxco2_dT_upper, xco2, dxco2_dT);
    smoothCubicInterpolation(
        Tint, yh2o_lower, dyh2o_dT_lower, yh2o_upper, dyh2o_dT_upper, yh2o, dyh2o_dT);
  }
  else
  {
    // Equilibrium mole fractions solved using iteration in this regime
    solveEquilibriumMoleFractionHighTemp(pressure, temperature, Xnacl, co2_density, xco2, yh2o);

    // Can use these in funcABHighTemp() to get derivatives analyticall rather than by iteration
    Real A, dA_dp, dA_dT, B, dB_dp, dB_dT, dB_dX;
    funcABHighTemp(pressure,
                   temperature,
                   Xnacl,
                   co2_density,
                   xco2,
                   yh2o,
                   A,
                   dA_dp,
                   dA_dT,
                   B,
                   dB_dp,
                   dB_dT,
                   dB_dX);

    dyh2o_dp = ((1.0 - B) * dA_dp + (A - 1.0) * A * dB_dp) / (1.0 - A * B) / (1.0 - A * B);
    dxco2_dp = dB_dp * (1.0 - yh2o) - B * dyh2o_dp;

    dyh2o_dT = ((1.0 - B) * dA_dT + (A - 1.0) * A * dB_dT) / (1.0 - A * B) / (1.0 - A * B);
    dxco2_dT = dB_dT * (1.0 - yh2o) - B * dyh2o_dT;

    dyh2o_dX = ((A - 1.0) * A * dB_dX) / (1.0 - A * B) / (1.0 - A * B);
    dxco2_dX = dB_dX * (1.0 - yh2o) - B * dyh2o_dX;
  }
}

void
PorousFlowBrineCO2::equilibriumMoleFractionsLowTemp(Real pressure,
                                                    Real temperature,
                                                    Real Xnacl,
                                                    Real & xco2,
                                                    Real & dxco2_dp,
                                                    Real & dxco2_dT,
                                                    Real & dxco2_dX,
                                                    Real & yh2o,
                                                    Real & dyh2o_dp,
                                                    Real & dyh2o_dT,
                                                    Real & dyh2o_dX) const
{
  if (temperature > 373.15)
    mooseError(name(),
               ": equilibriumMoleFractionsLowTemp() is not valid for T > 373.15K. Use "
               "equilibriumMoleFractions() instead");

  // CO2 density and derivatives wrt pressure and temperature
  Real co2_density, dco2_density_dp, dco2_density_dT;
  _co2_fp.rho_from_p_T(pressure, temperature, co2_density, dco2_density_dp, dco2_density_dT);

  // Assume infinite dilution (yh20 = 0 and xco2 = 0) in low temperature regime
  Real A, dA_dp, dA_dT, B, dB_dp, dB_dT;
  funcABLowTemp(pressure,
                temperature,
                co2_density,
                dco2_density_dp,
                dco2_density_dT,
                A,
                dA_dp,
                dA_dT,
                B,
                dB_dp,
                dB_dT);

  // As the activity coefficient for CO2 in brine used in this regime isn't a 'true'
  // activity coefficient, we instead calculate the molality of CO2 in water, then
  // correct it for brine, and then calculate the mole fractions.
  // The mole fraction in pure water is
  const Real yh2ow = (1.0 - B) / (1.0 / A - B);
  const Real xco2w = B * (1.0 - yh2ow);

  // Molality of CO2 in pure water
  const Real mco2w = xco2w * _invMh2o / (1.0 - xco2w);
  // Molality of CO2 in brine is then calculated using gamma
  Real gamma, dgamma_dp, dgamma_dT, dgamma_dX;
  activityCoefficient(pressure, temperature, Xnacl, gamma, dgamma_dp, dgamma_dT, dgamma_dX);
  const Real mco2 = mco2w / gamma;

  // Need NaCl molality (mol/kg)
  const Real mnacl = Xnacl / (1.0 - Xnacl) / _Mnacl;
  const Real dmnacl_dX = 1.0 / (1.0 - Xnacl) / (1.0 - Xnacl) / _Mnacl;

  // Mole fractions of CO2 and H2O in liquid and gas phases
  const Real total_moles = 2.0 * mnacl + _invMh2o + mco2;
  xco2 = mco2 / total_moles;
  yh2o = A * (1.0 - xco2 - 2.0 * mnacl / total_moles);

  // The derivatives of the mole fractions wrt pressure
  const Real dyh2ow_dp =
      ((1.0 - B) * dA_dp + (A - 1.0) * A * dB_dp) / (1.0 - A * B) / (1.0 - A * B);
  const Real dxco2w_dp = dB_dp * (1.0 - yh2ow) - B * dyh2ow_dp;

  const Real dmco2w_dp = _invMh2o * dxco2w_dp / (1.0 - xco2w) / (1.0 - xco2w);
  const Real dmco2_dp = dmco2w_dp / gamma - mco2w * dgamma_dp / gamma / gamma;
  dxco2_dp = (2.0 * mnacl + _invMh2o) * dmco2_dp / total_moles / total_moles;

  dyh2o_dp = (1.0 - xco2 - 2.0 * mnacl / total_moles) * dA_dp - A * dxco2_dp +
             2.0 * A * mnacl * dmco2_dp / total_moles / total_moles;

  // The derivatives of the mole fractions wrt temperature
  const Real dyh2ow_dT =
      ((1.0 - B) * dA_dT + (A - 1.0) * A * dB_dT) / (1.0 - A * B) / (1.0 - A * B);
  const Real dxco2w_dT = dB_dT * (1.0 - yh2ow) - B * dyh2ow_dT;

  const Real dmco2w_dT = _invMh2o * dxco2w_dT / (1.0 - xco2w) / (1.0 - xco2w);
  const Real dmco2_dT = dmco2w_dT / gamma - mco2w * dgamma_dT / gamma / gamma;
  dxco2_dT = (2.0 * mnacl + _invMh2o) * dmco2_dT / total_moles / total_moles;

  dyh2o_dT = (1.0 - xco2 - 2.0 * mnacl / total_moles) * dA_dT - A * dxco2_dT +
             2.0 * A * mnacl * dmco2_dT / total_moles / total_moles;

  // The derivatives of the mole fractions wrt salt mass fraction
  const Real dmco2_dX = -mco2w * dgamma_dX / gamma / gamma;
  dxco2_dX =
      ((2.0 * mnacl + _invMh2o) * dmco2_dX - 2.0 * mco2 * dmnacl_dX) / total_moles / total_moles;
  dyh2o_dX =
      A * (2.0 * (mnacl * dmco2_dX - (mco2 + _invMh2o) * dmnacl_dX) / total_moles / total_moles -
           dxco2_dX);
}

void
PorousFlowBrineCO2::funcABLowTemp(Real pressure,
                                  Real temperature,
                                  Real co2_density,
                                  Real dco2_density_dp,
                                  Real dco2_density_dT,
                                  Real & A,
                                  Real & dA_dp,
                                  Real & dA_dT,
                                  Real & B,
                                  Real & dB_dp,
                                  Real & dB_dT) const
{
  if (temperature > 373.15)
    mooseError(name(),
               ": funcABLowTemp() is not valid for T > 373.15K. Use funcABHighTemp() instead");

  // Pressure in bar
  const Real pbar = pressure * 1.0e-5;

  // Reference pressure and partial molar volumes
  const Real pref = 1.0;
  const Real vCO2 = 32.6;
  const Real vH2O = 18.1;

  const Real delta_pbar = pbar - pref;
  const Real Rt = _Rbar * temperature;

  // Equilibrium constants
  Real K0H2O, dK0H2O_dT, K0CO2, dK0CO2_dT;
  equilibriumConstantH2O(temperature, K0H2O, dK0H2O_dT);
  equilibriumConstantCO2(temperature, K0CO2, dK0CO2_dT);

  // Fugacity coefficients
  Real phiH2O, dphiH2O_dp, dphiH2O_dT;
  Real phiCO2, dphiCO2_dp, dphiCO2_dT;
  fugacityCoefficientsLowTemp(pressure,
                              temperature,
                              co2_density,
                              dco2_density_dp,
                              dco2_density_dT,
                              phiCO2,
                              dphiCO2_dp,
                              dphiCO2_dT,
                              phiH2O,
                              dphiH2O_dp,
                              dphiH2O_dT);

  A = K0H2O / (phiH2O * pbar) * std::exp(delta_pbar * vH2O / Rt);
  B = phiCO2 * pbar / (_invMh2o * K0CO2) * std::exp(-delta_pbar * vCO2 / Rt);

  dA_dp = (-1.0e-5 / pbar + 1.0e-5 * vH2O / Rt - dphiH2O_dp / phiH2O) * A;

  dB_dp = (1.0e-5 * phiCO2 + pbar * dphiCO2_dp - 1.0e-5 * vCO2 * pbar * phiCO2 / Rt) *
          std::exp(-delta_pbar * vCO2 / Rt) / (_invMh2o * K0CO2);

  dA_dT =
      (dK0H2O_dT - dphiH2O_dT * K0H2O / phiH2O - delta_pbar * vH2O * K0H2O / (Rt * temperature)) *
      std::exp(delta_pbar * vH2O / Rt) / (pbar * phiH2O);

  dB_dT = (-pbar * phiCO2 * dK0CO2_dT / K0CO2 + pbar * dphiCO2_dT +
           delta_pbar * vCO2 * pbar * phiCO2 / (Rt * temperature)) *
          std::exp(-delta_pbar * vCO2 / Rt) / (_invMh2o * K0CO2);
}

void
PorousFlowBrineCO2::funcABHighTemp(Real pressure,
                                   Real temperature,
                                   Real Xnacl,
                                   Real co2_density,
                                   Real xco2,
                                   Real yh2o,
                                   Real & A,
                                   Real & B) const
{
  if (temperature <= 373.15)
    mooseError(name(),
               ": funcABHighTemp() is not valid for T <= 373.15K. Use funcABLowTemp() instead");

  // Pressure in bar
  const Real pbar = pressure * 1.0e-5;
  // Temperature in C
  const Real Tc = temperature - _T_c2k;

  // Reference pressure and partial molar volumes
  const Real pref = -1.9906e-1 + 2.0471e-3 * Tc + 1.0152e-4 * Tc * Tc - 1.4234e-6 * Tc * Tc * Tc +
                    1.4168e-8 * Tc * Tc * Tc * Tc;
  const Real vCO2 = 32.6 + 3.413e-2 * (Tc - 100.0);
  const Real vH2O = 18.1 + 3.137e-2 * (Tc - 100.0);

  const Real delta_pbar = pbar - pref;
  const Real Rt = _Rbar * temperature;

  // Equilibrium constants
  Real K0H2O, dK0H2O_dT, K0CO2, dK0CO2_dT;
  equilibriumConstantH2O(temperature, K0H2O, dK0H2O_dT);
  equilibriumConstantCO2(temperature, K0CO2, dK0CO2_dT);

  // Fugacity coefficients
  Real phiH2O, phiCO2;
  fugacityCoefficientsHighTemp(pressure, temperature, co2_density, xco2, yh2o, phiCO2, phiH2O);

  // Activity coefficients
  const Real gammaH2O = activityCoefficientH2O(temperature, xco2);
  const Real gammaCO2 = activityCoefficientCO2(temperature, xco2);

  // Activity coefficient for CO2 in brine
  Real gamma, dgamma_dT, dgamma_dX;
  activityCoefficientHighTemp(temperature, Xnacl, gamma, dgamma_dT, dgamma_dX);

  A = K0H2O * gammaH2O / (phiH2O * pbar) * std::exp(delta_pbar * vH2O / Rt);
  B = phiCO2 * pbar / (_invMh2o * K0CO2 * gamma * gammaCO2) * std::exp(-delta_pbar * vCO2 / Rt);
}

void
PorousFlowBrineCO2::funcABHighTemp(Real pressure,
                                   Real temperature,
                                   Real Xnacl,
                                   Real co2_density,
                                   Real xco2,
                                   Real yh2o,
                                   Real & A,
                                   Real & dA_dp,
                                   Real & dA_dT,
                                   Real & B,
                                   Real & dB_dp,
                                   Real & dB_dT,
                                   Real & dB_dX) const
{
  funcABHighTemp(pressure, temperature, Xnacl, co2_density, xco2, yh2o, A, B);

  // Use finite differences for derivatives in the high temperature regime
  const Real dp = 1.0e-2;
  const Real dT = 1.0e-6;
  const Real dX = 1.0e-8;

  Real A2, B2;
  funcABHighTemp(pressure + dp, temperature, Xnacl, co2_density, xco2, yh2o, A2, B2);
  dA_dp = (A2 - A) / dp;
  dB_dp = (B2 - B) / dp;

  funcABHighTemp(pressure, temperature + dT, Xnacl, co2_density, xco2, yh2o, A2, B2);
  dA_dT = (A2 - A) / dT;
  dB_dT = (B2 - B) / dT;

  funcABHighTemp(pressure, temperature, Xnacl + dX, co2_density, xco2, yh2o, A2, B2);
  dB_dX = (B2 - B) / dX;
}

void
PorousFlowBrineCO2::solveEquilibriumMoleFractionHighTemp(
    Real pressure, Real temperature, Real Xnacl, Real co2_density, Real & xco2, Real & yh2o) const
{
  // Initial guess for yh2o and xco2 (from Spycher and Pruess (2010))
  Real y = _brine_fp.vaporPressure(temperature, 0.0) / pressure;
  Real x = 0.009;

  // Need salt mass fraction in molality
  const Real mnacl = Xnacl / (1.0 - Xnacl) / _Mnacl;

  // If y > 1, then just use y = 1, x = 0 (only a gas phase)
  if (y >= 1.0)
  {
    y = 1.0;
    x = 0.0;
  }
  else
  {
    // Residual function for Newton-Raphson
    auto fy = [mnacl, this](Real y, Real A, Real B) {
      return y -
             (1.0 - B) * _invMh2o / ((1.0 / A - B) * (2.0 * mnacl + _invMh2o) + 2.0 * mnacl * B);
    };

    // Derivative of fy wrt y
    auto dfy = [mnacl, this](Real A, Real B, Real dA, Real dB) {
      const Real denominator = (1.0 / A - B) * (2.0 * mnacl + _invMh2o) + 2.0 * mnacl * B;
      return 1.0 + _invMh2o * dB / denominator +
             (1.0 - B) * _invMh2o *
                 (2.0 * mnacl * dB - (2.0 * mnacl + _invMh2o) * (dB + dA / A / A)) / denominator /
                 denominator;
    };

    Real A, B;
    Real dA, dB;
    const Real dy = 1.0e-8;

    // Solve for yh2o using Newton-Raphson method
    unsigned int iter = 0;
    const Real tol = 1.0e-12;
    const unsigned int max_its = 10;
    funcABHighTemp(pressure, temperature, Xnacl, co2_density, x, y, A, B);

    while (std::abs(fy(y, A, B)) > tol)
    {
      funcABHighTemp(pressure, temperature, Xnacl, co2_density, x, y, A, B);
      // Finite difference derivatives of A and B wrt y
      funcABHighTemp(pressure, temperature, Xnacl, co2_density, x, y + dy, dA, dB);
      dA = (dA - A) / dy;
      dB = (dB - B) / dy;

      y = y - fy(y, A, B) / dfy(A, B, dA, dB);

      x = B * (1.0 - y);

      // Break if not converged and just use the value
      if (iter > max_its)
        break;
    }
  }

  yh2o = y;
  xco2 = x;
}

void
PorousFlowBrineCO2::partialDensityCO2(Real temperature,
                                      Real & partial_density,
                                      Real & dpartial_density_dT) const
{
  // This correlation uses temperature in C
  const Real Tc = temperature - _T_c2k;
  // The parial molar volume
  const Real V = 37.51 - 9.585e-2 * Tc + 8.74e-4 * Tc * Tc - 5.044e-7 * Tc * Tc * Tc;
  const Real dV_dT = -9.585e-2 + 1.748e-3 * Tc - 1.5132e-6 * Tc * Tc;

  partial_density = 1.0e6 * _Mco2 / V;
  dpartial_density_dT = -1.0e6 * _Mco2 * dV_dT / V / V;
}

Real
PorousFlowBrineCO2::totalMassFraction(
    Real pressure, Real temperature, Real Xnacl, Real saturation, unsigned qp) const
{
  // Check whether the input pressure and temperature are within the region of validity
  checkVariables(pressure, temperature);

  // FluidStateProperties data structure
  std::vector<FluidStateProperties> fsp(_num_phases, FluidStateProperties(_num_components));
  FluidStateProperties & liquid = fsp[_aqueous_phase_number];
  FluidStateProperties & gas = fsp[_gas_phase_number];

  // Calculate equilibrium mass fractions in the two-phase state
  Real Xco2, dXco2_dp, dXco2_dT, dXco2_dX, Yh2o, dYh2o_dp, dYh2o_dT, dYh2o_dX;
  equilibriumMassFractions(pressure,
                           temperature,
                           Xnacl,
                           Xco2,
                           dXco2_dp,
                           dXco2_dT,
                           dXco2_dX,
                           Yh2o,
                           dYh2o_dp,
                           dYh2o_dT,
                           dYh2o_dX);

  // Save the mass fractions in the FluidStateMassFractions object
  const Real Yco2 = 1.0 - Yh2o;
  liquid.mass_fraction[_aqueous_fluid_component] = 1.0 - Xco2;
  liquid.mass_fraction[_gas_fluid_component] = Xco2;
  gas.mass_fraction[_aqueous_fluid_component] = Yh2o;
  gas.mass_fraction[_gas_fluid_component] = Yco2;

  // Gas properties
  gasProperties(pressure, temperature, fsp);

  // Liquid properties
  const Real liquid_saturation = 1.0 - saturation;
  const Real liquid_pressure = pressure - _pc_uo.capillaryPressure(liquid_saturation, qp);
  liquidProperties(liquid_pressure, temperature, Xnacl, fsp);

  // The total mass fraction of ncg (z) can now be calculated
  const Real Z = (saturation * gas.density * Yco2 + liquid_saturation * liquid.density * Xco2) /
                 (saturation * gas.density + liquid_saturation * liquid.density);

  return Z;
}

void
PorousFlowBrineCO2::henryConstant(
    Real temperature, Real Xnacl, Real & Kh, Real & dKh_dT, Real & dKh_dX) const
{
  // Henry's constant for dissolution in water
  Real Kh_h2o, dKh_h2o_dT;
  _co2_fp.henryConstant_dT(temperature, Kh_h2o, dKh_h2o_dT);

  // The correction to salt is obtained through the salting out coefficient
  const std::vector<Real> b{1.19784e-1, -7.17823e-4, 4.93854e-6, -1.03826e-8, 1.08233e-11};

  // Need temperature in Celcius
  const Real Tc = temperature - _T_c2k;

  Real kb = 0.0;
  for (unsigned int i = 0; i < b.size(); ++i)
    kb += b[i] * MathUtils::pow(Tc, i);

  Real dkb_dT = 0.0;
  for (unsigned int i = 1; i < b.size(); ++i)
    dkb_dT += i * b[i] * MathUtils::pow(Tc, i - 1);

  // Need salt mass fraction in molality
  const Real xmol = Xnacl / (1.0 - Xnacl) / _Mnacl;
  const Real dxmol_dX = 1.0 / (1.0 - Xnacl) / (1.0 - Xnacl) / _Mnacl;
  // Henry's constant and its derivative wrt temperature and salt mass fraction
  Kh = Kh_h2o * std::pow(10.0, xmol * kb);
  dKh_dT = (xmol * std::log(10.0) * dkb_dT + dKh_h2o_dT / Kh_h2o) * Kh;
  dKh_dX = std::log(10.0) * kb * dxmol_dX * Kh;
}

void
PorousFlowBrineCO2::enthalpyOfDissolutionGas(
    Real temperature, Real Xnacl, Real & hdis, Real & dhdis_dT, Real & dhdis_dX) const
{
  // Henry's constant
  Real Kh, dKh_dT, dKh_dX;
  henryConstant(temperature, Xnacl, Kh, dKh_dT, dKh_dX);

  hdis = -_R * temperature * temperature * dKh_dT / Kh / _Mco2;

  // Derivative of enthalpy of dissolution wrt temperature and xnacl requires the second
  // derivatives of Henry's constant. For simplicity, approximate these numerically
  const Real dT = temperature * 1.0e-8;
  const Real T2 = temperature + dT;
  henryConstant(T2, Xnacl, Kh, dKh_dT, dKh_dX);

  dhdis_dT = (-_R * T2 * T2 * dKh_dT / Kh / _Mco2 - hdis) / dT;

  const Real dX = Xnacl * 1.0e-8;
  const Real X2 = Xnacl + dX;
  henryConstant(temperature, X2, Kh, dKh_dT, dKh_dX);

  dhdis_dX = (-_R * temperature * temperature * dKh_dT / Kh / _Mco2 - hdis) / dX;
}

void
PorousFlowBrineCO2::enthalpyOfDissolution(Real temperature, Real & hdis, Real & dhdis_dT) const
{
  // Linear fit to model of Duan and Sun (2003) (in kJ/mol)
  const Real delta_h = -58.3533 + 0.134519 * temperature;

  // Convert to J/kg
  hdis = delta_h * 1000.0 / _Mco2;
  dhdis_dT = 134.519 / _Mco2;
}

void
PorousFlowBrineCO2::smoothCubicInterpolation(
    Real temperature, Real f0, Real df0, Real f1, Real df1, Real & value, Real & deriv) const
{
  // Coefficients of cubic polynomial
  const Real dT = _Tupper - _Tlower;

  const Real a = f0;
  const Real b = df0 * dT;
  const Real c = 3.0 * (f1 - f0) - (2.0 * df0 + df1) * dT;
  const Real d = 2.0 * (f0 - f1) + (df0 + df1) * dT;

  const Real t2 = temperature * temperature;
  const Real t3 = temperature * t2;

  value = a + b * temperature + c * t2 + d * t3;
  deriv = b + 2.0 * c * temperature + 3.0 * d * t2;
}

void
PorousFlowBrineCO2::checkVariables(Real pressure, Real temperature) const
{
  // The calculation of mass fractions is valid from 12C <= T <= 300C, and
  // pressure less than 60 MPa
  if (temperature < 285.15 || temperature > 573.15)
    mooseException(name() + ": temperature " + Moose::stringify(temperature) +
                   " is outside range 285.15 K <= T <= 573.15 K");

  if (pressure > 6.0e7)
    mooseException(name() + ": pressure " + Moose::stringify(pressure) +
                   " must be less than 60 MPa");
}
