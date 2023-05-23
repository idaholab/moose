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
  params.addRequiredParam<Real>("weight_fraction_K",
                                "Weight fraction of potassium in NaK. Defaults to eutectic");
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
  std::cout << _Nk << " 67 ???" << std::endl;
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
NaKFluidProperties::T_from_p_h(Real p, Real h) const
{
  auto lambda = [&](Real p, Real current_T, Real & new_h, Real & dh_dp, Real & dh_dT)
  { h_from_p_T(p, current_T, new_h, dh_dp, dh_dT); };
  Real T = FluidPropertiesUtils::NewtonSolve(
               p, h, _T_initial_guess, _tolerance, lambda, name() + "::T_from_p_h")
               .first;
  // check for nans
  if (std::isnan(T))
    mooseError("Conversion from pressure (p = ",
               p,
               ") and enthalpy (h = ",
               h,
               ") to temperature failed to converge.");
  return T;
}

Real
NaKFluidProperties::T_from_p_rho(Real /* pressure */, Real temperature) const
{
  // NOTE we could also invert analytically the third degree polynomial, see Cardan's method
  auto lambda = [&](Real p, Real current_T, Real & new_rho, Real & drho_dp, Real & drho_dT)
  { rho_from_p_T(p, current_T, new_rho, drho_dp, drho_dT); };
  Real T = FluidPropertiesUtils::NewtonSolve(
               p, rho, _T_initial_guess, _tolerance, lambda, name() + "::T_from_p_rho")
               .first;
  // check for nans
  if (std::isnan(T))
    mooseError("Conversion from pressure (p = ",
               p,
               ") and density (rho = ",
               rho,
               ") to temperature failed to converge.");
  return T;
}

Real
NaKFluidProperties::rho_from_p_T(Real pressure, Real temperature) const
{
  const Real Tc = temperature - _T_k2c;
  // Range from liquid Na and K density correlation ranges
  if (Tc < 210 || Tc > 1110)
    flagInvalidSolution(
        "NaK density evaluated outside of Na density temperature range [210, 1110] C");
  if (Tc < 63.2 || Tc > 1250)
    flagInvalidSolution(
        "NaK density evaluated outside of K density temperature range [63, 1250] C");

  // Eq. 1.8 page 18 of NaK handbook
  const Real v_k = 0.8415 - 2.172e-3 * Tc - 2.7e-8 * Tc * Tc + 4.77e-12 * Tc * Tc * Tc;
  // Eq. 1.5 page 15 of NaK handbook
  const Real v_Na = 0.9453 - 2.2473e-4 * Tc;
  // Eq. 1.9 page 15 of NaK handbook
  std::cout << "Density = " << 1 / (_Nk * v_k + (1 - _Nk) * v_Na) << std::endl;
  return 1 / (_Nk * v_k + (1 - _Nk) * v_Na);
}

void
NaKFluidProperties::rho_from_p_T(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = this->rho_from_p_T(pressure, temperature);
  drho_dp = 0;

  const Real Tc = temperature - _T_k2c;
  // Eq. 1.8 page 18 of NaK handbook
  const Real v_k = 0.8415 - 2.172e-3 * Tc - 2.7e-8 * Tc * Tc + 4.77e-12 * Tc * Tc * Tc;
  // Eq. 1.5 page 15 of NaK handbook
  const Real v_Na = 0.9453 - 2.2473e-4 * Tc;
  drho_dT =
      -(_Nk * (-2.172e-3 - 2 * 2.7e-8 * Tc + 3 * 4.77e-12 * Tc * Tc) + (1 - _Nk) * -2.2473e-4) /
      MathUtils::pow(_Nk * v_k + (1 - _Nk) * v_Na, 2);
}

Real
NaKFluidProperties::e_from_p_T(Real pressure, Real temperature) const
{
  return h_from_p_T(pressure, temperature) - pressure / rho_from_p_T(pressure, temperature);
}

Real
NaKFluidProperties::e_from_p_rho(Real pressure, Real temperature) const
{
  Real temperature = T_from_p_rho(pressure, rho);
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
NaKFluidProperties::cp_from_p_T(Real pressure, Real temperature) const
{
  const Real Tc = temperature - _T_k2c;
  // Eq. 1.59 page 53 of the NaK handbook
  // converted from cal/g/C
  return (0.232 - 8.82e-5 * Tc + 8.2e-8 * Tc * Tc) * 4200;
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
  const Real rho = rho_from_p_T(-1, temperature);
  if (temperature < _T_k2c + 400)
    return 0.001 * 0.116 * std::pow(rho, 1 / 3) * exp(688 * rho / T);
  else
    return 0.001 * 0.082 * std::pow(rho, 1 / 3) * exp(979 * rho / T);
}

Real
NaKFluidProperties::k_from_p_T(Real /*pressure*/, Real temperature) const
{
  const Real Tc = temperature - _T_k2c;
  /* eq. 1.53 page 46 handbook */
  // Note: reported as very sensitive to the composition
  // Range: 150 - 680 C
  if (Tc < 150 || Tc > 680)
    flagInvalidSolution(
        "NaK thermal diffusivity evaluated outside of temperature range [150, 680] C");
  return 0.214 + 2.07e-4 * Tc - 2.2e-7 * Tc * Tc;
}

void
NaKFluidProperties::k_from_p_T(
    Real /*pressure*/, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const
{
  const Real Tc = temperature - _T_k2c;
  k = k_from_p_T(0, temperature);
  dk_dp = 0.0;
  dk_dT = 2.07e-4 - 2 * 2.2e-7 * Tc;
}

Real
NaKFluidProperties::h_from_p_T(Real pressure, Real temperature) const
{
  // Integration of cv from T_ref = T_fusion
  const Real T_ref = -12.6; // celsius
  const Real Tc = temperature - _T_k2c;
  return (0.232 * (Tc - T_ref) - 8.82e-5 / 2 * (Tc - T_ref) * (Tc - T_ref) +
          8.2e-8 / 3 * (Tc - T_ref) * (Tc - T_ref) * (Tc - T_ref)) *
         4200;
}

void
NaKFluidProperties::h_from_p_T(
    Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const
{
  const Real T_ref = -12.6; // celsius
  const Real Tc = temperature - _T_k2c;
  h = (0.232 * (Tc - T_ref) - 8.82e-5 / 2 * (Tc - T_ref) * (Tc - T_ref) +
       8.2e-8 / 3 * (Tc - T_ref) * (Tc - T_ref) * (Tc - T_ref)) *
      4200;
  dh_dp = 0;
  dh_dT = cp_from_p_T(pressure, temperature);
}
