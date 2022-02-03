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
#include "SinglePhaseFluidProperties.h"
#include "MathUtils.h"
#include "Conversion.h"

registerMooseObject("PorousFlowApp", PorousFlowBrineCO2);

InputParameters
PorousFlowBrineCO2::validParams()
{
  InputParameters params = PorousFlowFluidStateMultiComponentBase::validParams();
  params.addRequiredParam<UserObjectName>("brine_fp", "The name of the user object for brine");
  params.addRequiredParam<UserObjectName>("co2_fp", "The name of the user object for CO2");
  params.addParam<unsigned int>("salt_component", 2, "The component number of salt");
  params.addClassDescription("Fluid state class for brine and CO2");
  return params;
}

PorousFlowBrineCO2::PorousFlowBrineCO2(const InputParameters & parameters)
  : PorousFlowFluidStateMultiComponentBase(parameters),
    _salt_component(getParam<unsigned int>("salt_component")),
    _brine_fp(getUserObject<BrineFluidProperties>("brine_fp")),
    _co2_fp(getUserObject<SinglePhaseFluidProperties>("co2_fp")),
    _Mh2o(_brine_fp.molarMassH2O()),
    _invMh2o(1.0 / _Mh2o),
    _Mco2(_co2_fp.molarMass()),
    _Mnacl(_brine_fp.molarMassNaCl()),
    _Rbar(_R * 10.0),
    _Tlower(372.15),
    _Tupper(382.15),
    _Zmin(1.0e-4),
    _co2_henry(_co2_fp.henryCoefficients())
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

  _empty_fsp = FluidStateProperties(_num_components);
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
                                             unsigned int qp,
                                             std::vector<FluidStateProperties> & fsp) const
{
  FluidStateProperties & liquid = fsp[_aqueous_phase_number];
  FluidStateProperties & gas = fsp[_gas_phase_number];

  // Check whether the input temperature is within the region of validity
  checkVariables(pressure, temperature);

  // AD versions of primary variables
  DualReal p = pressure;
  Moose::derivInsert(p.derivatives(), _pidx, 1.0);
  DualReal T = temperature;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);
  DualReal Zco2 = Z;
  Moose::derivInsert(Zco2.derivatives(), _Zidx, 1.0);
  DualReal X = Xnacl;
  Moose::derivInsert(X.derivatives(), _Xidx, 1.0);

  // Clear all of the FluidStateProperties data
  clearFluidStateProperties(fsp);

  FluidStatePhaseEnum phase_state;
  massFractions(p, T, X, Zco2, phase_state, fsp);

  switch (phase_state)
  {
    case FluidStatePhaseEnum::GAS:
    {
      // Set the gas saturations
      gas.saturation = 1.0;

      // Calculate gas properties
      gasProperties(p, T, fsp);

      break;
    }

    case FluidStatePhaseEnum::LIQUID:
    {
      // Calculate the liquid properties
      const DualReal liquid_pressure = p - _pc.capillaryPressure(1.0, qp);
      liquidProperties(liquid_pressure, T, X, fsp);

      break;
    }

    case FluidStatePhaseEnum::TWOPHASE:
    {
      // Calculate the gas and liquid properties in the two phase region
      twoPhaseProperties(p, T, X, Zco2, qp, fsp);

      break;
    }
  }

  // Liquid saturations can now be set
  liquid.saturation = 1.0 - gas.saturation;

  // Save pressures to FluidStateProperties object
  gas.pressure = p;
  liquid.pressure = p - _pc.capillaryPressure(liquid.saturation, qp);
}

void
PorousFlowBrineCO2::massFractions(const DualReal & pressure,
                                  const DualReal & temperature,
                                  const DualReal & Xnacl,
                                  const DualReal & Z,
                                  FluidStatePhaseEnum & phase_state,
                                  std::vector<FluidStateProperties> & fsp) const
{
  FluidStateProperties & liquid = fsp[_aqueous_phase_number];
  FluidStateProperties & gas = fsp[_gas_phase_number];

  DualReal Xco2 = 0.0;
  DualReal Yh2o = 0.0;
  DualReal Yco2 = 0.0;

  // If the amount of CO2 is less than the smallest solubility, then all CO2 will
  // be dissolved, and the equilibrium mass fractions do not need to be computed
  if (Z < _Zmin)
    phase_state = FluidStatePhaseEnum::LIQUID;

  else
  {
    // Equilibrium mass fraction of CO2 in liquid and H2O in gas phases
    equilibriumMassFractions(pressure, temperature, Xnacl, Xco2, Yh2o);

    Yco2 = 1.0 - Yh2o;

    // Determine which phases are present based on the value of z
    phaseState(Z.value(), Xco2.value(), Yco2.value(), phase_state);
  }

  // The equilibrium mass fractions calculated above are only correct in the two phase
  // state. If only liquid or gas phases are present, the mass fractions are given by
  // the total mass fraction z
  DualReal Xh2o = 0.0;

  switch (phase_state)
  {
    case FluidStatePhaseEnum::LIQUID:
    {
      Xco2 = Z;
      Yco2 = 0.0;
      Xh2o = 1.0 - Z;
      Yh2o = 0.0;
      Moose::derivInsert(Xco2.derivatives(), _pidx, 0.0);
      Moose::derivInsert(Xco2.derivatives(), _Tidx, 0.0);
      Moose::derivInsert(Xco2.derivatives(), _Xidx, 0.0);
      Moose::derivInsert(Xco2.derivatives(), _Zidx, 1.0);
      Moose::derivInsert(Yco2.derivatives(), _pidx, 0.0);
      Moose::derivInsert(Yco2.derivatives(), _Tidx, 0.0);
      Moose::derivInsert(Yco2.derivatives(), _Xidx, 0.0);
      break;
    }

    case FluidStatePhaseEnum::GAS:
    {
      Xco2 = 0.0;
      Yco2 = Z;
      Yh2o = 1.0 - Z;
      Moose::derivInsert(Xco2.derivatives(), _pidx, 0.0);
      Moose::derivInsert(Xco2.derivatives(), _Tidx, 0.0);
      Moose::derivInsert(Xco2.derivatives(), _Xidx, 0.0);
      Moose::derivInsert(Yco2.derivatives(), _pidx, 0.0);
      Moose::derivInsert(Yco2.derivatives(), _Tidx, 0.0);
      Moose::derivInsert(Yco2.derivatives(), _Xidx, 0.0);
      Moose::derivInsert(Yco2.derivatives(), _Zidx, 1.0);
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
}

void
PorousFlowBrineCO2::gasProperties(const DualReal & pressure,
                                  const DualReal & temperature,
                                  std::vector<FluidStateProperties> & fsp) const
{
  FluidStateProperties & gas = fsp[_gas_phase_number];

  // Gas density, viscosity and enthalpy are approximated with pure CO2 - no correction due
  // to the small amount of water vapor is made
  DualReal co2_density, co2_viscosity;
  _co2_fp.rho_mu_from_p_T(pressure, temperature, co2_density, co2_viscosity);

  DualReal co2_enthalpy = _co2_fp.h_from_p_T(pressure, temperature);

  // Save the values to the FluidStateProperties object. Note that derivatives wrt z are 0
  gas.density = co2_density;
  gas.viscosity = co2_viscosity;
  gas.enthalpy = co2_enthalpy;

  mooseAssert(gas.density.value() > 0.0, "Gas density must be greater than zero");
  gas.internal_energy = gas.enthalpy - pressure / gas.density;
}

void
PorousFlowBrineCO2::liquidProperties(const DualReal & pressure,
                                     const DualReal & temperature,
                                     const DualReal & Xnacl,
                                     std::vector<FluidStateProperties> & fsp) const
{
  FluidStateProperties & liquid = fsp[_aqueous_phase_number];

  // The liquid density includes the density increase due to dissolved CO2
  const DualReal brine_density = _brine_fp.rho_from_p_T_X(pressure, temperature, Xnacl);

  // Mass fraction of CO2 in liquid phase
  const DualReal Xco2 = liquid.mass_fraction[_gas_fluid_component];

  // The liquid density
  const DualReal co2_partial_density = partialDensityCO2(temperature);

  const DualReal liquid_density = 1.0 / (Xco2 / co2_partial_density + (1.0 - Xco2) / brine_density);

  // Assume that liquid viscosity is just the brine viscosity
  const DualReal liquid_viscosity = _brine_fp.mu_from_p_T_X(pressure, temperature, Xnacl);

  // Liquid enthalpy (including contribution due to the enthalpy of dissolution)
  const DualReal brine_enthalpy = _brine_fp.h_from_p_T_X(pressure, temperature, Xnacl);

  // Enthalpy of CO2
  const DualReal co2_enthalpy = _co2_fp.h_from_p_T(pressure, temperature);

  // Enthalpy of dissolution
  const DualReal hdis = enthalpyOfDissolution(temperature);

  const DualReal liquid_enthalpy = (1.0 - Xco2) * brine_enthalpy + Xco2 * (co2_enthalpy + hdis);

  // Save the values to the FluidStateProperties object
  liquid.density = liquid_density;
  liquid.viscosity = liquid_viscosity;
  liquid.enthalpy = liquid_enthalpy;

  mooseAssert(liquid.density.value() > 0.0, "Liquid density must be greater than zero");
  liquid.internal_energy = liquid.enthalpy - pressure / liquid.density;
}

DualReal
PorousFlowBrineCO2::saturation(const DualReal & pressure,
                               const DualReal & temperature,
                               const DualReal & Xnacl,
                               const DualReal & Z,
                               std::vector<FluidStateProperties> & fsp) const
{
  auto & gas = fsp[_gas_phase_number];
  auto & liquid = fsp[_aqueous_fluid_component];

  // Approximate liquid density as saturation isn't known yet, by using the gas
  // pressure rather than the liquid pressure. This does result in a small error
  // in the calculated saturation, but this is below the error associated with
  // the correlations. A more accurate saturation could be found iteraviely,
  // at the cost of increased computational expense

  // Gas density
  const DualReal gas_density = _co2_fp.rho_from_p_T(pressure, temperature);

  // Approximate liquid density as saturation isn't known yet
  const DualReal brine_density = _brine_fp.rho_from_p_T_X(pressure, temperature, Xnacl);

  // Mass fraction of CO2 in liquid phase
  const DualReal Xco2 = liquid.mass_fraction[_gas_fluid_component];

  // The liquid density
  const DualReal co2_partial_density = partialDensityCO2(temperature);

  const DualReal liquid_density = 1.0 / (Xco2 / co2_partial_density + (1.0 - Xco2) / brine_density);

  const DualReal Yco2 = gas.mass_fraction[_gas_fluid_component];

  // Set mass equilibrium constants used in the calculation of vapor mass fraction
  const DualReal K0 = Yco2 / Xco2;
  const DualReal K1 = (1.0 - Yco2) / (1.0 - Xco2);
  const DualReal vapor_mass_fraction = vaporMassFraction(Z, K0, K1);

  // The gas saturation in the two phase case
  const DualReal saturation = vapor_mass_fraction * liquid_density /
                              (gas_density + vapor_mass_fraction * (liquid_density - gas_density));

  return saturation;
}

void
PorousFlowBrineCO2::twoPhaseProperties(const DualReal & pressure,
                                       const DualReal & temperature,
                                       const DualReal & Xnacl,
                                       const DualReal & Z,
                                       unsigned int qp,
                                       std::vector<FluidStateProperties> & fsp) const
{
  auto & gas = fsp[_gas_phase_number];

  // Calculate all of the gas phase properties, as these don't depend on saturation
  gasProperties(pressure, temperature, fsp);

  // The gas saturation in the two phase case
  gas.saturation = saturation(pressure, temperature, Xnacl, Z, fsp);

  // The liquid pressure and properties can now be calculated
  const DualReal liquid_pressure = pressure - _pc.capillaryPressure(1.0 - gas.saturation, qp);
  liquidProperties(liquid_pressure, temperature, Xnacl, fsp);
}

void
PorousFlowBrineCO2::equilibriumMassFractions(const DualReal & pressure,
                                             const DualReal & temperature,
                                             const DualReal & Xnacl,
                                             DualReal & Xco2,
                                             DualReal & Yh2o) const
{
  // Mole fractions at equilibrium
  DualReal xco2, yh2o;
  equilibriumMoleFractions(pressure, temperature, Xnacl, xco2, yh2o);

  // The mass fraction of H2O in gas (assume no salt in gas phase) and derivatives
  // wrt p, T, and X
  Yh2o = yh2o * _Mh2o / (yh2o * _Mh2o + (1.0 - yh2o) * _Mco2);

  // NaCl molality (mol/kg)
  const DualReal mnacl = Xnacl / (1.0 - Xnacl) / _Mnacl;

  // The molality of CO2 in 1kg of H2O
  const DualReal mco2 = xco2 * (2.0 * mnacl + _invMh2o) / (1.0 - xco2);
  // The mass fraction of CO2 in brine is then
  const DualReal denominator = (1.0 + mnacl * _Mnacl + mco2 * _Mco2);
  Xco2 = mco2 * _Mco2 / denominator;
}

void
PorousFlowBrineCO2::fugacityCoefficientsLowTemp(const DualReal & pressure,
                                                const DualReal & temperature,
                                                const DualReal & co2_density,
                                                DualReal & fco2,
                                                DualReal & fh2o) const
{
  if (temperature.value() > 373.15)
    mooseError(name(),
               ": fugacityCoefficientsLowTemp() is not valid for T > 373.15K. Use "
               "fugacityCoefficientsHighTemp() instead");

  // Need pressure in bar
  const DualReal pbar = pressure * 1.0e-5;

  // Molar volume in cm^3/mol
  const DualReal V = _Mco2 / co2_density * 1.0e6;

  // Redlich-Kwong parameters
  const DualReal aCO2 = 7.54e7 - 4.13e4 * temperature;
  const Real bCO2 = 27.8;
  const Real aCO2H2O = 7.89e7;
  const Real bH2O = 18.18;

  const DualReal t15 = std::pow(temperature, 1.5);

  // The fugacity coefficients for H2O and CO2
  auto lnPhi = [V, aCO2, bCO2, t15, this](DualReal a, DualReal b)
  {
    return std::log(V / (V - bCO2)) + b / (V - bCO2) -
           2.0 * a / (_Rbar * t15 * bCO2) * std::log((V + bCO2) / V) +
           aCO2 * b / (_Rbar * t15 * bCO2 * bCO2) * (std::log((V + bCO2) / V) - bCO2 / (V + bCO2));
  };

  const DualReal lnPhiH2O = lnPhi(aCO2H2O, bH2O) - std::log(pbar * V / (_Rbar * temperature));
  const DualReal lnPhiCO2 = lnPhi(aCO2, bCO2) - std::log(pbar * V / (_Rbar * temperature));

  fh2o = std::exp(lnPhiH2O);
  fco2 = std::exp(lnPhiCO2);
}

void
PorousFlowBrineCO2::fugacityCoefficientsHighTemp(const DualReal & pressure,
                                                 const DualReal & temperature,
                                                 const DualReal & co2_density,
                                                 const DualReal & xco2,
                                                 const DualReal & yh2o,
                                                 DualReal & fco2,
                                                 DualReal & fh2o) const
{
  if (temperature.value() <= 373.15)
    mooseError(name(),
               ": fugacityCoefficientsHighTemp() is not valid for T <= 373.15K. Use "
               "fugacityCoefficientsLowTemp() instead");

  fh2o = fugacityCoefficientH2OHighTemp(pressure, temperature, co2_density, xco2, yh2o);
  fco2 = fugacityCoefficientCO2HighTemp(pressure, temperature, co2_density, xco2, yh2o);
}

DualReal
PorousFlowBrineCO2::fugacityCoefficientH2OHighTemp(const DualReal & pressure,
                                                   const DualReal & temperature,
                                                   const DualReal & co2_density,
                                                   const DualReal & xco2,
                                                   const DualReal & yh2o) const
{
  // Need pressure in bar
  const DualReal pbar = pressure * 1.0e-5;
  // Molar volume in cm^3/mol
  const DualReal V = _Mco2 / co2_density * 1.0e6;

  // Redlich-Kwong parameters
  const DualReal yco2 = 1.0 - yh2o;
  const DualReal xh2o = 1.0 - xco2;

  const DualReal aCO2 = 8.008e7 - 4.984e4 * temperature;
  const DualReal aH2O = 1.337e8 - 1.4e4 * temperature;
  const Real bCO2 = 28.25;
  const Real bH2O = 15.7;
  const DualReal KH2OCO2 = 1.427e-2 - 4.037e-4 * temperature;
  const DualReal KCO2H2O = 0.4228 - 7.422e-4 * temperature;
  const DualReal kH2OCO2 = KH2OCO2 * yh2o + KCO2H2O * yco2;
  const DualReal kCO2H2O = KCO2H2O * yh2o + KH2OCO2 * yco2;

  const DualReal aH2OCO2 = std::sqrt(aCO2 * aH2O) * (1.0 - kH2OCO2);
  const DualReal aCO2H2O = std::sqrt(aCO2 * aH2O) * (1.0 - kCO2H2O);

  const DualReal amix = yh2o * yh2o * aH2O + yh2o * yco2 * (aH2OCO2 + aCO2H2O) + yco2 * yco2 * aCO2;
  const DualReal bmix = yh2o * bH2O + yco2 * bCO2;

  const DualReal t15 = std::pow(temperature, 1.5);

  DualReal lnPhiH2O = bH2O / bmix * (pbar * V / (_Rbar * temperature) - 1.0) -
                      std::log(pbar * (V - bmix) / (_Rbar * temperature));
  DualReal term3 = (2.0 * yh2o * aH2O + yco2 * (aH2OCO2 + aCO2H2O) -
                    yh2o * yco2 * std::sqrt(aH2O * aCO2) * (kH2OCO2 - kCO2H2O) * (yh2o - yco2) +
                    xh2o * xco2 * std::sqrt(aH2O * aCO2) * (kH2OCO2 - kCO2H2O)) /
                   amix;
  term3 -= bH2O / bmix;
  term3 *= amix / (bmix * _Rbar * t15) * std::log(V / (V + bmix));
  lnPhiH2O += term3;

  return std::exp(lnPhiH2O);
}

DualReal
PorousFlowBrineCO2::fugacityCoefficientCO2HighTemp(const DualReal & pressure,
                                                   const DualReal & temperature,
                                                   const DualReal & co2_density,
                                                   const DualReal & xco2,
                                                   const DualReal & yh2o) const
{
  // Need pressure in bar
  const DualReal pbar = pressure * 1.0e-5;
  // Molar volume in cm^3/mol
  const DualReal V = _Mco2 / co2_density * 1.0e6;

  // Redlich-Kwong parameters
  const DualReal yco2 = 1.0 - yh2o;
  const DualReal xh2o = 1.0 - xco2;

  const DualReal aCO2 = 8.008e7 - 4.984e4 * temperature;
  const DualReal aH2O = 1.337e8 - 1.4e4 * temperature;
  const Real bCO2 = 28.25;
  const Real bH2O = 15.7;
  const DualReal KH2OCO2 = 1.427e-2 - 4.037e-4 * temperature;
  const DualReal KCO2H2O = 0.4228 - 7.422e-4 * temperature;
  const DualReal kH2OCO2 = KH2OCO2 * yh2o + KCO2H2O * yco2;
  const DualReal kCO2H2O = KCO2H2O * yh2o + KH2OCO2 * yco2;

  const DualReal aH2OCO2 = std::sqrt(aCO2 * aH2O) * (1.0 - kH2OCO2);
  const DualReal aCO2H2O = std::sqrt(aCO2 * aH2O) * (1.0 - kCO2H2O);

  const DualReal amix = yh2o * yh2o * aH2O + yh2o * yco2 * (aH2OCO2 + aCO2H2O) + yco2 * yco2 * aCO2;
  const DualReal bmix = yh2o * bH2O + yco2 * bCO2;

  const DualReal t15 = std::pow(temperature, 1.5);

  DualReal lnPhiCO2 = bCO2 / bmix * (pbar * V / (_Rbar * temperature) - 1.0) -
                      std::log(pbar * (V - bmix) / (_Rbar * temperature));

  DualReal term3 = (2.0 * yco2 * aCO2 + yh2o * (aH2OCO2 + aCO2H2O) -
                    yh2o * yco2 * std::sqrt(aH2O * aCO2) * (kH2OCO2 - kCO2H2O) * (yh2o - yco2) +
                    xh2o * xco2 * std::sqrt(aH2O * aCO2) * (kCO2H2O - kH2OCO2)) /
                   amix;

  lnPhiCO2 += (term3 - bCO2 / bmix) * amix / (bmix * _Rbar * t15) * std::log(V / (V + bmix));

  return std::exp(lnPhiCO2);
}

DualReal
PorousFlowBrineCO2::activityCoefficientH2O(const DualReal & temperature,
                                           const DualReal & xco2) const
{
  if (temperature.value() <= 373.15)
    return 1.0;
  else
  {
    const DualReal Tref = temperature - 373.15;
    const DualReal xh2o = 1.0 - xco2;
    const DualReal Am = -3.084e-2 * Tref + 1.927e-5 * Tref * Tref;

    return std::exp((Am - 2.0 * Am * xh2o) * xco2 * xco2);
  }
}

DualReal
PorousFlowBrineCO2::activityCoefficientCO2(const DualReal & temperature,
                                           const DualReal & xco2) const
{
  if (temperature.value() <= 373.15)
    return 1.0;
  else
  {
    const DualReal Tref = temperature - 373.15;
    const DualReal xh2o = 1.0 - xco2;
    const DualReal Am = -3.084e-2 * Tref + 1.927e-5 * Tref * Tref;

    return std::exp(2.0 * Am * xco2 * xh2o * xh2o);
  }
}

DualReal
PorousFlowBrineCO2::activityCoefficient(const DualReal & pressure,
                                        const DualReal & temperature,
                                        const DualReal & Xnacl) const
{
  // Need pressure in bar
  const DualReal pbar = pressure * 1.0e-5;
  // Need NaCl molality (mol/kg)
  const DualReal mnacl = Xnacl / (1.0 - Xnacl) / _Mnacl;

  const DualReal lambda = -0.411370585 + 6.07632013e-4 * temperature + 97.5347708 / temperature -
                          0.0237622469 * pbar / temperature +
                          0.0170656236 * pbar / (630.0 - temperature) +
                          1.41335834e-5 * temperature * std::log(pbar);

  const DualReal xi = 3.36389723e-4 - 1.9829898e-5 * temperature +
                      2.12220830e-3 * pbar / temperature -
                      5.24873303e-3 * pbar / (630.0 - temperature);

  return std::exp(2.0 * lambda * mnacl + xi * mnacl * mnacl);
}

DualReal
PorousFlowBrineCO2::activityCoefficientHighTemp(const DualReal & temperature,
                                                const DualReal & Xnacl) const
{
  // Need NaCl molality (mol/kg)
  const DualReal mnacl = Xnacl / (1.0 - Xnacl) / _Mnacl;

  const DualReal T2 = temperature * temperature;
  const DualReal T3 = temperature * T2;

  const DualReal lambda = 2.217e-4 * temperature + 1.074 / temperature + 2648.0 / T2;
  const DualReal xi = 1.3e-5 * temperature - 20.12 / temperature + 5259.0 / T2;

  return (1.0 - mnacl / _invMh2o) * std::exp(2.0 * lambda * mnacl + xi * mnacl * mnacl);
}

DualReal
PorousFlowBrineCO2::equilibriumConstantH2O(const DualReal & temperature) const
{
  // Uses temperature in Celsius
  const DualReal Tc = temperature - _T_c2k;
  const DualReal Tc2 = Tc * Tc;
  const DualReal Tc3 = Tc2 * Tc;
  const DualReal Tc4 = Tc3 * Tc;

  DualReal logK0H2O;

  if (Tc <= 99.0)
    logK0H2O = -2.209 + 3.097e-2 * Tc - 1.098e-4 * Tc2 + 2.048e-7 * Tc3;

  else if (Tc > 99.0 && Tc < 109.0)
  {
    const DualReal Tint = (Tc - 99.0) / 10.0;
    const DualReal Tint2 = Tint * Tint;
    logK0H2O = -0.0204026 + 0.0152513 * Tint + 0.417565 * Tint2 - 0.278636 * Tint * Tint2;
  }

  else // 109 <= Tc <= 300
    logK0H2O = -2.1077 + 2.8127e-2 * Tc - 8.4298e-5 * Tc2 + 1.4969e-7 * Tc3 - 1.1812e-10 * Tc4;

  return std::pow(10.0, logK0H2O);
}

DualReal
PorousFlowBrineCO2::equilibriumConstantCO2(const DualReal & temperature) const
{
  // Uses temperature in Celsius
  const DualReal Tc = temperature - _T_c2k;
  const DualReal Tc2 = Tc * Tc;
  const DualReal Tc3 = Tc2 * Tc;

  DualReal logK0CO2;

  if (Tc <= 99.0)
    logK0CO2 = 1.189 + 1.304e-2 * Tc - 5.446e-5 * Tc2;

  else if (Tc > 99.0 && Tc < 109.0)
  {
    const DualReal Tint = (Tc - 99.0) / 10.0;
    const DualReal Tint2 = Tint * Tint;
    logK0CO2 = 1.9462 + 2.25692e-2 * Tint - 9.49577e-3 * Tint2 - 6.77721e-3 * Tint * Tint2;
  }

  else // 109 <= Tc <= 300
    logK0CO2 = 1.668 + 3.992e-3 * Tc - 1.156e-5 * Tc2 + 1.593e-9 * Tc3;

  return std::pow(10.0, logK0CO2);
}

void
PorousFlowBrineCO2::equilibriumMoleFractions(const DualReal & pressure,
                                             const DualReal & temperature,
                                             const DualReal & Xnacl,
                                             DualReal & xco2,
                                             DualReal & yh2o) const
{
  if (temperature.value() <= _Tlower)
  {
    equilibriumMoleFractionsLowTemp(pressure, temperature, Xnacl, xco2, yh2o);
  }
  else if (temperature.value() > _Tlower && temperature.value() < _Tupper)
  {
    // Cubic polynomial in this regime
    const Real Tint = (temperature.value() - _Tlower) / 10.0;

    // Equilibrium mole fractions and derivatives at the lower temperature
    DualReal Tlower = _Tlower;
    Moose::derivInsert(Tlower.derivatives(), _Tidx, 1.0);

    DualReal xco2_lower, yh2o_lower;
    equilibriumMoleFractionsLowTemp(pressure, Tlower, Xnacl, xco2_lower, yh2o_lower);

    const Real dxco2_dT_lower = xco2_lower.derivatives()[_Tidx];
    const Real dyh2o_dT_lower = yh2o_lower.derivatives()[_Tidx];

    // Equilibrium mole fractions and derivatives at the upper temperature
    Real xco2_upper, yh2o_upper;
    Real co2_density_upper = _co2_fp.rho_from_p_T(pressure.value(), _Tupper);

    solveEquilibriumMoleFractionHighTemp(
        pressure.value(), _Tupper, Xnacl.value(), co2_density_upper, xco2_upper, yh2o_upper);

    Real A, dA_dp, dA_dT, B, dB_dp, dB_dT, dB_dX;
    funcABHighTemp(pressure.value(),
                   _Tupper,
                   Xnacl.value(),
                   co2_density_upper,
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
    Real xco2r, yh2or, dxco2_dT, dyh2o_dT;
    smoothCubicInterpolation(
        Tint, xco2_lower.value(), dxco2_dT_lower, xco2_upper, dxco2_dT_upper, xco2r, dxco2_dT);
    smoothCubicInterpolation(
        Tint, yh2o_lower.value(), dyh2o_dT_lower, yh2o_upper, dyh2o_dT_upper, yh2or, dyh2o_dT);

    xco2 = xco2r;
    Moose::derivInsert(xco2.derivatives(), _pidx, xco2_lower.derivatives()[_pidx]);
    Moose::derivInsert(xco2.derivatives(), _Tidx, dxco2_dT);
    Moose::derivInsert(xco2.derivatives(), _Xidx, xco2_lower.derivatives()[_Xidx]);

    yh2o = yh2or;
    Moose::derivInsert(yh2o.derivatives(), _pidx, yh2o_lower.derivatives()[_pidx]);
    Moose::derivInsert(yh2o.derivatives(), _Tidx, dyh2o_dT);
    Moose::derivInsert(yh2o.derivatives(), _Xidx, yh2o_lower.derivatives()[_Xidx]);
  }
  else
  {
    // CO2 density and derivatives wrt pressure and temperature
    const Real co2_density = _co2_fp.rho_from_p_T(pressure.value(), temperature.value());

    // Equilibrium mole fractions solved using iteration in this regime
    Real xco2r, yh2or;
    solveEquilibriumMoleFractionHighTemp(
        pressure.value(), temperature.value(), Xnacl.value(), co2_density, xco2r, yh2or);

    // Can use these in funcABHighTemp() to get derivatives analytically rather than by iteration
    Real A, dA_dp, dA_dT, B, dB_dp, dB_dT, dB_dX;
    funcABHighTemp(pressure.value(),
                   temperature.value(),
                   Xnacl.value(),
                   co2_density,
                   xco2r,
                   yh2or,
                   A,
                   dA_dp,
                   dA_dT,
                   B,
                   dB_dp,
                   dB_dT,
                   dB_dX);

    const Real dyh2o_dp =
        ((1.0 - B) * dA_dp + (A - 1.0) * A * dB_dp) / (1.0 - A * B) / (1.0 - A * B);
    const Real dxco2_dp = dB_dp * (1.0 - yh2or) - B * dyh2o_dp;

    const Real dyh2o_dT =
        ((1.0 - B) * dA_dT + (A - 1.0) * A * dB_dT) / (1.0 - A * B) / (1.0 - A * B);
    const Real dxco2_dT = dB_dT * (1.0 - yh2or) - B * dyh2o_dT;

    const Real dyh2o_dX = ((A - 1.0) * A * dB_dX) / (1.0 - A * B) / (1.0 - A * B);
    const Real dxco2_dX = dB_dX * (1.0 - yh2or) - B * dyh2o_dX;

    xco2 = xco2r;
    Moose::derivInsert(xco2.derivatives(), _pidx, dxco2_dp);
    Moose::derivInsert(xco2.derivatives(), _Tidx, dxco2_dT);
    Moose::derivInsert(xco2.derivatives(), _Xidx, dxco2_dX);

    yh2o = yh2or;
    Moose::derivInsert(yh2o.derivatives(), _pidx, dyh2o_dp);
    Moose::derivInsert(yh2o.derivatives(), _Tidx, dyh2o_dT);
    Moose::derivInsert(yh2o.derivatives(), _Xidx, dyh2o_dX);
  }
}

void
PorousFlowBrineCO2::equilibriumMoleFractionsLowTemp(const DualReal & pressure,
                                                    const DualReal & temperature,
                                                    const DualReal & Xnacl,
                                                    DualReal & xco2,
                                                    DualReal & yh2o) const
{
  if (temperature.value() > 373.15)
    mooseError(name(),
               ": equilibriumMoleFractionsLowTemp() is not valid for T > 373.15K. Use "
               "equilibriumMoleFractions() instead");

  // CO2 density and derivatives wrt pressure and temperature
  const DualReal co2_density = _co2_fp.rho_from_p_T(pressure, temperature);

  // Assume infinite dilution (yh20 = 0 and xco2 = 0) in low temperature regime
  DualReal A, B;
  funcABLowTemp(pressure, temperature, co2_density, A, B);

  // As the activity coefficient for CO2 in brine used in this regime isn't a 'true'
  // activity coefficient, we instead calculate the molality of CO2 in water, then
  // correct it for brine, and then calculate the mole fractions.
  // The mole fraction in pure water is
  const DualReal yh2ow = (1.0 - B) / (1.0 / A - B);
  const DualReal xco2w = B * (1.0 - yh2ow);

  // Molality of CO2 in pure water
  const DualReal mco2w = xco2w * _invMh2o / (1.0 - xco2w);
  // Molality of CO2 in brine is then calculated using gamma
  const DualReal gamma = activityCoefficient(pressure, temperature, Xnacl);
  const DualReal mco2 = mco2w / gamma;

  // Need NaCl molality (mol/kg)
  const DualReal mnacl = Xnacl / (1.0 - Xnacl) / _Mnacl;

  // Mole fractions of CO2 and H2O in liquid and gas phases
  const DualReal total_moles = 2.0 * mnacl + _invMh2o + mco2;
  xco2 = mco2 / total_moles;
  yh2o = A * (1.0 - xco2 - 2.0 * mnacl / total_moles);
}

void
PorousFlowBrineCO2::funcABLowTemp(const DualReal & pressure,
                                  const DualReal & temperature,
                                  const DualReal & co2_density,
                                  DualReal & A,
                                  DualReal & B) const
{
  if (temperature.value() > 373.15)
    mooseError(name(),
               ": funcABLowTemp() is not valid for T > 373.15K. Use funcABHighTemp() instead");

  // Pressure in bar
  const DualReal pbar = pressure * 1.0e-5;

  // Reference pressure and partial molar volumes
  const Real pref = 1.0;
  const Real vCO2 = 32.6;
  const Real vH2O = 18.1;

  const DualReal delta_pbar = pbar - pref;
  const DualReal Rt = _Rbar * temperature;

  // Equilibrium constants
  const DualReal K0H2O = equilibriumConstantH2O(temperature);
  const DualReal K0CO2 = equilibriumConstantCO2(temperature);

  // Fugacity coefficients
  DualReal phiH2O, phiCO2;
  fugacityCoefficientsLowTemp(pressure, temperature, co2_density, phiCO2, phiH2O);

  A = K0H2O / (phiH2O * pbar) * std::exp(delta_pbar * vH2O / Rt);
  B = phiCO2 * pbar / (_invMh2o * K0CO2) * std::exp(-delta_pbar * vCO2 / Rt);
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
  // Use dummy DualReal temperature as derivatives aren't required
  const DualReal T = temperature;
  Real K0H2O = equilibriumConstantH2O(T).value();
  Real K0CO2 = equilibriumConstantCO2(T).value();

  // Fugacity coefficients
  // Use dummy DualReal variables as derivatives aren't required
  const DualReal p = pressure;
  const DualReal rhoco2 = co2_density;
  const DualReal x = xco2;
  const DualReal y = yh2o;

  DualReal phiH2O, phiCO2;
  fugacityCoefficientsHighTemp(p, T, rhoco2, x, y, phiCO2, phiH2O);

  // Activity coefficients
  const Real gammaH2O = activityCoefficientH2O(T, x).value();
  const Real gammaCO2 = activityCoefficientCO2(T, x).value();

  // Activity coefficient for CO2 in brine
  // Use dummy DualReal Xnacl as derivatives aren't required
  const DualReal X = Xnacl;
  const Real gamma = activityCoefficientHighTemp(T, X).value();

  A = K0H2O * gammaH2O / (phiH2O.value() * pbar) * std::exp(delta_pbar * vH2O / Rt);
  B = phiCO2.value() * pbar / (_invMh2o * K0CO2 * gamma * gammaCO2) *
      std::exp(-delta_pbar * vCO2 / Rt);
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
    auto dfy = [mnacl, this](Real A, Real B, Real dA, Real dB)
    {
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

DualReal
PorousFlowBrineCO2::partialDensityCO2(const DualReal & temperature) const
{
  // This correlation uses temperature in C
  const DualReal Tc = temperature - _T_c2k;
  // The parial molar volume
  const DualReal V = 37.51 - 9.585e-2 * Tc + 8.74e-4 * Tc * Tc - 5.044e-7 * Tc * Tc * Tc;

  return 1.0e6 * _Mco2 / V;
}

Real
PorousFlowBrineCO2::totalMassFraction(
    Real pressure, Real temperature, Real Xnacl, Real saturation, unsigned int qp) const
{
  // Check whether the input pressure and temperature are within the region of validity
  checkVariables(pressure, temperature);

  // As we do not require derivatives, we can simply ignore their initialisation
  const DualReal p = pressure;
  const DualReal T = temperature;
  const DualReal X = Xnacl;

  // FluidStateProperties data structure
  std::vector<FluidStateProperties> fsp(_num_phases, FluidStateProperties(_num_components));
  FluidStateProperties & liquid = fsp[_aqueous_phase_number];
  FluidStateProperties & gas = fsp[_gas_phase_number];

  // Calculate equilibrium mass fractions in the two-phase state
  DualReal Xco2, Yh2o;
  equilibriumMassFractions(p, T, X, Xco2, Yh2o);

  // Save the mass fractions in the FluidStateMassFractions object
  const DualReal Yco2 = 1.0 - Yh2o;
  liquid.mass_fraction[_aqueous_fluid_component] = 1.0 - Xco2;
  liquid.mass_fraction[_gas_fluid_component] = Xco2;
  gas.mass_fraction[_aqueous_fluid_component] = Yh2o;
  gas.mass_fraction[_gas_fluid_component] = Yco2;

  // Gas properties
  gasProperties(pressure, temperature, fsp);

  // Liquid properties
  const DualReal liquid_saturation = 1.0 - saturation;
  const DualReal liquid_pressure = p - _pc.capillaryPressure(liquid_saturation, qp);
  liquidProperties(liquid_pressure, T, X, fsp);

  // The total mass fraction of ncg (z) can now be calculated
  const DualReal Z = (saturation * gas.density * Yco2 + liquid_saturation * liquid.density * Xco2) /
                     (saturation * gas.density + liquid_saturation * liquid.density);

  return Z.value();
}

DualReal
PorousFlowBrineCO2::henryConstant(const DualReal & temperature, const DualReal & Xnacl) const
{
  // Henry's constant for dissolution in water
  const DualReal Kh_h2o = _brine_fp.henryConstant(temperature, _co2_henry);

  // The correction to salt is obtained through the salting out coefficient
  const std::vector<Real> b{1.19784e-1, -7.17823e-4, 4.93854e-6, -1.03826e-8, 1.08233e-11};

  // Need temperature in Celsius
  const DualReal Tc = temperature - _T_c2k;

  DualReal kb = 0.0;
  for (unsigned int i = 0; i < b.size(); ++i)
    kb += b[i] * std::pow(Tc, i);

  // Need salt mass fraction in molality
  const DualReal xmol = Xnacl / (1.0 - Xnacl) / _Mnacl;

  // Henry's constant and its derivative wrt temperature and salt mass fraction
  return Kh_h2o * std::pow(10.0, xmol * kb);
}

DualReal
PorousFlowBrineCO2::enthalpyOfDissolutionGas(const DualReal & temperature,
                                             const DualReal & Xnacl) const
{
  // Henry's constant
  const DualReal Kh = henryConstant(temperature, Xnacl);

  DualReal hdis = -_R * temperature * temperature * Kh.derivatives()[_Tidx] / Kh / _Mco2;

  // Derivative of enthalpy of dissolution wrt temperature and xnacl requires the second
  // derivatives of Henry's constant. For simplicity, approximate these numerically
  const Real dT = temperature.value() * 1.0e-8;
  const DualReal T2 = temperature + dT;
  const DualReal Kh2 = henryConstant(T2, Xnacl);

  const Real dhdis_dT =
      (-_R * T2 * T2 * Kh2.derivatives()[_Tidx] / Kh2 / _Mco2 - hdis).value() / dT;

  const Real dX = Xnacl.value() * 1.0e-8;
  const DualReal X2 = Xnacl + dX;
  const DualReal Kh3 = henryConstant(temperature, X2);

  const Real dhdis_dX =
      (-_R * temperature * temperature * Kh3.derivatives()[_Tidx] / Kh3 / _Mco2 - hdis).value() /
      dX;

  hdis.derivatives() = temperature.derivatives() * dhdis_dT + Xnacl.derivatives() * dhdis_dX;

  return hdis;
}

DualReal
PorousFlowBrineCO2::enthalpyOfDissolution(const DualReal & temperature) const
{
  // Linear fit to model of Duan and Sun (2003) (in kJ/mol)
  const DualReal delta_h = -58.3533 + 0.134519 * temperature;

  // Convert to J/kg
  return delta_h * 1000.0 / _Mco2;
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
