/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowFluidStateBrineCO2.h"
#include "BrineFluidProperties.h"
#include "SinglePhaseFluidPropertiesPT.h"
#include "PorousFlowCapillaryPressure.h"

template <>
InputParameters
validParams<PorousFlowFluidStateBrineCO2>()
{
  InputParameters params = validParams<PorousFlowFluidStateFlashBase>();
  params.addRequiredParam<UserObjectName>("brine_fp", "The name of the user object for brine");
  params.addRequiredParam<UserObjectName>("co2_fp", "The name of the user object for CO2");
  params.addCoupledVar("xnacl", 0, "The salt mass fraction in the brine (kg/kg)");
  params.addClassDescription("Fluid state class for brine and CO2");
  return params;
}

PorousFlowFluidStateBrineCO2::PorousFlowFluidStateBrineCO2(const InputParameters & parameters)
  : PorousFlowFluidStateFlashBase(parameters),

    _xnacl(_nodal_material ? coupledNodalValue("xnacl") : coupledValue("xnacl")),
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
  if (_brine_fp.fluidName() != "brine")
    mooseError("Only a valid Brine FluidProperties UserObject can be provided in brine_fp");

  if (_co2_fp.fluidName() != "co2")
    mooseError("Only a valid CO2 FluidProperties UserObject can be provided in co2_fp");
}

void
PorousFlowFluidStateBrineCO2::thermophysicalProperties()
{
  // The FluidProperty objects use temperature in K
  Real Tk = _temperature[_qp] + _T_c2k;
  Real pressure = _gas_porepressure[_qp];
  Real xnacl = _xnacl[_qp];

  // Mass fraction of CO2 in liquid and H2O in gas phases
  Real X0, dX0_dp, dX0_dT, Y0, Y1, dY1_dp, dY1_dT;
  massFractions(pressure, Tk, xnacl, X0, dX0_dp, dX0_dT, Y1, dY1_dp, dY1_dT);
  Y0 = 1.0 - Y1;

  // Determine which phases are present based on the value of z
  bool is_liquid = false;
  bool is_gas = false;
  bool is_twophase = false;

  if ((*_z[0])[_qp] <= X0)
  {
    // In this case, there is not enough CO2 to form a gas phase,
    // so only a liquid phase is present
    is_liquid = true;
  }
  else if ((*_z[0])[_qp] > X0 && (*_z[0])[_qp] < Y0)
  {
    // Two phases are present
    is_twophase = true;
  }
  else // ((*_z[0])[_qp] >= Y0)
  {
    // In this case, there is not enough brine to form a liquid
    // phase, so only a gas phase is present
    is_gas = true;
  }

  // Material must provide the following properties
  Real vapor_mass_fraction = 0.0;
  Real gas_saturation, liquid_saturation;
  Real gas_density, liquid_density;
  Real gas_viscosity, liquid_viscosity;

  // And the following derivatives
  Real dX0_dz = 0.0, dY0_dp = 0.0, dY0_dT = 0.0, dY0_dz = 0.0;
  Real dgas_density_dp = 0.0, dgas_density_dT = 0.0, dgas_density_dz = 0.0;
  Real dliquid_density_dp = 0.0, dliquid_density_dT = 0.0, dliquid_density_dz = 0.0;
  Real dgas_viscosity_dp = 0.0, dgas_viscosity_dT = 0.0, dgas_viscosity_dz = 0.0;
  Real dliquid_viscosity_dp = 0.0, dliquid_viscosity_dT = 0.0, dliquid_viscosity_dz = 0.0;

  if (is_liquid)
  {
    X0 = (*_z[0])[_qp];
    Y0 = 0.0;
    gas_saturation = 0.0;
    dX0_dp = 0.0;
    dX0_dT = 0.0;
    dX0_dz = 1.0;
    gas_density = 0.0;
    gas_viscosity = 1.0; // To guard against division by 0
  }
  else if (is_twophase)
  {
    // Two phase are present. Set mass equilibrium constants used in the
    // calculation of vapor mass fraction
    Y0 = 1.0 - Y1;
    std::vector<Real> Ki(_num_components);
    Ki[0] = Y0 / X0;
    Ki[1] = (1.0 - Y0) / (1.0 - X0);
    vapor_mass_fraction = vaporMassFraction(Ki);

    // Derivatives of mass fractions wrt PorousFlow variables
    dY0_dp = -dY1_dp;
    dY0_dT = -dY1_dT;
  }
  else // if (is_gas)
  {
    X0 = 0.0;
    dX0_dp = 0.0;
    dX0_dT = 0.0;
    Y0 = (*_z[0])[_qp];
    dY0_dz = 1.0;
    vapor_mass_fraction = 1.0;
    gas_saturation = 1.0;
    liquid_density = 0.0;
    liquid_viscosity = 1.0; // To guard against division by 0
  }

  // Update all remaining mass fractions and derivatives
  // Mass fraction of CO2 in gas and H2O in liquid phases
  Real X1 = 1.0 - X0;
  Y1 = 1.0 - Y0;

  // Calculate the gas density and viscosity in the single phase gas or two
  // phase region
  if (is_gas || is_twophase)
  {
    Real co2_density, dco2_density_dp, dco2_density_dT;
    _co2_fp.rho_dpT(pressure, Tk, co2_density, dco2_density_dp, dco2_density_dT);

    // Gas density is given by the CO2 density - no correction due to the small amount of
    // brine vapor is made
    gas_density = co2_density;
    dgas_density_dp = dco2_density_dp;
    dgas_density_dT = dco2_density_dT;
    dgas_density_dz = 0.0;

    Real co2_viscosity, dco2_viscosity_drho, dco2_viscosity_dT;
    _co2_fp.mu_drhoT_from_rho_T(
        co2_density, Tk, dco2_density_dT, co2_viscosity, dco2_viscosity_drho, dco2_viscosity_dT);

    // Assume that the viscosity of the gas phase is a weighted sum of the
    // individual viscosities
    gas_viscosity = co2_viscosity;
    dgas_viscosity_dp = dco2_viscosity_drho * dco2_density_dp;
    dgas_viscosity_dT = dco2_viscosity_dT;
    dgas_viscosity_dz = 0.0;
  }

  // Calculate the saturation in the two phase case using the vapor mass
  // fraction
  if (is_twophase)
  {
    // Liquid density
    Real co2_partial_density, dco2_partial_density_dT;
    partialDensityCO2(Tk, co2_partial_density, dco2_partial_density_dT);
    // Use old value of gas saturation to estimate liquid saturation
    Real liqsat = 1.0;
    if (!_is_initqp)
      liqsat -= _saturation_old[_qp][_gas_phase_number];

    liquid_density =
        1.0 / (X0 / co2_partial_density +
               X1 / _brine_fp.rho(pressure - _pc_uo.capillaryPressure(liqsat), Tk, xnacl));

    // The gas saturation in the two phase case
    gas_saturation = vapor_mass_fraction * liquid_density /
                     (gas_density + vapor_mass_fraction * (liquid_density - gas_density));
  }

  // Calculate the saturations and pressures for each phase
  liquid_saturation = 1.0 - gas_saturation;
  Real liquid_porepressure = pressure - _pc_uo.capillaryPressure(liquid_saturation);

  // Calculate liquid density and viscosity if in the two phase or single phase
  // liquid region, including a density correction due to the presence of dissolved
  // CO2
  if (is_liquid || is_twophase)
  {
    Real brine_density, dbrine_density_dp, dbrine_density_dT, dbrine_density_dx;
    Real dliquid_viscosity_drho, dliquid_viscosity_dx;

    _brine_fp.rho_dpTx(liquid_porepressure,
                       Tk,
                       xnacl,
                       brine_density,
                       dbrine_density_dp,
                       dbrine_density_dT,
                       dbrine_density_dx);

    // The liquid density
    Real co2_partial_density, dco2_partial_density_dT;
    partialDensityCO2(Tk, co2_partial_density, dco2_partial_density_dT);
    liquid_density = 1.0 / (X0 / co2_partial_density + X1 / brine_density);

    dliquid_density_dp =
        (dX0_dp / brine_density + X1 * dbrine_density_dp / brine_density / brine_density -
         dX0_dp / co2_partial_density) *
        liquid_density * liquid_density;

    dliquid_density_dT =
        (dX0_dT / brine_density + X1 * dbrine_density_dT / brine_density / brine_density -
         dX0_dT / co2_partial_density +
         X0 * dco2_partial_density_dT / co2_partial_density / co2_partial_density) *
        liquid_density * liquid_density;

    dliquid_density_dz =
        (dX0_dz / brine_density - dX0_dz / co2_partial_density) * liquid_density * liquid_density;

    // Liquid viscosity is just the brine viscosity
    // Note: brine viscosity (and derivatives) requires water density (and derivatives)
    Real water_density, dwater_density_dp, dwater_density_dT;
    _water_fp.rho_dpT(liquid_porepressure, Tk, water_density, dwater_density_dp, dwater_density_dT);

    _brine_fp.mu_drhoTx(water_density,
                        Tk,
                        xnacl,
                        dwater_density_dT,
                        liquid_viscosity,
                        dliquid_viscosity_drho,
                        dliquid_viscosity_dT,
                        dliquid_viscosity_dx);

    // The derivative of viscosity wrt pressure is given by the chain rule
    dliquid_viscosity_dp = dliquid_viscosity_drho * dwater_density_dp;
  }

  // Save properties in FluidStateProperties vector
  auto & liquid = _fsp[_aqueous_phase_number];
  auto & gas = _fsp[_gas_phase_number];

  liquid.saturation = liquid_saturation;
  gas.saturation = gas_saturation;
  liquid.pressure = liquid_porepressure;
  gas.pressure = _gas_porepressure[_qp];

  liquid.mass_fraction[_aqueous_fluid_component] = 1.0 - X0;
  liquid.mass_fraction[_gas_fluid_component] = X0;
  gas.mass_fraction[_aqueous_fluid_component] = 1.0 - Y0;
  gas.mass_fraction[_gas_fluid_component] = Y0;

  liquid.fluid_density = liquid_density;
  gas.fluid_density = gas_density;
  liquid.fluid_viscosity = liquid_viscosity;
  gas.fluid_viscosity = gas_viscosity;

  // Derivatives wrt PorousFlow variables are required by the kernels
  // Note: these don't need to be stateful so don't calculate them in
  // initQpStatefulProperties
  if (!_is_initqp)
  {
    // Derivative of gas saturation wrt variables
    Real ds_dp = 0.0, ds_dT = 0.0, ds_dz = 0.0;
    if (is_twophase)
    {
      Real K0 = Y0 / X0;
      Real K1 = (1.0 - Y0) / (1.0 - X0);
      Real dv_dz = (K1 - K0) / ((K0 - 1.0) * (K1 - 1.0));

      ds_dz = gas_density * liquid_density * dv_dz;
      ds_dz /= (gas_density + vapor_mass_fraction * (liquid_density - gas_density)) *
               (gas_density + vapor_mass_fraction * (liquid_density - gas_density));

      Real dK0_dp = (-Y0 * dX0_dp - X0 * dY1_dp) / X0 / X0;
      Real dK0_dT = (-Y0 * dX0_dT - X0 * dY1_dT) / X0 / X0;

      Real dK1_dp = (Y1 * dX0_dp + (1.0 - X0) * dY1_dp) / (1.0 - X0) / (1.0 - X0);
      Real dK1_dT = (Y1 * dX0_dT + (1.0 - X0) * dY1_dT) / (1.0 - X0) / (1.0 - X0);

      Real dv_dp = (*_z[0])[_qp] * dK1_dp / (K1 - 1.0) / (K1 - 1.0) +
                   (1.0 - (*_z[0])[_qp]) * dK0_dp / (K0 - 1.0) / (K0 - 1.0);

      ds_dp = gas_density * liquid_density * dv_dp +
              vapor_mass_fraction * (1.0 - vapor_mass_fraction) *
                  (gas_density * dliquid_density_dp - dgas_density_dp * liquid_density);
      ds_dp /= (gas_density + vapor_mass_fraction * (liquid_density - gas_density)) *
               (gas_density + vapor_mass_fraction * (liquid_density - gas_density));

      Real dv_dT = (*_z[0])[_qp] * dK1_dT / (K1 - 1.0) / (K1 - 1.0) +
                   (1.0 - (*_z[0])[_qp]) * dK0_dT / (K0 - 1.0) / (K0 - 1.0);

      ds_dT = gas_density * liquid_density * dv_dT +
              vapor_mass_fraction * (1.0 - vapor_mass_fraction) *
                  (gas_density * dliquid_density_dT - dgas_density_dT * liquid_density);
      ds_dT /= (gas_density + vapor_mass_fraction * (liquid_density - gas_density)) *
               (gas_density + vapor_mass_fraction * (liquid_density - gas_density));
    }

    // Save derivatives in FluidStateProperties vector
    liquid.dsaturation_dp = -ds_dp;
    liquid.dsaturation_dT = -ds_dT;
    liquid.dsaturation_dz = -ds_dz;
    gas.dsaturation_dp = ds_dp;
    gas.dsaturation_dT = ds_dT;
    gas.dsaturation_dz = ds_dz;

    liquid.dmass_fraction_dp[_aqueous_fluid_component] = -dX0_dp;
    liquid.dmass_fraction_dp[_gas_fluid_component] = dX0_dp;
    liquid.dmass_fraction_dT[_aqueous_fluid_component] = -dX0_dT;
    liquid.dmass_fraction_dT[_gas_fluid_component] = dX0_dT;
    liquid.dmass_fraction_dz[_aqueous_fluid_component] = -dX0_dz;
    liquid.dmass_fraction_dz[_gas_fluid_component] = dX0_dz;
    gas.dmass_fraction_dp[_aqueous_fluid_component] = -dY0_dp;
    gas.dmass_fraction_dp[_gas_fluid_component] = dY0_dp;
    gas.dmass_fraction_dT[_aqueous_fluid_component] = -dY0_dT;
    gas.dmass_fraction_dT[_gas_fluid_component] = dY0_dT;
    gas.dmass_fraction_dz[_aqueous_fluid_component] = -dY0_dz;
    gas.dmass_fraction_dz[_gas_fluid_component] = dY0_dz;

    liquid.dfluid_density_dp = dliquid_density_dp;
    liquid.dfluid_density_dT = dliquid_density_dT;
    liquid.dfluid_density_dz = dliquid_density_dz;
    gas.dfluid_density_dp = dgas_density_dp;
    gas.dfluid_density_dT = dgas_density_dT;
    gas.dfluid_density_dz = dgas_density_dz;

    liquid.dfluid_viscosity_dp = dliquid_viscosity_dp;
    liquid.dfluid_viscosity_dT = dliquid_viscosity_dT;
    liquid.dfluid_viscosity_dz = dliquid_viscosity_dz;
    gas.dfluid_viscosity_dp = dgas_viscosity_dp;
    gas.dfluid_viscosity_dT = dgas_viscosity_dT;
    gas.dfluid_viscosity_dz = dgas_viscosity_dz;
  }
}

void
PorousFlowFluidStateBrineCO2::massFractions(Real pressure,
                                            Real temperature,
                                            Real xnacl,
                                            Real & xco2l,
                                            Real & dxco2l_dp,
                                            Real & dxco2l_dT,
                                            Real & xh2og,
                                            Real & dxh2og_dp,
                                            Real & dxh2og_dT) const
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
  xh2og = yH2Ob * _Mh2o / (yH2Ob * _Mh2o + (1.0 - yH2Ob) * _Mco2);

  // The number of moles of CO2 in 1kg of H2O
  Real nco2 = xCO2b * (2.0 * mnacl + _invMh2o) / (1.0 - xCO2b);

  // The mass fraction of CO2 in liquid
  xco2l = nco2 * _Mco2 / (1.0 + mnacl * _Mnacl + nco2 * _Mco2);

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

  dxh2og_dp = _Mco2 * _Mh2o * dyH2Ob_dp / (yH2Ob * _Mh2o + (1.0 - yH2Ob) * _Mco2) /
              (yH2Ob * _Mh2o + (1.0 - yH2Ob) * _Mco2);

  Real dnco2_dp = dxCO2b_dp * (2.0 * mnacl + _invMh2o) / (1.0 - xCO2b) / (1.0 - xCO2b);

  dxco2l_dp = (1.0 + mnacl * _Mnacl) * _Mco2 * dnco2_dp / (1.0 + mnacl * _Mnacl + nco2 * _Mco2) /
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

  dxh2og_dT = _Mco2 * _Mh2o * dyH2Ob_dT / (yH2Ob * _Mh2o + (1.0 - yH2Ob) * _Mco2) /
              (yH2Ob * _Mh2o + (1.0 - yH2Ob) * _Mco2);

  Real dnco2_dT = dxCO2b_dT * (2.0 * mnacl + _invMh2o) / (1.0 - xCO2b) / (1.0 - xCO2b);

  dxco2l_dT = (1.0 + mnacl * _Mnacl) * _Mco2 * dnco2_dT / (1.0 + mnacl * _Mnacl + nco2 * _Mco2) /
              (1.0 + mnacl * _Mnacl + nco2 * _Mco2);
}

void
PorousFlowFluidStateBrineCO2::fugacityCoefficientCO2(
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
PorousFlowFluidStateBrineCO2::fugacityCoefficientH2O(
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
PorousFlowFluidStateBrineCO2::activityCoefficient(Real pressure,
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
PorousFlowFluidStateBrineCO2::equilibriumConstantH2O(Real temperature,
                                                     Real & kh2o,
                                                     Real & dkh2o_dT) const
{
  // Uses temperature in Celcius
  Real Tc = temperature - _T_c2k;

  Real logK0H2O = -2.209 + 3.097e-2 * Tc - 1.098e-4 * Tc * Tc + 2.048e-7 * Tc * Tc * Tc;
  Real dlogK0H2O = 3.097e-2 - 2.196e-4 * Tc + 6.144e-7 * Tc * Tc;

  kh2o = std::pow(10.0, logK0H2O);
  dkh2o_dT = std::log(10.0) * dlogK0H2O * kh2o;
}

void
PorousFlowFluidStateBrineCO2::equilibriumConstantCO2(Real temperature,
                                                     Real & kco2,
                                                     Real & dkco2_dT) const
{
  // Uses temperature in Celcius
  Real Tc = temperature - _T_c2k;

  Real logK0CO2 = 1.189 + 1.304e-2 * Tc - 5.446e-5 * Tc * Tc;
  Real dlogK0CO2 = 1.304e-2 - 1.0892e-4 * Tc;

  kco2 = std::pow(10.0, logK0CO2);
  dkco2_dT = std::log(10.0) * dlogK0CO2 * kco2;
}

void
PorousFlowFluidStateBrineCO2::partialDensityCO2(Real temperature,
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
