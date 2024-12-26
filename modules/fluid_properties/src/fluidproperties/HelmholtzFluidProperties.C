//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HelmholtzFluidProperties.h"
#include "BrentsMethod.h"
#include "libmesh/utility.h"

InputParameters
HelmholtzFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
  params.addClassDescription("Base class for Helmholtz free energy fluid EOS");
  return params;
}

HelmholtzFluidProperties::HelmholtzFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters)
{
}

Real
HelmholtzFluidProperties::rho_from_p_T(Real pressure, Real temperature) const
{
  Real density;
  // Initial estimate of a bracketing interval for the density
  Real lower_density = 1.0e-2;
  Real upper_density = 100.0;

  // The density is found by finding the zero of the pressure
  auto pressure_diff = [&pressure, &temperature, this](Real x)
  { return this->p_from_rho_T(x, temperature) - pressure; };

  BrentsMethod::bracket(pressure_diff, lower_density, upper_density);
  density = BrentsMethod::root(pressure_diff, lower_density, upper_density);

  return density;
}

void
HelmholtzFluidProperties::rho_from_p_T(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = this->rho_from_p_T(pressure, temperature);

  // Scale the density and temperature
  const Real delta = rho / criticalDensity();
  const Real tau = criticalTemperature() / temperature;
  const Real da_dd = dalpha_ddelta(delta, tau);
  const Real d2a_dd2 = d2alpha_ddelta2(delta, tau);

  drho_dp = molarMass() / (_R * temperature * delta * (2.0 * da_dd + delta * d2a_dd2));
  drho_dT = rho * (tau * d2alpha_ddeltatau(delta, tau) - da_dd) / temperature /
            (2.0 * da_dd + delta * d2a_dd2);
}

Real
HelmholtzFluidProperties::e_from_p_T(Real pressure, Real temperature) const
{
  // Require density first
  const Real density = rho_from_p_T(pressure, temperature);
  // Scale the input density and temperature
  const Real delta = density / criticalDensity();
  const Real tau = criticalTemperature() / temperature;

  return _R * temperature * tau * dalpha_dtau(delta, tau) / molarMass();
}

void
HelmholtzFluidProperties::e_from_p_T(
    Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const
{
  e = this->e_from_p_T(pressure, temperature);

  // Require density first
  const Real density = rho_from_p_T(pressure, temperature);
  // Scale the input density and temperature
  const Real delta = density / criticalDensity();
  const Real tau = criticalTemperature() / temperature;

  const Real da_dd = dalpha_ddelta(delta, tau);
  const Real d2a_dd2 = d2alpha_ddelta2(delta, tau);
  const Real d2a_ddt = d2alpha_ddeltatau(delta, tau);

  de_dp = tau * d2a_ddt / (density * (2.0 * da_dd + delta * d2a_dd2));
  de_dT = -_R *
          (delta * tau * d2a_ddt * (da_dd - tau * d2a_ddt) / (2.0 * da_dd + delta * d2a_dd2) +
           tau * tau * d2alpha_dtau2(delta, tau)) /
          molarMass();
}

Real
HelmholtzFluidProperties::c_from_p_T(Real pressure, Real temperature) const
{
  // Require density first
  const Real density = rho_from_p_T(pressure, temperature);
  // Scale the input density and temperature
  const Real delta = density / criticalDensity();
  const Real tau = criticalTemperature() / temperature;

  const Real da_dd = dalpha_ddelta(delta, tau);

  Real w = 2.0 * delta * da_dd + delta * delta * d2alpha_ddelta2(delta, tau);
  w -= Utility::pow<2>(delta * da_dd - delta * tau * d2alpha_ddeltatau(delta, tau)) /
       (tau * tau * d2alpha_dtau2(delta, tau));

  return std::sqrt(_R * temperature * w / molarMass());
}

Real
HelmholtzFluidProperties::cp_from_p_T(Real pressure, Real temperature) const
{
  // Require density first
  const Real density = rho_from_p_T(pressure, temperature);
  // Scale the input density and temperature
  const Real delta = density / criticalDensity();
  const Real tau = criticalTemperature() / temperature;

  const Real da_dd = dalpha_ddelta(delta, tau);

  const Real cp = _R *
                  (-tau * tau * d2alpha_dtau2(delta, tau) +
                   Utility::pow<2>(delta * da_dd - delta * tau * d2alpha_ddeltatau(delta, tau)) /
                       (2.0 * delta * da_dd + delta * delta * d2alpha_ddelta2(delta, tau))) /
                  molarMass();

  return cp;
}

Real
HelmholtzFluidProperties::cv_from_p_T(Real pressure, Real temperature) const
{
  // Require density first
  const Real density = rho_from_p_T(pressure, temperature);
  // Scale the input density and temperature
  const Real delta = density / criticalDensity();
  const Real tau = criticalTemperature() / temperature;

  return -_R * tau * tau * d2alpha_dtau2(delta, tau) / molarMass();
}

Real
HelmholtzFluidProperties::s_from_p_T(Real pressure, Real temperature) const
{
  // Require density first
  const Real density = rho_from_p_T(pressure, temperature);
  // Scale the input density and temperature
  const Real delta = density / criticalDensity();
  const Real tau = criticalTemperature() / temperature;

  return _R * (tau * dalpha_dtau(delta, tau) - alpha(delta, tau)) / molarMass();
}

void
HelmholtzFluidProperties::s_from_p_T(
    Real pressure, Real temperature, Real & s, Real & ds_dp, Real & ds_dT) const
{
  s = this->s_from_p_T(pressure, temperature);

  // Require density first
  const Real density = rho_from_p_T(pressure, temperature);
  // Scale the input density and temperature
  const Real delta = density / criticalDensity();
  const Real tau = criticalTemperature() / temperature;

  const Real da_dd = dalpha_ddelta(delta, tau);
  const Real da_dt = dalpha_dtau(delta, tau);
  const Real d2a_dd2 = d2alpha_ddelta2(delta, tau);
  const Real d2a_dt2 = d2alpha_dtau2(delta, tau);
  const Real d2a_ddt = d2alpha_ddeltatau(delta, tau);

  ds_dp = tau * (d2a_ddt - da_dd) / (density * temperature * (2.0 * da_dd + delta * d2a_dd2));
  ds_dT = -_R * tau * (da_dt - alpha(delta, tau) + tau * (d2a_dt2 - da_dt)) /
          (molarMass() * temperature);
}

Real
HelmholtzFluidProperties::h_from_p_T(Real pressure, Real temperature) const
{
  // Require density first
  const Real density = rho_from_p_T(pressure, temperature);
  // Scale the input density and temperature
  const Real delta = density / criticalDensity();
  const Real tau = criticalTemperature() / temperature;

  return _R * temperature * (tau * dalpha_dtau(delta, tau) + delta * dalpha_ddelta(delta, tau)) /
         molarMass();
}

void
HelmholtzFluidProperties::h_from_p_T(
    Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const
{
  h = this->h_from_p_T(pressure, temperature);

  // Require density first
  const Real density = rho_from_p_T(pressure, temperature);
  // Scale the input density and temperature
  const Real delta = density / criticalDensity();
  const Real tau = criticalTemperature() / temperature;

  const Real da_dd = dalpha_ddelta(delta, tau);
  const Real d2a_dd2 = d2alpha_ddelta2(delta, tau);
  const Real d2a_ddt = d2alpha_ddeltatau(delta, tau);

  dh_dp = (da_dd + delta * d2a_dd2 + tau * d2a_ddt) / (density * (2.0 * da_dd + delta * d2a_dd2));
  dh_dT = _R *
          (delta * da_dd * (1.0 - tau * d2a_ddt / da_dd) * (1.0 - tau * d2a_ddt / da_dd) /
               (2.0 + delta * d2a_dd2 / da_dd) -
           tau * tau * d2alpha_dtau2(delta, tau)) /
          molarMass();
}

Real
HelmholtzFluidProperties::T_from_p_h(Real pressure, Real enthalpy) const
{
  auto lambda = [&](Real pressure, Real current_T, Real & new_h, Real & dh_dp, Real & dh_dT)
  { h_from_p_T(pressure, current_T, new_h, dh_dp, dh_dT); };
  Real T = FluidPropertiesUtils::NewtonSolve(
               pressure, enthalpy, _T_initial_guess, _tolerance, lambda, name() + "::T_from_p_h")
               .first;
  // check for nans
  if (std::isnan(T))
    mooseError("Conversion from enthalpy (h = ",
               enthalpy,
               ") and pressure (p = ",
               pressure,
               ") to temperature failed to converge.");
  return T;
}

Real
HelmholtzFluidProperties::p_from_rho_T(Real density, Real temperature) const
{
  // Scale the input density and temperature
  const Real delta = density / criticalDensity();
  const Real tau = criticalTemperature() / temperature;

  return _R * density * temperature * delta * dalpha_ddelta(delta, tau) / molarMass();
}
