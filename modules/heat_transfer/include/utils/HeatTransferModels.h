//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HeatConductionNames.h"

namespace HeatTransferModels
{

/**
 * Computes the thermal conductance across a cylindrical medium.
 *
 * Note that thermal conductance has the same units as a heat transfer coefficient.
 *
 * @param[in] k        Thermal conductivity of the medium
 * @param[in] r_inner  Inner radius
 * @param[in] r_outer  Outer radius
 */
template <typename T1, typename T2, typename T3>
auto
cylindricalThermalConductance(const T1 & k, const T2 & r_inner, const T3 & r_outer)
{
  mooseAssert(r_outer > r_inner, "Outer radius must be larger than inner radius.");
  using std::log;

  const auto r_avg = 0.5 * (r_inner + r_outer);
  return k / (r_avg * log(r_outer / r_inner));
}

/**
 * Computes the conduction heat flux across a cylindrical gap.
 *
 * The convention is that positive heat fluxes correspond to heat moving from
 * the inner surface to the outer surface.
 *
 * @param[in] k_gap    Gap thermal conductivity
 * @param[in] r_inner  Inner radius
 * @param[in] r_outer  Outer radius
 * @param[in] T_inner  Inner temperature
 * @param[in] T_outer  Outer temperature
 */
template <typename T1, typename T2, typename T3, typename T4, typename T5>
auto
cylindricalGapConductionHeatFlux(const T1 & k_gap,
                                 const T2 & r_inner,
                                 const T3 & r_outer,
                                 const T4 & T_inner,
                                 const T5 & T_outer)
{
  return cylindricalThermalConductance(k_gap, r_inner, r_outer) * (T_inner - T_outer);
}

/**
 * Computes the radiation heat flux across a cylindrical gap.
 *
 * The convention is that positive heat fluxes correspond to heat moving from
 * the inner surface to the outer surface.
 *
 * @param[in] r_inner      Inner radius
 * @param[in] r_outer      Outer radius
 * @param[in] emiss_inner  Inner emissivity
 * @param[in] emiss_outer  Outer emissivity
 * @param[in] T_inner      Inner temperature
 * @param[in] T_outer      Outer temperature
 * @param[in] sigma        The Stefan-Boltzmann constant
 */
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
auto
cylindricalGapRadiationHeatFlux(const T1 & r_inner,
                                const T2 & r_outer,
                                const T3 & emiss_inner,
                                const T4 & emiss_outer,
                                const T5 & T_inner,
                                const T6 & T_outer,
                                const Real & sigma = HeatConduction::Constants::sigma)
{
  mooseAssert(r_outer > r_inner, "Outer radius must be larger than inner radius.");
  using std::pow;

  const auto rad_resistance =
      1.0 / emiss_inner + r_inner / r_outer * (1.0 - emiss_outer) / emiss_outer;
  return sigma * (pow(T_inner, 4) - pow(T_outer, 4)) / rad_resistance;
}

/**
 * Error-controlled adaptive Simpson integrator.
 *
 * Implements recursive Simpson's rule with a simple error estimator and
 * Richardson correction. The interval is subdivided until the
 * absolute/relative tolerance is met or max_depth is reached.
 *
 * @param[in] f         Integrand f(x)
 * @param[in] a         Lower bound
 * @param[in] b         Upper bound
 * @param[in] abs_tol   Absolute tolerance (default 1e-8)
 * @param[in] rel_tol   Relative tolerance (default 1e-6)
 * @param[in] max_depth Maximum recursion depth (default 20)
 */
template <typename Scalar, typename Fun>
Scalar
adaptiveSimpson(const Fun & f,
                Scalar a,
                Scalar b,
                Scalar abs_tol = Scalar(1E-8),
                Scalar rel_tol = Scalar(1E-6),
                unsigned max_depth = 20)
{
  auto simpson = [&](Scalar x0, Scalar x1) -> Scalar
  {
    const Scalar xm = Scalar(0.5) * (x0 + x1);
    return (x1 - x0) * (f(x0) + Scalar(4) * f(xm) + f(x1)) / Scalar(6);
  };

  std::function<Scalar(Scalar, Scalar, Scalar, Scalar, unsigned)> recurse =
      [&](Scalar x0, Scalar x1, Scalar S0, Scalar tol, unsigned depth) -> Scalar
  {
    const Scalar xm = Scalar(0.5) * (x0 + x1);
    const Scalar Sl = simpson(x0, xm);
    const Scalar Sr = simpson(xm, x1);
    const Scalar err = std::fabs(Sl + Sr - S0);

    if (err < tol || depth == 0)
      return Sl + Sr + err / Scalar(15); // Richardson correction

    return recurse(x0, xm, Sl, tol * Scalar(0.5), depth - 1) +
           recurse(xm, x1, Sr, tol * Scalar(0.5), depth - 1);
  };

  const Scalar S0 = simpson(a, b);
  const Scalar tol = std::max(abs_tol, rel_tol * std::fabs(S0));

  return recurse(a, b, S0, tol, max_depth);
}

/**
 * Band-integrated Planck emission
 * \f$ 4\pi\,\kappa \int_{\nu_a}^{\nu_b} B_\nu(T, n_1)\, d\nu \f$.
 *
 * Computes the band emissive power (per unit volume) for a participating medium
 * with refractive index n1 and absorption coefficient kappa over the frequency
 * band [nu_a, nu_b]. Uses the same adaptive Simpson integrator above.
 *
 * @param[in] n1              Refractive index of incident medium
 * @param[in] kappa           Absorption coefficient
 * @param[in] T               Temperature
 * @param[in] nu_a            Lower frequency bound
 * @param[in] nu_b            Upper frequency bound
 * @param[in] abs_tol         Absolute tolerance for the integrator (default 1e-8)
 * @param[in] rel_tol         Relative tolerance for the integrator (default 1e-6)
 * @param[in] planck_units    Units string for Planck constant (default "J*s")
 * @param[in] sol_units       Units string for speed of light (default "m/s")
 * @param[in] boltzmann_units Units string for Boltzmann constant (default "J/K")
 */
template <typename Scalar>
Scalar
integratedPlanckBand(Scalar n1,
                     Scalar kappa,
                     Scalar T,
                     Scalar nu_a,
                     Scalar nu_b,
                     Scalar abs_tol = Scalar(1E-8),
                     Scalar rel_tol = Scalar(1E-6),
                     const std::string & planck_units = "J*s",
                     const std::string & sol_units = "m/s",
                     const std::string & boltzmann_units = "J/K")
{
  const auto hp = HeatConduction::Constants::planckConstant(planck_units);
  const auto c0 = HeatConduction::Constants::speedOfLight(sol_units);
  const auto kb = HeatConduction::Constants::boltzmannConstant(boltzmann_units);

  const Scalar n1_sq = n1 * n1;

  // Planck spectrum
  auto planck = [=](Scalar nu) -> Scalar
  {
    const Scalar pre = Scalar(2) * n1_sq * hp * nu * nu * nu / Utility::pow<2>(c0);

    const Scalar expo = hp * nu / (kb * T);

    const Scalar inv = std::exp(expo) - Scalar(1);

    return pre / inv; // spectral emissive power
  };

  const Scalar integral = adaptiveSimpson<Scalar>(planck, nu_a, nu_b, abs_tol, rel_tol);

  return Scalar(4.0) * pi * kappa * integral;
}
}
