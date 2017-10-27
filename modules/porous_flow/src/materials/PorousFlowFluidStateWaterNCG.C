/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowFluidStateWaterNCG.h"
#include "PorousFlowCapillaryPressure.h"

template <>
InputParameters
validParams<PorousFlowFluidStateWaterNCG>()
{
  InputParameters params = validParams<PorousFlowFluidStateFlashBase>();
  params.addRequiredParam<UserObjectName>("water_fp", "The name of the user object for water");
  params.addRequiredParam<UserObjectName>(
      "gas_fp", "The name of the user object for the non-condensable gas");
  params.addClassDescription("Fluid state class for water and non-condensable gas");
  return params;
}

PorousFlowFluidStateWaterNCG::PorousFlowFluidStateWaterNCG(const InputParameters & parameters)
  : PorousFlowFluidStateFlashBase(parameters),

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

void
PorousFlowFluidStateWaterNCG::thermophysicalProperties()
{
  // The FluidProperty objects use temperature in K
  Real Tk = _temperature[_qp] + _T_c2k;

  // Check whether the input temperature is within the region of validity of this equation
  // of state (T_triple <= T <= T_critical)
  if (Tk < _water_triple_temperature || Tk > _water_critical_temperature)
    mooseError("PorousFlowFluidStateWaterNCG: Temperature is outside range 273.16 K <= T "
               "<= 647.096 K");

  // Equilibrium constants for each component (Henry's law for the NCG
  // component, and Raoult's law for water).
  // Note: these are in terms of mole fraction
  Real psat, dpsat_dT;
  _water_fp.vaporPressure_dT(Tk, psat, dpsat_dT);
  Real K0 = _ncg_fp.henryConstant(Tk) / _gas_porepressure[_qp];
  Real K1 = psat / _gas_porepressure[_qp];

  // The mole fractions for the NCG component in the two component
  // case can be expressed in terms of the equilibrium constants only
  Real x0 = (1.0 - K1) / (K0 - K1);
  Real y0 = K0 * x0;

  // Convert mole fractions to mass fractions
  Real X0 = moleFractionToMassFraction(x0);
  Real Y0 = moleFractionToMassFraction(y0);

  // Determine which phases are present based on the value of z
  bool is_liquid = false;
  bool is_gas = false;
  bool is_twophase = false;

  if ((*_z[0])[_qp] <= X0)
  {
    // In this case, there is not enough NCG to form a gas phase,
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
    // In this case, there is not enough water to form a liquid
    // phase, so only a gas phase is present
    is_gas = true;
  }

  // Material must provide the following properties
  Real vapor_mass_fraction = 0.0;
  Real gas_saturation, liquid_saturation;
  Real gas_density, liquid_density;
  Real gas_viscosity, liquid_viscosity;

  // And the following derivatives
  Real dX0_dp = 0.0, dX0_dT = 0.0, dX0_dz = 0.0;
  Real dY0_dp = 0.0, dY0_dT = 0.0, dY0_dz = 0.0;
  Real dgas_density_dp = 0.0, dgas_density_dT = 0.0, dgas_density_dz = 0.0;
  Real dliquid_density_dp = 0.0, dliquid_density_dT = 0.0, dliquid_density_dz = 0.0;
  Real dgas_viscosity_dp = 0.0, dgas_viscosity_dT = 0.0, dgas_viscosity_dz = 0.0;
  Real dliquid_viscosity_dp = 0.0, dliquid_viscosity_dT = 0.0, dliquid_viscosity_dz = 0.0;

  if (is_liquid)
  {
    X0 = (*_z[0])[_qp];
    Y0 = 0.0;
    gas_saturation = 0.0;
    dX0_dz = 1.0;
    gas_density = 0.0;
    gas_viscosity = 1.0; // To guard against division by 0
  }
  else if (is_twophase)
  {
    // Two phase are present. Set mass equilibrium constants used in the
    // calculation of vapor mass fraction
    std::vector<Real> Ki(_num_components);
    Ki[0] = Y0 / X0;
    Ki[1] = (1.0 - Y0) / (1.0 - X0);
    vapor_mass_fraction = vaporMassFraction(Ki);

    // Derivatives of mass fractions wrt PorousFlow variables
    Real Kh, dKh_dT;
    _ncg_fp.henryConstant_dT(Tk, Kh, dKh_dT);
    Real dK0_dp = -Kh / _gas_porepressure[_qp] / _gas_porepressure[_qp];
    Real dK0_dT = dKh_dT / _gas_porepressure[_qp];

    Real dK1_dp = -psat / _gas_porepressure[_qp] / _gas_porepressure[_qp];
    Real dK1_dT = dpsat_dT / _gas_porepressure[_qp];

    Real dx0_dp = ((K1 - 1.0) * dK0_dp + (1 - K0) * dK1_dp) / (K0 - K1) / (K0 - K1);
    Real dy0_dp = x0 * dK0_dp + K0 * dx0_dp;
    Real dx0_dT = ((K1 - 1.0) * dK0_dT + (1 - K0) * dK1_dT) / (K0 - K1) / (K0 - K1);
    Real dy0_dT = x0 * dK0_dT + K0 * dx0_dT;

    Real dX0_dx0 =
        _Mncg * _Mh2o / (x0 * _Mncg + (1.0 - x0) * _Mh2o) / (x0 * _Mncg + (1.0 - x0) * _Mh2o);
    Real dY0_dy0 =
        _Mncg * _Mh2o / (y0 * _Mncg + (1.0 - y0) * _Mh2o) / (y0 * _Mncg + (1.0 - y0) * _Mh2o);

    dX0_dp = dX0_dx0 * dx0_dp;
    dX0_dT = dX0_dx0 * dx0_dT;
    dY0_dp = dY0_dy0 * dy0_dp;
    dY0_dT = dY0_dy0 * dy0_dT;
  }
  else // if (is_gas)
  {
    X0 = 0.0;
    Y0 = (*_z[0])[_qp];
    vapor_mass_fraction = 1.0;
    gas_saturation = 1.0;
    dY0_dz = 1.0;
    liquid_density = 0.0;
    liquid_viscosity = 1.0; // To guard against division by 0
  }

  // Calculate the gas density and viscosity in the single phase gas or two
  // phase region
  if (is_gas || is_twophase)
  {
    Real ncg_density, dncg_density_dp, dncg_density_dT;
    Real vapor_density, dvapor_density_dp, dvapor_density_dT;
    // NCG density calculated using partial pressure Y0 * gas_poreressure (Dalton's law)
    _ncg_fp.rho_dpT(Y0 * _gas_porepressure[_qp], Tk, ncg_density, dncg_density_dp, dncg_density_dT);
    // Vapor density calculated using partial pressure X1 * psat (Raoult's law)
    _water_fp.rho_dpT((1.0 - X0) * psat, Tk, vapor_density, dvapor_density_dp, dvapor_density_dT);

    // The derivatives wrt pressure above must be multiplied by the derivative of the pressure
    // variable using the chain rule
    gas_density = ncg_density + vapor_density;
    dgas_density_dp = (Y0 + dY0_dp * _gas_porepressure[_qp]) * dncg_density_dp -
                      dX0_dp * psat * dvapor_density_dp;
    dgas_density_dT = dncg_density_dT + dvapor_density_dT;

    Real ncg_viscosity, dncg_viscosity_drho, dncg_viscosity_dT;
    Real vapor_viscosity, dvapor_viscosity_drho, dvapor_viscosity_dT;
    _ncg_fp.mu_drhoT_from_rho_T(
        ncg_density, Tk, dncg_density_dT, ncg_viscosity, dncg_viscosity_drho, dncg_viscosity_dT);
    _water_fp.mu_drhoT_from_rho_T(vapor_density,
                                  Tk,
                                  dvapor_density_dT,
                                  vapor_viscosity,
                                  dvapor_viscosity_drho,
                                  dvapor_viscosity_dT);

    // Assume that the viscosity of the gas phase is a weighted sum of the
    // individual viscosities
    gas_viscosity = Y0 * ncg_viscosity + (1.0 - Y0) * vapor_viscosity;
    dgas_viscosity_dp =
        dY0_dp * (ncg_viscosity - vapor_viscosity) +
        Y0 * (Y0 + dY0_dp * _gas_porepressure[_qp]) * dncg_viscosity_drho * dncg_density_dp -
        dX0_dp * psat * (1.0 - Y0) * dvapor_viscosity_drho * dvapor_density_dp;
    dgas_viscosity_dT = dY0_dT * (ncg_viscosity - vapor_viscosity) + Y0 * dncg_viscosity_dT +
                        (1.0 - Y0) * dvapor_density_dT;

    // Also calculate derivatives wrt z in the gas phase (these are 0 in the two phase region)
    if (is_gas)
    {
      dgas_density_dz =
          dY0_dz * _gas_porepressure[_qp] * dncg_density_dp - dX0_dz * psat * dvapor_density_dp;

      dgas_viscosity_dz =
          dY0_dz * (ncg_viscosity - vapor_viscosity) +
          Y0 * dncg_viscosity_drho * dncg_density_dp * dY0_dz * _gas_porepressure[_qp] -
          dX0_dz * psat * (1.0 - Y0) * dvapor_viscosity_drho * dvapor_density_dp;
    }
  }

  // Calculate the saturation in the two phase case using the vapor mass fraction
  if (is_twophase)
  {
    // Liquid density is approximated by the water density.
    // Use old value of gas saturation to estimate liquid saturation
    Real liqsat = 1.0;
    if (!_is_initqp)
      liqsat -= _saturation_old[_qp][_gas_phase_number];
    liquid_density = _water_fp.rho(_gas_porepressure[_qp] + _pc_uo.capillaryPressure(liqsat), Tk);

    // The gas saturation in the two phase case
    gas_saturation = vapor_mass_fraction * liquid_density /
                     (gas_density + vapor_mass_fraction * (liquid_density - gas_density));
  }

  // Calculate the saturations and pressures for each phase
  liquid_saturation = 1.0 - gas_saturation;
  Real liquid_porepressure = _gas_porepressure[_qp] - _pc_uo.capillaryPressure(liquid_saturation);

  // Calculate liquid density and viscosity if in the two phase or single phase
  // liquid region, assuming they are not affected by the presence of dissolved
  // NCG. Note: the (small) contribution due to derivative of capillary pressure
  //  wrt pressure (using the chain rule) is not implemented.
  if (is_liquid || is_twophase)
  {
    Real dliquid_viscosity_drho;
    _water_fp.rho_dpT(
        liquid_porepressure, Tk, liquid_density, dliquid_density_dp, dliquid_density_dT);
    _water_fp.mu_drhoT_from_rho_T(liquid_density,
                                  Tk,
                                  dliquid_density_dT,
                                  liquid_viscosity,
                                  dliquid_viscosity_drho,
                                  dliquid_viscosity_dT);

    // The derivative of viscosity wrt pressure is given by the chain rule
    dliquid_viscosity_dp = dliquid_viscosity_drho * dliquid_density_dp;
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
      K0 = Y0 / X0;
      K1 = (1.0 - Y0) / (1.0 - X0);
      Real dv_dz = (K1 - K0) / ((K0 - 1.0) * (K1 - 1.0));
      ds_dz = gas_density * liquid_density * dv_dz +
              vapor_mass_fraction * (1.0 - vapor_mass_fraction) *
                  (gas_density * dliquid_density_dz - dgas_density_dz * liquid_density);
      ds_dz /= (gas_density + vapor_mass_fraction * (liquid_density - gas_density)) *
               (gas_density + vapor_mass_fraction * (liquid_density - gas_density));

      Real Kh, dKh_dT;
      _ncg_fp.henryConstant_dT(Tk, Kh, dKh_dT);
      Real dK0_dp = -Kh / _gas_porepressure[_qp] / _gas_porepressure[_qp];
      Real dK0_dT = dKh_dT / _gas_porepressure[_qp];

      Real dK1_dp = -psat / _gas_porepressure[_qp] / _gas_porepressure[_qp];
      Real dK1_dT = dpsat_dT / _gas_porepressure[_qp];

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

Real
PorousFlowFluidStateWaterNCG::enthalpyOfDissolution(Real temperature, Real Kh, Real dKh_dT) const
{
  return -_R * temperature * temperature * _Mncg * dKh_dT / Kh;
}

Real
PorousFlowFluidStateWaterNCG::moleFractionToMassFraction(Real xmol) const
{
  return xmol * _Mncg / (xmol * _Mncg + (1.0 - xmol) * _Mh2o);
}
