/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowFluidStateWaterNCG.h"

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
    _Mncg(_ncg_fp.molarMass())
{
}

void
PorousFlowFluidStateWaterNCG::thermophysicalProperties() const
{
  // The FluidProperty objects use temperature in K
  Real Tk = _temperature[_qp] + _T_c2k;

  // Equilibrium constants for each component (Henry's law for the NCG
  // component, and Raoult's law for water).
  // Note: these are in terms of mole fraction
  Real K0 = _ncg_fp.henryConstant(Tk) / _gas_porepressure[_qp];
  Real K1 = _water_fp.pSat(Tk) / _gas_porepressure[_qp];

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

    Real psat, dpsat_dT;
    _water_fp.pSat_dT(Tk, psat, dpsat_dT);
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
    _ncg_fp.rho_dpT(Y0 * _gas_porepressure[_qp], Tk, ncg_density, dncg_density_dp, dncg_density_dT);
    _water_fp.rho_dpT((1.0 - Y0) * _gas_porepressure[_qp],
                      Tk,
                      vapor_density,
                      dvapor_density_dp,
                      dvapor_density_dT);

    gas_density = ncg_density + vapor_density;
    dgas_density_dp = (dncg_density_dp) * (Y0 + dY0_dp * _gas_porepressure[_qp]) +
                      dvapor_density_dp * (1.0 - Y0 - dY0_dp * _gas_porepressure[_qp]);
    dgas_density_dT = dncg_density_dT + dvapor_density_dT;
    dgas_density_dz = _gas_porepressure[_qp] * dY0_dz * (dncg_density_dp - dvapor_density_dp);

    Real ncg_viscosity, dncg_viscosity_drho, dncg_viscosity_dT;
    Real vapor_viscosity, dvapor_viscosity_drho, dvapor_viscosity_dT;
    _ncg_fp.mu_drhoT(ncg_density, Tk, ncg_viscosity, dncg_viscosity_drho, dncg_viscosity_dT);
    _water_fp.mu_drhoT(
        vapor_density, Tk, vapor_viscosity, dvapor_viscosity_drho, dvapor_viscosity_dT);

    // Assume that the viscosity of the gas phase is a weighted sum of the
    // individual viscosities
    gas_viscosity = Y0 * ncg_viscosity + (1.0 - Y0) * vapor_viscosity;
    dgas_viscosity_dp = dY0_dp * (ncg_viscosity - vapor_viscosity) +
                        Y0 * dncg_viscosity_drho * dncg_density_dp +
                        (1.0 - Y0) * dvapor_viscosity_drho * dvapor_density_dp;
    dgas_viscosity_dT = dY0_dT * (ncg_viscosity - vapor_viscosity) + Y0 * dncg_viscosity_dT +
                        (1.0 - Y0) * dvapor_density_dT;
    dgas_viscosity_dz =
        dY0_dz * (ncg_viscosity - vapor_viscosity) +
        Y0 * dncg_viscosity_drho * dncg_density_dp * dY0_dz * _gas_porepressure[_qp] -
        (1.0 - Y0) * dvapor_viscosity_drho * dvapor_density_dp * dY0_dz * _gas_porepressure[_qp];
  }

  // Calculate the saturation in the two phase case using the vapor mass
  // fraction
  if (is_twophase)
  {
    // Liquid density is approximated by the water density. Note: calculated
    // using gas pressure as liquid saturation is not known yet
    liquid_density = _water_fp.rho(_gas_porepressure[_qp], Tk);

    // The gas saturation in the two phase case
    gas_saturation = vapor_mass_fraction * liquid_density /
                     (gas_density + vapor_mass_fraction * (liquid_density - gas_density));
  }

  // Set the saturations and pressures for each phase
  liquid_saturation = 1.0 - gas_saturation;
  Real seff = effectiveSaturation(liquid_saturation);
  Real liquid_porepressure = _gas_porepressure[_qp] + capillaryPressure(seff);

  _saturation[_qp][_aqueous_phase_number] = liquid_saturation;
  _saturation[_qp][_gas_phase_number] = gas_saturation;
  _porepressure[_qp][_aqueous_phase_number] = liquid_porepressure;
  _porepressure[_qp][_gas_phase_number] = _gas_porepressure[_qp];

  // Set the mass fractions
  _mass_frac[_qp][_aqueous_phase_number][_aqueous_fluid_component] = 1.0 - X0;
  _mass_frac[_qp][_aqueous_phase_number][_gas_fluid_component] = X0;
  _mass_frac[_qp][_gas_phase_number][_aqueous_fluid_component] = 1.0 - Y0;
  _mass_frac[_qp][_gas_phase_number][_gas_fluid_component] = Y0;

  // Calculate liquid density and viscosity if in the two phase or single phase
  // liquid region, assuming they are not affected by the presence of dissolved
  // NCG
  if (is_liquid || is_twophase)
  {
    Real dliquid_viscosity_drho;
    _water_fp.rho_dpT(
        liquid_porepressure, Tk, liquid_density, dliquid_density_dp, dliquid_density_dT);
    _water_fp.mu_drhoT(
        liquid_density, Tk, liquid_viscosity, dliquid_viscosity_drho, dliquid_viscosity_dT);

    // The derivative of viscosity wrt pressure is given by the chain rule
    dliquid_viscosity_dp = dliquid_viscosity_drho * dliquid_density_dp;
  }

  // Set the phase densities and viscosities
  _fluid_density[_qp][_aqueous_phase_number] = liquid_density;
  _fluid_density[_qp][_gas_phase_number] = gas_density;

  _fluid_viscosity[_qp][_aqueous_phase_number] = liquid_viscosity;
  _fluid_viscosity[_qp][_gas_phase_number] = gas_viscosity;

  // Derivatives wrt PorousFlow variables are required by the kernels
  // Note: these don't need to be stateful so don't calculate them in
  // initQpStatefulProperties
  if (!_is_initqp)
  {
    // Calculate derivatives of material properties wrt primary variables
    // Derivative of z wrt variables
    std::vector<Real> dz_dvar;
    dz_dvar.assign(_num_pf_vars, 0.0);
    if (_dictator.isPorousFlowVariable(_z_varnum[0]))
      dz_dvar[_zvar[0]] = 1.0;

    // Derivative of saturation wrt variables
    if (is_twophase)
    {
      K0 = Y0 / X0;
      K1 = (1.0 - Y0) / (1.0 - X0);
      Real ds_dv = gas_density * liquid_density /
                   (gas_density + vapor_mass_fraction * (liquid_density - gas_density)) /
                   (gas_density + vapor_mass_fraction * (liquid_density - gas_density));
      Real dv_dz = (K1 - K0) / ((K0 - 1.0) * (K1 - 1.0));
      _dsaturation_dvar[_qp][_gas_phase_number][_zvar[0]] = ds_dv * dv_dz;
      _dsaturation_dvar[_qp][_aqueous_phase_number][_zvar[0]] = -ds_dv * dv_dz;

      Real Kh, dKh_dT;
      _ncg_fp.henryConstant_dT(Tk, Kh, dKh_dT);
      Real dK0_dp = -Kh / _gas_porepressure[_qp] / _gas_porepressure[_qp];
      Real dK0_dT = dKh_dT / _gas_porepressure[_qp];

      Real psat, dpsat_dT;
      _water_fp.pSat_dT(Tk, psat, dpsat_dT);
      Real dK1_dp = -psat / _gas_porepressure[_qp] / _gas_porepressure[_qp];
      Real dK1_dT = dpsat_dT / _gas_porepressure[_qp];

      Real dv_dp = (*_z[0])[_qp] * dK1_dp / (K1 - 1.0) / (K1 - 1.0) +
                   (1.0 - (*_z[0])[_qp]) * dK0_dp / (K0 - 1.0) / (K0 - 1.0);
      Real ds_dp = gas_density * liquid_density * dv_dp +
                   vapor_mass_fraction * (1.0 - vapor_mass_fraction) *
                       (gas_density * dliquid_density_dp - dgas_density_dp * liquid_density);

      ds_dp /= (gas_density + vapor_mass_fraction * (liquid_density - gas_density)) *
               (gas_density + vapor_mass_fraction * (liquid_density - gas_density));

      Real dv_dT = (*_z[0])[_qp] * dK1_dT / (K1 - 1.0) / (K1 - 1.0) +
                   (1.0 - (*_z[0])[_qp]) * dK0_dT / (K0 - 1.0) / (K0 - 1.0);
      Real ds_dT = gas_density * liquid_density * dv_dT +
                   vapor_mass_fraction * (1.0 - vapor_mass_fraction) *
                       (gas_density * dliquid_density_dT - dgas_density_dT * liquid_density);

      ds_dT /= (gas_density + vapor_mass_fraction * (liquid_density - gas_density)) *
               (gas_density + vapor_mass_fraction * (liquid_density - gas_density));

      _dsaturation_dvar[_qp][_gas_phase_number][_pvar] = ds_dp;
      _dsaturation_dvar[_qp][_aqueous_phase_number][_pvar] = -ds_dp;
    }

    // Derivative of porepressure wrt variables
    if (_dictator.isPorousFlowVariable(_gas_porepressure_varnum))
    {
      for (unsigned int ph = 0; ph < _num_phases; ++ph)
      {
        _dporepressure_dvar[_qp][ph][_pvar] = 1.0;
        if (!_nodal_material)
          (*_dgradp_qp_dgradv)[_qp][ph][_pvar] = 1.0;
      }

      // The aqueous phase porepressure is a function of liquid saturation,
      // which depends on both gas porepressure and z
      if (is_twophase)
      {
        Real dpc = dCapillaryPressure_dS(seff) * _dseff_ds;
        _dporepressure_dvar[_qp][_aqueous_phase_number][_pvar] +=
            dpc * _dsaturation_dvar[_qp][_aqueous_phase_number][_pvar];
        _dporepressure_dvar[_qp][_aqueous_phase_number][_zvar[0]] =
            dpc * _dsaturation_dvar[_qp][_aqueous_phase_number][_zvar[0]];
      }
    }

    // Derivatives of properties wrt primary variables
    for (unsigned int v = 0; v < _num_pf_vars; ++v)
    {
      // Derivative of density in each phase
      _dfluid_density_dvar[_qp][_aqueous_phase_number][v] +=
          dliquid_density_dp * _dporepressure_dvar[_qp][_aqueous_phase_number][v];
      _dfluid_density_dvar[_qp][_gas_phase_number][v] +=
          dgas_density_dp * _dporepressure_dvar[_qp][_gas_phase_number][v];
      _dfluid_density_dvar[_qp][_aqueous_phase_number][v] +=
          dliquid_density_dT * _dtemperature_dvar[_qp][v];
      _dfluid_density_dvar[_qp][_gas_phase_number][v] +=
          dgas_density_dT * _dtemperature_dvar[_qp][v];
      _dfluid_density_dvar[_qp][_aqueous_phase_number][v] += dliquid_density_dz * dz_dvar[v];
      _dfluid_density_dvar[_qp][_gas_phase_number][v] += dgas_density_dz * dz_dvar[v];

      // Derivative of viscosity in each phase
      _dfluid_viscosity_dvar[_qp][_aqueous_phase_number][v] +=
          dliquid_viscosity_dp * _dporepressure_dvar[_qp][_aqueous_phase_number][v];
      _dfluid_viscosity_dvar[_qp][_gas_phase_number][v] +=
          dgas_viscosity_dp * _dporepressure_dvar[_qp][_gas_phase_number][v];
      _dfluid_viscosity_dvar[_qp][_aqueous_phase_number][v] +=
          dliquid_viscosity_dT * _dtemperature_dvar[_qp][v];
      _dfluid_viscosity_dvar[_qp][_gas_phase_number][v] +=
          dgas_viscosity_dT * _dtemperature_dvar[_qp][v];
      _dfluid_viscosity_dvar[_qp][_aqueous_phase_number][v] += dliquid_viscosity_dz * dz_dvar[v];
      _dfluid_viscosity_dvar[_qp][_gas_phase_number][v] += dgas_viscosity_dz * dz_dvar[v];

      // The derivative of the mass fractions for each fluid component in each
      // phase
      _dmass_frac_dvar[_qp][_aqueous_phase_number][_aqueous_fluid_component][v] =
          -dX0_dp * _dporepressure_dvar[_qp][_aqueous_phase_number][v];
      _dmass_frac_dvar[_qp][_aqueous_phase_number][_aqueous_fluid_component][v] -=
          dX0_dT * _dtemperature_dvar[_qp][v];
      _dmass_frac_dvar[_qp][_aqueous_phase_number][_aqueous_fluid_component][v] -=
          dX0_dz * dz_dvar[v];
      _dmass_frac_dvar[_qp][_aqueous_phase_number][_gas_fluid_component][v] =
          dX0_dp * _dporepressure_dvar[_qp][_aqueous_phase_number][v];
      _dmass_frac_dvar[_qp][_aqueous_phase_number][_gas_fluid_component][v] +=
          dX0_dT * _dtemperature_dvar[_qp][v];
      _dmass_frac_dvar[_qp][_aqueous_phase_number][_gas_fluid_component][v] += dX0_dz * dz_dvar[v];
      _dmass_frac_dvar[_qp][_gas_phase_number][_aqueous_fluid_component][v] =
          -dY0_dp * _dporepressure_dvar[_qp][_gas_phase_number][v];
      _dmass_frac_dvar[_qp][_gas_phase_number][_aqueous_fluid_component][v] -=
          dY0_dT * _dtemperature_dvar[_qp][v];
      _dmass_frac_dvar[_qp][_gas_phase_number][_aqueous_fluid_component][v] -= dY0_dz * dz_dvar[v];
      _dmass_frac_dvar[_qp][_gas_phase_number][_gas_fluid_component][v] =
          dY0_dp * _dporepressure_dvar[_qp][_gas_phase_number][v];
      _dmass_frac_dvar[_qp][_gas_phase_number][_gas_fluid_component][v] +=
          dY0_dT * _dtemperature_dvar[_qp][v];
      _dmass_frac_dvar[_qp][_gas_phase_number][_gas_fluid_component][v] += dY0_dz * dz_dvar[v];
    }
  }

  // If the material properties are being evaluated at the qps, calculate the
  // gradients as well. Note: only nodal properties are evaluated in
  // initQpStatefulProperties(), so no need to check _is_initqp flag for qp
  // properties
  if (!_nodal_material)
  {
    Real dpc = dCapillaryPressure_dS(seff) * _dseff_ds;
    (*_grads_qp)[_qp][_gas_phase_number] =
        _dsaturation_dvar[_qp][_gas_phase_number][_pvar] * _gas_gradp_qp[_qp] +
        _dsaturation_dvar[_qp][_gas_phase_number][_zvar[0]] * (*_gradz_qp[0])[_qp];
    (*_grads_qp)[_qp][_aqueous_phase_number] = -(*_grads_qp)[_qp][_gas_phase_number];

    (*_gradp_qp)[_qp][_gas_phase_number] = _gas_gradp_qp[_qp];
    (*_gradp_qp)[_qp][_aqueous_phase_number] =
        _gas_gradp_qp[_qp] + dpc * (*_grads_qp)[_qp][_aqueous_phase_number];

    (*_grad_mass_frac_qp)[_qp][_aqueous_phase_number][_aqueous_fluid_component] =
        -dX0_dp * _gas_gradp_qp[_qp] - dX0_dz * (*_gradz_qp[0])[_qp];
    (*_grad_mass_frac_qp)[_qp][_aqueous_phase_number][_gas_fluid_component] =
        -(*_grad_mass_frac_qp)[_qp][_aqueous_phase_number][_aqueous_fluid_component];
    (*_grad_mass_frac_qp)[_qp][_gas_phase_number][_aqueous_fluid_component] =
        -dY0_dp * _gas_gradp_qp[_qp] - dY0_dz * (*_gradz_qp[0])[_qp];
    (*_grad_mass_frac_qp)[_qp][_gas_phase_number][_gas_fluid_component] =
        -(*_grad_mass_frac_qp)[_qp][_gas_phase_number][_aqueous_fluid_component];
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
