//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NaKFluidProperties.h"

registerMooseObject("FluidPropertiesApp", NaKFluidProperties);

InputParameters
NaKFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
  params.addClassDescription("Fluid properties for NaK");
  params.addRequiredParam<Real>(
      "weight_fraction_K",
      "Weight fraction of potassium in NaK. Only eutectic is implemented (X_K = 0.778)");
  return params;
}

NaKFluidProperties::NaKFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters), _MNaK((39.102 + 22.9898) / 1000)
{
  // We did not implement other weight fractions
  const Real Xk = getParam<Real>("weight_fraction_K");
  if (Xk != 0.778)
    paramError("weight_fraction_K",
               "Only 0.778 weight percent potassium (eutectic) is implemented");

  // Compute mole fractions from mass fractions
  const Real AwK = 39.102;
  const Real AwNa = 22.9898;
  _Nk = Xk / AwK / (Xk / AwK + (1 - Xk) / AwNa);
}

NaKFluidProperties::~NaKFluidProperties() {}

std::string
NaKFluidProperties::fluidName() const
{
  return "NaK";
}

Real
NaKFluidProperties::molarMass() const
{
  return _MNaK;
}

Real
NaKFluidProperties::T_from_p_h(Real /* pressure */, Real enthalpy) const
{
  // analytical inversion of h_from_p_T
  Real h2 = enthalpy * enthalpy;
  Real B1 =
      std::pow(183.357574154983 * std::sqrt(8405 * h2 - 8208700353 * enthalpy + 9265308922016000) +
                   16810 * enthalpy - 8208700353,
               1. / 3);
  return _T_c2k + 0.174216027874564 * (3.65930571002297 * B1 - 2.28699205892461e7 / B1 + 3087);
}

Real
NaKFluidProperties::T_from_p_rho(Real pressure, Real density) const
{
  // NOTE we could also invert analytically the third degree polynomial, see Cardan's method
  auto lambda = [&](Real p, Real current_T, Real & new_rho, Real & drho_dp, Real & drho_dT)
  { rho_from_p_T(p, current_T, new_rho, drho_dp, drho_dT); };
  Real T = FluidPropertiesUtils::NewtonSolve(pressure,
                                             density,
                                             _T_initial_guess,
                                             _tolerance,
                                             lambda,
                                             name() + "::T_from_p_rho",
                                             _max_newton_its)
               .first;
  // check for nans
  if (std::isnan(T))
    mooseError("Conversion from pressure (p = ",
               pressure,
               ") and density (rho = ",
               density,
               ") to temperature failed to converge.");
  return T;
}

Real
NaKFluidProperties::rho_from_p_T(Real /*pressure*/, Real temperature) const
{
  const Real Tc = temperature - _T_c2k;
  // Range from liquid Na and K density correlation ranges
  if (Tc < 210 || Tc > 1110)
    flagInvalidSolution(
        "NaK density evaluated outside of Na density temperature range [210, 1110] C");
  if (Tc < 63.2 || Tc > 1250)
    flagInvalidSolution(
        "NaK density evaluated outside of K density temperature range [63, 1250] C");

  // Eq. 1.8 page 18 of NaK handbook
  const Real v_k = 1. / (0.8415 - 2.172e-4 * Tc - 2.7e-8 * Tc * Tc + 4.77e-12 * Tc * Tc * Tc);
  // Eq. 1.5 page 15 of NaK handbook
  const Real v_Na = 1. / (0.9453 - 2.2473e-4 * Tc);
  // Eq. 1.9 page 15 of NaK handbook
  return 1000 / (_Nk * v_k + (1 - _Nk) * v_Na);
}

void
NaKFluidProperties::rho_from_p_T(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = this->rho_from_p_T(pressure, temperature);
  drho_dp = 0;

  const Real Tc = temperature - _T_c2k;
  // Eq. 1.8 page 18 of NaK handbook
  const Real v_k = 1. / (0.8415 - 2.172e-4 * Tc - 2.7e-8 * Tc * Tc + 4.77e-12 * Tc * Tc * Tc);
  // Eq. 1.5 page 15 of NaK handbook
  const Real v_Na = 1. / (0.9453 - 2.2473e-4 * Tc);
  drho_dT =
      1000 *
      -(_Nk * -(-2.172e-4 - 2 * 2.7e-8 * Tc + 3 * 4.77e-12 * Tc * Tc) * MathUtils::pow(v_k, 2) +
        (1 - _Nk) * 2.2473e-4 * MathUtils::pow(v_Na, 2)) /
      MathUtils::pow(_Nk * v_k + (1 - _Nk) * v_Na, 2);
}

Real
NaKFluidProperties::e_from_p_T(Real pressure, Real temperature) const
{
  return h_from_p_T(pressure, temperature) - pressure / rho_from_p_T(pressure, temperature);
}

Real
NaKFluidProperties::e_from_p_rho(Real pressure, Real density) const
{
  Real temperature = T_from_p_rho(pressure, density);
  return e_from_p_T(pressure, temperature);
}

void
NaKFluidProperties::e_from_p_T(
    Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const
{
  Real h, dh_dp, dh_dT;
  h_from_p_T(pressure, temperature, h, dh_dp, dh_dT);
  Real rho, drho_dp, drho_dT;
  rho_from_p_T(pressure, temperature, rho, drho_dp, drho_dT);

  e = h - pressure / rho;
  de_dp = dh_dp + pressure * drho_dp / rho / rho - 1.0 / rho;
  de_dT = dh_dT + pressure * drho_dT / rho / rho;
}

Real
NaKFluidProperties::cp_from_p_T(Real /*pressure*/, Real temperature) const
{
  const Real Tc = temperature - _T_c2k;
  // Eq. 1.59 page 53 of the NaK handbook
  // converted from cal/g/C
  return (0.232 - 8.82e-5 * Tc + 8.2e-8 * Tc * Tc) * 4200;
}

void
NaKFluidProperties::cp_from_p_T(
    Real /*pressure*/, Real temperature, Real & cp, Real & dcp_dp, Real & dcp_dT) const
{
  const Real Tc = temperature - _T_c2k;
  // Eq. 1.59 page 53 of the NaK handbook
  // converted from cal/g/C
  cp = cp_from_p_T(-1, temperature);
  dcp_dp = 0;
  dcp_dT = (-8.82e-5 + 2 * 8.2e-8 * Tc) * 4200;
}

Real
NaKFluidProperties::cv_from_p_T(Real pressure, Real temperature) const
{
  Real e, de_dp, de_dT;
  e_from_p_T(pressure, temperature, e, de_dp, de_dT);
  return de_dT;
}

Real
NaKFluidProperties::mu_from_p_T(Real /*pressure*/, Real temperature) const
{
  /* eq. 1.18-19 page 24 of NaK handbook */
  // No range given
  // Converted from centipoise
  const Real rho = rho_from_p_T(-1, temperature) / 1000;
  if (temperature < _T_c2k + 400)
    return 0.001 * 0.116 * std::pow(rho, 1 / 3.) * exp(688 * rho / temperature);
  else
    return 0.001 * 0.082 * std::pow(rho, 1 / 3.) * exp(979 * rho / temperature);
}

void
NaKFluidProperties::mu_from_p_T(
    Real /*pressure*/, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  /* eq. 1.18-19 page 24 of NaK handbook */
  // No range given
  // Converted from centipoise
  Real rho, drho_dp, drho_dT;
  rho_from_p_T(-1, temperature, rho, drho_dp, drho_dT);
  rho /= 1000;
  drho_dT /= 1000;
  if (temperature < _T_c2k + 400)
  {
    mu = 0.001 * 0.116 * std::pow(rho, 1 / 3.) * exp(688 * rho / temperature);
    dmu_dp = 0;
    dmu_dT =
        0.001 * 0.116 * exp(688 * rho / temperature) *
        (1 / 3. * drho_dT * std::pow(rho, -2 / 3.) +
         std::pow(rho, 1 / 3.) * 688 * (drho_dT * temperature - rho) / temperature / temperature);
  }
  else
  {
    mu = 0.001 * 0.082 * std::pow(rho, 1 / 3.) * exp(979 * rho / temperature);
    dmu_dp = 0;
    dmu_dT =
        0.001 * 0.082 * exp(979 * rho / temperature) *
        (1 / 3. * drho_dT * std::pow(rho, -2 / 3.) +
         std::pow(rho, 1 / 3.) * 979 * (drho_dT * temperature - rho) / temperature / temperature);
  }
}

Real
NaKFluidProperties::k_from_p_T(Real /*pressure*/, Real temperature) const
{
  const Real Tc = temperature - _T_c2k;
  /* eq. 1.53 page 46 handbook */
  // Note: reported as very sensitive to the composition
  // Range: 150 - 680 C
  // Converted from (W/cm-C)
  if (Tc < 150 || Tc > 680)
    flagInvalidSolution(
        "NaK thermal diffusivity evaluated outside of temperature range [150, 680] C");
  return 100 * (0.214 + 2.07e-4 * Tc - 2.2e-7 * Tc * Tc);
}

void
NaKFluidProperties::k_from_p_T(
    Real /*pressure*/, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const
{
  const Real Tc = temperature - _T_c2k;
  k = k_from_p_T(0, temperature);
  dk_dp = 0.0;
  dk_dT = 100 * (2.07e-4 - 2 * 2.2e-7 * Tc);
}

Real
NaKFluidProperties::h_from_p_T(Real /*pressure*/, Real temperature) const
{
  // Integration of cv
  const Real Tc = temperature - _T_c2k;
  return (0.232 * Tc - 8.82e-5 / 2 * Tc * Tc + 8.2e-8 / 3 * Tc * Tc * Tc) * 4200;
}

void
NaKFluidProperties::h_from_p_T(
    Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const
{
  const Real Tc = temperature - _T_c2k;
  h = (0.232 * Tc - 8.82e-5 / 2 * Tc * Tc + 8.2e-8 / 3 * Tc * Tc * Tc) * 4200;
  dh_dp = 0;
  dh_dT = cp_from_p_T(pressure, temperature);
}
