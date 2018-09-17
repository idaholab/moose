//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Water97FluidProperties.h"
#include "Conversion.h"
#include "MathUtils.h"
#include "libmesh/utility.h"

registerMooseObject("FluidPropertiesApp", Water97FluidProperties);

template <>
InputParameters
validParams<Water97FluidProperties>()
{
  InputParameters params = validParams<SinglePhaseFluidPropertiesPT>();
  params.addClassDescription("Fluid properties for water and steam (H2O) using IAPWS-IF97");
  return params;
}

Water97FluidProperties::Water97FluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidPropertiesPT(parameters),
    _Mh2o(18.015e-3),
    _Rw(461.526),
    _p_critical(22.064e6),
    _T_critical(647.096),
    _rho_critical(322.0),
    _p_triple(611.657),
    _T_triple(273.16)
{
}

Water97FluidProperties::~Water97FluidProperties() {}

std::string
Water97FluidProperties::fluidName() const
{
  return "water";
}

Real
Water97FluidProperties::molarMass() const
{
  return _Mh2o;
}

Real
Water97FluidProperties::criticalPressure() const
{
  return _p_critical;
}

Real
Water97FluidProperties::criticalTemperature() const
{
  return _T_critical;
}

Real
Water97FluidProperties::criticalDensity() const
{
  return _rho_critical;
}

Real
Water97FluidProperties::triplePointPressure() const
{
  return _p_triple;
}

Real
Water97FluidProperties::triplePointTemperature() const
{
  return _T_triple;
}

Real
Water97FluidProperties::rho_from_p_T(Real pressure, Real temperature) const
{
  Real density, pi, tau;

  // Determine which region the point is in
  unsigned int region = inRegion(pressure, temperature);

  switch (region)
  {
    case 1:
      pi = pressure / _p_star[0];
      tau = _T_star[0] / temperature;
      density = pressure / (pi * _Rw * temperature * dgamma1_dpi(pi, tau));
      break;

    case 2:
      pi = pressure / _p_star[1];
      tau = _T_star[1] / temperature;
      density = pressure / (pi * _Rw * temperature * dgamma2_dpi(pi, tau));
      break;

    case 3:
      density = densityRegion3(pressure, temperature);
      break;

    case 5:
      pi = pressure / _p_star[4];
      tau = _T_star[4] / temperature;
      density = pressure / (pi * _Rw * temperature * dgamma5_dpi(pi, tau));
      break;

    default:
      mooseError(name(), ": inRegion() has given an incorrect region");
  }
  return density;
}

void
Water97FluidProperties::rho_from_p_T(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  Real pi, tau, ddensity_dp, ddensity_dT;

  // Determine which region the point is in
  unsigned int region = inRegion(pressure, temperature);

  switch (region)
  {
    case 1:
    {
      pi = pressure / _p_star[0];
      tau = _T_star[0] / temperature;
      Real dgdp = dgamma1_dpi(pi, tau);
      ddensity_dp = -d2gamma1_dpi2(pi, tau) / (_Rw * temperature * dgdp * dgdp);
      ddensity_dT = -pressure * (dgdp - tau * d2gamma1_dpitau(pi, tau)) /
                    (_Rw * pi * temperature * temperature * dgdp * dgdp);
      break;
    }

    case 2:
    {
      pi = pressure / _p_star[1];
      tau = _T_star[1] / temperature;
      Real dgdp = dgamma2_dpi(pi, tau);
      ddensity_dp = -d2gamma2_dpi2(pi, tau) / (_Rw * temperature * dgdp * dgdp);
      ddensity_dT = -pressure * (dgdp - tau * d2gamma2_dpitau(pi, tau)) /
                    (_Rw * pi * temperature * temperature * dgdp * dgdp);
      break;
    }

    case 3:
    {
      // Calculate density first, then use that in Helmholtz free energy
      Real density = densityRegion3(pressure, temperature);
      Real delta = density / _rho_critical;
      tau = _T_star[2] / temperature;
      Real dpdd = dphi3_ddelta(delta, tau);
      Real d2pdd2 = d2phi3_ddelta2(delta, tau);
      ddensity_dp = 1.0 / (_Rw * temperature * delta * (2.0 * dpdd + delta * d2pdd2));
      ddensity_dT = density * (tau * d2phi3_ddeltatau(delta, tau) - dpdd) / temperature /
                    (2.0 * dpdd + delta * d2pdd2);
      break;
    }

    case 5:
    {
      pi = pressure / _p_star[4];
      tau = _T_star[4] / temperature;
      Real dgdp = dgamma5_dpi(pi, tau);
      ddensity_dp = -d2gamma5_dpi2(pi, tau) / (_Rw * temperature * dgdp * dgdp);
      ddensity_dT = -pressure * (dgdp - tau * d2gamma5_dpitau(pi, tau)) /
                    (_Rw * pi * temperature * temperature * dgdp * dgdp);
      break;
    }

    default:
      mooseError(name(), ": inRegion() has given an incorrect region");
  }

  rho = this->rho_from_p_T(pressure, temperature);
  drho_dp = ddensity_dp;
  drho_dT = ddensity_dT;
}

Real
Water97FluidProperties::e_from_p_T(Real pressure, Real temperature) const
{
  Real internal_energy, pi, tau;

  // Determine which region the point is in
  unsigned int region = inRegion(pressure, temperature);
  switch (region)
  {
    case 1:
      pi = pressure / _p_star[0];
      tau = _T_star[0] / temperature;
      internal_energy =
          _Rw * temperature * (tau * dgamma1_dtau(pi, tau) - pi * dgamma1_dpi(pi, tau));
      break;

    case 2:
      pi = pressure / _p_star[1];
      tau = _T_star[1] / temperature;
      internal_energy =
          _Rw * temperature * (tau * dgamma2_dtau(pi, tau) - pi * dgamma2_dpi(pi, tau));
      break;

    case 3:
    {
      // Calculate density first, then use that in Helmholtz free energy
      Real density3 = densityRegion3(pressure, temperature);
      Real delta = density3 / _rho_critical;
      tau = _T_star[2] / temperature;
      internal_energy = _Rw * temperature * tau * dphi3_dtau(delta, tau);
      break;
    }

    case 5:
      pi = pressure / _p_star[4];
      tau = _T_star[4] / temperature;
      internal_energy =
          _Rw * temperature * (tau * dgamma5_dtau(pi, tau) - pi * dgamma5_dpi(pi, tau));
      break;

    default:
      mooseError(name(), ": inRegion() has given an incorrect region");
  }
  // Output in J/kg
  return internal_energy;
}

void
Water97FluidProperties::e_from_p_T(
    Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const
{
  Real pi, tau, dinternal_energy_dp, dinternal_energy_dT;

  // Determine which region the point is in
  unsigned int region = inRegion(pressure, temperature);
  switch (region)
  {
    case 1:
    {
      pi = pressure / _p_star[0];
      tau = _T_star[0] / temperature;
      Real dgdp = dgamma1_dpi(pi, tau);
      Real d2gdpt = d2gamma1_dpitau(pi, tau);
      dinternal_energy_dp =
          _Rw * temperature * (tau * d2gdpt - dgdp - pi * d2gamma1_dpi2(pi, tau)) / _p_star[0];
      dinternal_energy_dT =
          _Rw * (pi * tau * d2gdpt - tau * tau * d2gamma1_dtau2(pi, tau) - pi * dgdp);
      break;
    }

    case 2:
    {
      pi = pressure / _p_star[1];
      tau = _T_star[1] / temperature;
      Real dgdp = dgamma2_dpi(pi, tau);
      Real d2gdpt = d2gamma2_dpitau(pi, tau);
      dinternal_energy_dp =
          _Rw * temperature * (tau * d2gdpt - dgdp - pi * d2gamma2_dpi2(pi, tau)) / _p_star[1];
      dinternal_energy_dT =
          _Rw * (pi * tau * d2gdpt - tau * tau * d2gamma2_dtau2(pi, tau) - pi * dgdp);
      break;
    }

    case 3:
    {
      // Calculate density first, then use that in Helmholtz free energy
      Real density3 = densityRegion3(pressure, temperature);
      Real delta = density3 / _rho_critical;
      tau = _T_star[2] / temperature;
      Real dpdd = dphi3_ddelta(delta, tau);
      Real d2pddt = d2phi3_ddeltatau(delta, tau);
      Real d2pdd2 = d2phi3_ddelta2(delta, tau);
      dinternal_energy_dp =
          _T_star[2] * d2pddt / _rho_critical /
          (2.0 * temperature * delta * dpdd + temperature * delta * delta * d2pdd2);
      dinternal_energy_dT =
          -_Rw * (delta * tau * d2pddt * (dpdd - tau * d2pddt) / (2.0 * dpdd + delta * d2pdd2) +
                  tau * tau * d2phi3_dtau2(delta, tau));
      break;
    }

    case 5:
    {
      pi = pressure / _p_star[4];
      tau = _T_star[4] / temperature;
      Real dgdp = dgamma5_dpi(pi, tau);
      Real d2gdpt = d2gamma5_dpitau(pi, tau);
      dinternal_energy_dp =
          _Rw * temperature * (tau * d2gdpt - dgdp - pi * d2gamma5_dpi2(pi, tau)) / _p_star[4];
      dinternal_energy_dT =
          _Rw * (pi * tau * d2gdpt - tau * tau * d2gamma5_dtau2(pi, tau) - pi * dgdp);
      break;
    }

    default:
      mooseError(name(), ": inRegion has given an incorrect region");
  }

  e = this->e_from_p_T(pressure, temperature);
  de_dp = dinternal_energy_dp;
  de_dT = dinternal_energy_dT;
}

void
Water97FluidProperties::rho_e_dpT(Real pressure,
                                  Real temperature,
                                  Real & rho,
                                  Real & drho_dp,
                                  Real & drho_dT,
                                  Real & e,
                                  Real & de_dp,
                                  Real & de_dT) const
{
  rho_from_p_T(pressure, temperature, rho, drho_dp, drho_dT);
  e_from_p_T(pressure, temperature, e, de_dp, de_dT);
}

Real
Water97FluidProperties::c_from_p_T(Real pressure, Real temperature) const
{
  Real speed2, pi, tau, delta;

  // Determine which region the point is in
  unsigned int region = inRegion(pressure, temperature);
  switch (region)
  {
    case 1:
      pi = pressure / _p_star[0];
      tau = _T_star[0] / temperature;
      speed2 = _Rw * temperature * Utility::pow<2>(dgamma1_dpi(pi, tau)) /
               (Utility::pow<2>(dgamma1_dpi(pi, tau) - tau * d2gamma1_dpitau(pi, tau)) /
                    (tau * tau * d2gamma1_dtau2(pi, tau)) -
                d2gamma1_dpi2(pi, tau));
      break;

    case 2:
      pi = pressure / _p_star[1];
      tau = _T_star[1] / temperature;
      speed2 = _Rw * temperature * Utility::pow<2>(pi * dgamma2_dpi(pi, tau)) /
               ((-pi * pi * d2gamma2_dpi2(pi, tau)) +
                Utility::pow<2>(pi * dgamma2_dpi(pi, tau) - tau * pi * d2gamma2_dpitau(pi, tau)) /
                    (tau * tau * d2gamma2_dtau2(pi, tau)));
      break;

    case 3:
    {
      // Calculate density first, then use that in Helmholtz free energy
      Real density3 = densityRegion3(pressure, temperature);
      delta = density3 / _rho_critical;
      tau = _T_star[2] / temperature;
      speed2 =
          _Rw * temperature *
          (2.0 * delta * dphi3_ddelta(delta, tau) + delta * delta * d2phi3_ddelta2(delta, tau) -
           Utility::pow<2>(delta * dphi3_ddelta(delta, tau) -
                           delta * tau * d2phi3_ddeltatau(delta, tau)) /
               (tau * tau * d2phi3_dtau2(delta, tau)));
      break;
    }

    case 5:
      pi = pressure / _p_star[4];
      tau = _T_star[4] / temperature;
      speed2 = _Rw * temperature * Utility::pow<2>(pi * dgamma5_dpi(pi, tau)) /
               ((-pi * pi * d2gamma5_dpi2(pi, tau)) +
                Utility::pow<2>(pi * dgamma5_dpi(pi, tau) - tau * pi * d2gamma5_dpitau(pi, tau)) /
                    (tau * tau * d2gamma5_dtau2(pi, tau)));
      break;

    default:
      mooseError(name(), ": inRegion() has given an incorrect region");
  }

  return std::sqrt(speed2);
}

Real
Water97FluidProperties::cp_from_p_T(Real pressure, Real temperature) const
{
  Real specific_heat, pi, tau, delta;

  // Determine which region the point is in
  unsigned int region = inRegion(pressure, temperature);
  switch (region)
  {
    case 1:
      pi = pressure / _p_star[0];
      tau = _T_star[0] / temperature;
      specific_heat = -_Rw * tau * tau * d2gamma1_dtau2(pi, tau);
      break;

    case 2:
      pi = pressure / _p_star[1];
      tau = _T_star[1] / temperature;
      specific_heat = -_Rw * tau * tau * d2gamma2_dtau2(pi, tau);
      break;

    case 3:
    {
      // Calculate density first, then use that in Helmholtz free energy
      Real density3 = densityRegion3(pressure, temperature);
      delta = density3 / _rho_critical;
      tau = _T_star[2] / temperature;
      specific_heat =
          _Rw *
          (-tau * tau * d2phi3_dtau2(delta, tau) +
           (delta * dphi3_ddelta(delta, tau) - delta * tau * d2phi3_ddeltatau(delta, tau)) *
               (delta * dphi3_ddelta(delta, tau) - delta * tau * d2phi3_ddeltatau(delta, tau)) /
               (2.0 * delta * dphi3_ddelta(delta, tau) +
                delta * delta * d2phi3_ddelta2(delta, tau)));
      break;
    }

    case 5:
      pi = pressure / _p_star[4];
      tau = _T_star[4] / temperature;
      specific_heat = -_Rw * tau * tau * d2gamma5_dtau2(pi, tau);
      break;

    default:
      mooseError(name(), ": inRegion() has given an incorrect region");
  }
  return specific_heat;
}

Real
Water97FluidProperties::cv_from_p_T(Real pressure, Real temperature) const
{
  Real specific_heat, pi, tau, delta;

  // Determine which region the point is in
  unsigned int region = inRegion(pressure, temperature);
  switch (region)
  {
    case 1:
      pi = pressure / _p_star[0];
      tau = _T_star[0] / temperature;
      specific_heat =
          _Rw * (-tau * tau * d2gamma1_dtau2(pi, tau) +
                 Utility::pow<2>(dgamma1_dpi(pi, tau) - tau * d2gamma1_dpitau(pi, tau)) /
                     d2gamma1_dpi2(pi, tau));
      break;

    case 2:
      pi = pressure / _p_star[1];
      tau = _T_star[1] / temperature;
      specific_heat =
          _Rw * (-tau * tau * d2gamma2_dtau2(pi, tau) +
                 Utility::pow<2>(dgamma2_dpi(pi, tau) - tau * d2gamma2_dpitau(pi, tau)) /
                     d2gamma2_dpi2(pi, tau));
      break;

    case 3:
    {
      // Calculate density first, then use that in Helmholtz free energy
      Real density3 = densityRegion3(pressure, temperature);
      delta = density3 / _rho_critical;
      tau = _T_star[2] / temperature;
      specific_heat = _Rw * (-tau * tau * d2phi3_dtau2(delta, tau));
      break;
    }

    case 5:
      pi = pressure / _p_star[4];
      tau = _T_star[4] / temperature;
      specific_heat =
          _Rw * (-tau * tau * d2gamma5_dtau2(pi, tau) +
                 Utility::pow<2>(dgamma5_dpi(pi, tau) - tau * d2gamma5_dpitau(pi, tau)) /
                     d2gamma5_dpi2(pi, tau));
      break;

    default:
      mooseError(name(), ": inRegion() has given an incorrect region");
  }
  return specific_heat;
}

Real
Water97FluidProperties::mu_from_p_T(Real pressure, Real temperature) const
{
  Real rho = this->rho_from_p_T(pressure, temperature);
  return this->mu_from_rho_T(rho, temperature);
}

void
Water97FluidProperties::mu_from_p_T(
    Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  Real rho, drho_dp, drho_dT;
  this->rho_from_p_T(pressure, temperature, rho, drho_dp, drho_dT);
  Real dmu_drho;
  this->mu_drhoT_from_rho_T(rho, temperature, drho_dT, mu, dmu_drho, dmu_dT);
  dmu_dp = dmu_drho * drho_dp;
}

Real
Water97FluidProperties::mu_from_rho_T(Real density, Real temperature) const
{
  Real mu_star = 1.e-6;
  Real rhobar = density / _rho_critical;
  Real Tbar = temperature / _T_critical;

  // Viscosity in limit of zero density
  Real sum0 = 0.0;
  for (std::size_t i = 0; i < _mu_H0.size(); ++i)
    sum0 += _mu_H0[i] / MathUtils::pow(Tbar, i);

  Real mu0 = 100.0 * std::sqrt(Tbar) / sum0;

  // Residual component due to finite density
  Real sum1 = 0.0;
  for (std::size_t i = 0; i < _mu_H1.size(); ++i)
    sum1 += MathUtils::pow(1.0 / Tbar - 1.0, _mu_I[i]) * _mu_H1[i] *
            MathUtils::pow(rhobar - 1.0, _mu_J[i]);

  Real mu1 = std::exp(rhobar * sum1);

  // The water viscosity (in Pa.s) is then given by
  return mu_star * mu0 * mu1;
}

void
Water97FluidProperties::mu_drhoT_from_rho_T(Real density,
                                            Real temperature,
                                            Real ddensity_dT,
                                            Real & mu,
                                            Real & dmu_drho,
                                            Real & dmu_dT) const
{
  Real mu_star = 1.0e-6;
  Real rhobar = density / _rho_critical;
  Real Tbar = temperature / _T_critical;
  Real drhobar_drho = 1.0 / _rho_critical;
  Real dTbar_dT = 1.0 / _T_critical;

  // Limit of zero density. Derivative wrt rho is 0
  Real sum0 = 0.0, dsum0_dTbar = 0.0;
  for (std::size_t i = 0; i < _mu_H0.size(); ++i)
  {
    sum0 += _mu_H0[i] / MathUtils::pow(Tbar, i);
    dsum0_dTbar -= i * _mu_H0[i] / MathUtils::pow(Tbar, i + 1);
  }

  Real mu0 = 100.0 * std::sqrt(Tbar) / sum0;
  Real dmu0_dTbar =
      50.0 / std::sqrt(Tbar) / sum0 - 100.0 * std::sqrt(Tbar) * dsum0_dTbar / sum0 / sum0;

  // Residual component due to finite density
  Real sum1 = 0.0, dsum1_drho = 0.0, dsum1_dTbar = 0.0;
  for (std::size_t i = 0; i < _mu_H1.size(); ++i)
  {
    sum1 += MathUtils::pow(1.0 / Tbar - 1.0, _mu_I[i]) * _mu_H1[i] *
            MathUtils::pow(rhobar - 1.0, _mu_J[i]);
    dsum1_drho += MathUtils::pow(1.0 / Tbar - 1.0, _mu_I[i]) * _mu_H1[i] * _mu_J[i] *
                  MathUtils::pow(rhobar - 1.0, _mu_J[i] - 1);
    dsum1_dTbar -= _mu_I[i] * MathUtils::pow(1.0 / Tbar - 1.0, _mu_I[i] - 1) * _mu_H1[i] *
                   MathUtils::pow(rhobar - 1.0, _mu_J[i]) / Tbar / Tbar;
  }

  Real mu1 = std::exp(rhobar * sum1);
  Real dmu1_drho = (sum1 + rhobar * dsum1_drho) * mu1;
  Real dmu1_dTbar = (rhobar * dsum1_dTbar) * mu1;

  // Viscosity and its derivatives are then
  mu = mu_star * mu0 * mu1;
  dmu_drho = mu_star * mu0 * dmu1_drho * drhobar_drho;
  dmu_dT = mu_star * (dmu0_dTbar * mu1 + mu0 * dmu1_dTbar) * dTbar_dT + dmu_drho * ddensity_dT;
}

void
Water97FluidProperties::rho_mu(Real pressure, Real temperature, Real & rho, Real & mu) const
{
  rho = this->rho_from_p_T(pressure, temperature);
  mu = this->mu_from_rho_T(rho, temperature);
}

void
Water97FluidProperties::rho_mu_dpT(Real pressure,
                                   Real temperature,
                                   Real & rho,
                                   Real & drho_dp,
                                   Real & drho_dT,
                                   Real & mu,
                                   Real & dmu_dp,
                                   Real & dmu_dT) const
{
  this->rho_from_p_T(pressure, temperature, rho, drho_dp, drho_dT);
  Real dmu_drho;
  this->mu_drhoT_from_rho_T(rho, temperature, drho_dT, mu, dmu_drho, dmu_dT);
  dmu_dp = dmu_drho * drho_dp;
}

Real
Water97FluidProperties::k_from_p_T(Real pressure, Real temperature) const
{
  Real rho = this->rho_from_p_T(pressure, temperature);
  return this->k_from_rho_T(rho, temperature);
}

void
Water97FluidProperties::k_from_p_T(
    Real /*pressure*/, Real /*temperature*/, Real & /*k*/, Real & /*dk_dp*/, Real & /*dk_dT*/) const
{
  mooseError(name(), "k_dpT() is not implemented");
}

Real
Water97FluidProperties::k_from_rho_T(Real density, Real temperature) const
{
  // Scale the density and temperature. Note that the scales are slightly
  // different to the critical values used in IAPWS-IF97
  Real Tbar = temperature / 647.26;
  Real rhobar = density / 317.7;

  // Ideal gas component
  Real sum0 = 0.0;

  for (std::size_t i = 0; i < _k_a.size(); ++i)
    sum0 += _k_a[i] * MathUtils::pow(Tbar, i);

  Real lambda0 = std::sqrt(Tbar) * sum0;

  // The contribution due to finite density
  Real lambda1 = -0.39707 + 0.400302 * rhobar +
                 1.06 * std::exp(-0.171587 * Utility::pow<2>(rhobar + 2.392190));

  // Critical enhancement
  Real DeltaT = std::abs(Tbar - 1.0) + 0.00308976;
  Real Q = 2.0 + 0.0822994 / std::pow(DeltaT, 0.6);
  Real S = (Tbar >= 1.0 ? 1.0 / DeltaT : 10.0932 / std::pow(DeltaT, 0.6));

  Real lambda2 =
      (0.0701309 / Utility::pow<10>(Tbar) + 0.011852) * std::pow(rhobar, 1.8) *
          std::exp(0.642857 * (1.0 - std::pow(rhobar, 2.8))) +
      0.00169937 * S * std::pow(rhobar, Q) *
          std::exp((Q / (1.0 + Q)) * (1.0 - std::pow(rhobar, 1.0 + Q))) -
      1.02 * std::exp(-4.11717 * std::pow(Tbar, 1.5) - 6.17937 / Utility::pow<5>(rhobar));

  return lambda0 + lambda1 + lambda2;
}

Real
Water97FluidProperties::s_from_p_T(Real pressure, Real temperature) const
{
  Real entropy, pi, tau, density3, delta;

  // Determine which region the point is in
  unsigned int region = inRegion(pressure, temperature);
  switch (region)
  {
    case 1:
      pi = pressure / _p_star[0];
      tau = _T_star[0] / temperature;
      entropy = _Rw * (tau * dgamma1_dtau(pi, tau) - gamma1(pi, tau));
      break;

    case 2:
      pi = pressure / _p_star[1];
      tau = _T_star[1] / temperature;
      entropy = _Rw * (tau * dgamma2_dtau(pi, tau) - gamma2(pi, tau));
      break;

    case 3:
      // Calculate density first, then use that in Helmholtz free energy
      density3 = densityRegion3(pressure, temperature);
      delta = density3 / _rho_critical;
      tau = _T_star[2] / temperature;
      entropy = _Rw * (tau * dphi3_dtau(delta, tau) - phi3(delta, tau));
      break;

    case 5:
      pi = pressure / _p_star[4];
      tau = _T_star[4] / temperature;
      entropy = _Rw * (tau * dgamma5_dtau(pi, tau) - gamma5(pi, tau));
      break;

    default:
      mooseError(name(), ": inRegion() has given an incorrect region");
  }
  return entropy;
}

void
Water97FluidProperties::s_from_p_T(Real p, Real T, Real & s, Real & ds_dp, Real & ds_dT) const
{
  SinglePhaseFluidProperties::s_from_p_T(p, T, s, ds_dp, ds_dT);
}

Real
Water97FluidProperties::h_from_p_T(Real pressure, Real temperature) const
{
  Real enthalpy, pi, tau, delta;

  // Determine which region the point is in
  unsigned int region = inRegion(pressure, temperature);
  switch (region)
  {
    case 1:
      pi = pressure / _p_star[0];
      tau = _T_star[0] / temperature;
      enthalpy = _Rw * _T_star[0] * dgamma1_dtau(pi, tau);
      break;

    case 2:
      pi = pressure / _p_star[1];
      tau = _T_star[1] / temperature;
      enthalpy = _Rw * _T_star[1] * dgamma2_dtau(pi, tau);
      break;

    case 3:
    {
      // Calculate density first, then use that in Helmholtz free energy
      Real density3 = densityRegion3(pressure, temperature);
      delta = density3 / _rho_critical;
      tau = _T_star[2] / temperature;
      enthalpy =
          _Rw * temperature * (tau * dphi3_dtau(delta, tau) + delta * dphi3_ddelta(delta, tau));
      break;
    }

    case 5:
      pi = pressure / _p_star[4];
      tau = _T_star[4] / temperature;
      enthalpy = _Rw * _T_star[4] * dgamma5_dtau(pi, tau);
      break;

    default:
      mooseError("Water97FluidProperties::inRegion has given an incorrect region");
  }
  return enthalpy;
}

void
Water97FluidProperties::h_from_p_T(
    Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const
{
  Real enthalpy, pi, tau, delta, denthalpy_dp, denthalpy_dT;

  // Determine which region the point is in
  unsigned int region = inRegion(pressure, temperature);
  switch (region)
  {
    case 1:
      pi = pressure / _p_star[0];
      tau = _T_star[0] / temperature;
      enthalpy = _Rw * _T_star[0] * dgamma1_dtau(pi, tau);
      denthalpy_dp = _Rw * _T_star[0] * d2gamma1_dpitau(pi, tau) / _p_star[0];
      denthalpy_dT = -_Rw * tau * tau * d2gamma1_dtau2(pi, tau);
      break;

    case 2:
      pi = pressure / _p_star[1];
      tau = _T_star[1] / temperature;
      enthalpy = _Rw * _T_star[1] * dgamma2_dtau(pi, tau);
      denthalpy_dp = _Rw * _T_star[1] * d2gamma2_dpitau(pi, tau) / _p_star[1];
      denthalpy_dT = -_Rw * tau * tau * d2gamma2_dtau2(pi, tau);
      break;

    case 3:
    {
      // Calculate density first, then use that in Helmholtz free energy
      Real density3 = densityRegion3(pressure, temperature);
      delta = density3 / _rho_critical;
      tau = _T_star[2] / temperature;
      Real dpdd = dphi3_ddelta(delta, tau);
      Real d2pddt = d2phi3_ddeltatau(delta, tau);
      Real d2pdd2 = d2phi3_ddelta2(delta, tau);
      enthalpy = _Rw * temperature * (tau * dphi3_dtau(delta, tau) + delta * dpdd);
      denthalpy_dp = (d2pddt + dpdd + delta * d2pdd2) / _rho_critical /
                     (2.0 * delta * dpdd + delta * delta * d2pdd2);
      denthalpy_dT = _Rw * delta * dpdd * (1.0 - tau * d2pddt / dpdd) *
                         (1.0 - tau * d2pddt / dpdd) / (2.0 + delta * d2pdd2 / dpdd) -
                     _Rw * tau * tau * d2phi3_dtau2(delta, tau);
      break;
    }

    case 5:
      pi = pressure / _p_star[4];
      tau = _T_star[4] / temperature;
      enthalpy = _Rw * _T_star[4] * dgamma5_dtau(pi, tau);
      denthalpy_dp = _Rw * _T_star[4] * d2gamma5_dpitau(pi, tau) / _p_star[4];
      denthalpy_dT = -_Rw * tau * tau * d2gamma5_dtau2(pi, tau);
      break;

    default:
      mooseError("Water97FluidProperties::inRegion has given an incorrect region");
  }
  h = enthalpy;
  dh_dp = denthalpy_dp;
  dh_dT = denthalpy_dT;
}

Real
Water97FluidProperties::vaporPressure(Real temperature) const
{
  // Check whether the input temperature is within the region of validity of this equation.
  // Valid for 273.15 K <= t <= 647.096 K
  if (temperature < 273.15 || temperature > _T_critical)
    throw MooseException(name() + ": vaporPressure(): Temperature is outside range 273.15 K <= T "
                                  "<= 647.096 K");

  Real theta, theta2, a, b, c, p;
  theta = temperature + _n4[8] / (temperature - _n4[9]);
  theta2 = theta * theta;
  a = theta2 + _n4[0] * theta + _n4[1];
  b = _n4[2] * theta2 + _n4[3] * theta + _n4[4];
  c = _n4[5] * theta2 + _n4[6] * theta + _n4[7];
  p = Utility::pow<4>(2.0 * c / (-b + std::sqrt(b * b - 4.0 * a * c)));

  return p * 1.e6;
}

void
Water97FluidProperties::vaporPressure_dT(Real temperature, Real & psat, Real & dpsat_dT) const
{
  // Check whether the input temperature is within the region of validity of this equation.
  // Valid for 273.15 K <= t <= 647.096 K
  if (temperature < 273.15 || temperature > _T_critical)
    throw MooseException(name() + ": vaporPressure_dT(): Temperature is outside range 273.15 K <= "
                                  "T <= 647.096 K");

  Real theta, dtheta_dT, theta2, a, b, c, da_dtheta, db_dtheta, dc_dtheta;
  theta = temperature + _n4[8] / (temperature - _n4[9]);
  dtheta_dT = 1.0 - _n4[8] / (temperature - _n4[9]) / (temperature - _n4[9]);
  theta2 = theta * theta;

  a = theta2 + _n4[0] * theta + _n4[1];
  b = _n4[2] * theta2 + _n4[3] * theta + _n4[4];
  c = _n4[5] * theta2 + _n4[6] * theta + _n4[7];

  da_dtheta = 2.0 * theta + _n4[0];
  db_dtheta = 2.0 * _n4[2] * theta + _n4[3];
  dc_dtheta = 2.0 * _n4[5] * theta + _n4[6];

  Real denominator = -b + std::sqrt(b * b - 4.0 * a * c);

  psat = Utility::pow<4>(2.0 * c / denominator) * 1.0e6;

  // The derivative wrt temperature is given by the chain rule
  Real dpsat = 4.0 * Utility::pow<3>(2.0 * c / denominator);
  dpsat *= (2.0 * dc_dtheta / denominator -
            2.0 * c / denominator / denominator *
                (-db_dtheta + std::pow(b * b - 4.0 * a * c, -0.5) *
                                  (b * db_dtheta - 2.0 * da_dtheta * c - 2.0 * a * dc_dtheta)));
  dpsat_dT = dpsat * dtheta_dT * 1.0e6;
}

Real
Water97FluidProperties::vaporTemperature(Real pressure) const
{
  // Check whether the input pressure is within the region of validity of this equation.
  // Valid for 611.213 Pa <= p <= 22.064 MPa
  if (pressure < 611.23 || pressure > _p_critical)
    throw MooseException(name() + ": vaporTemperature(): Pressure is outside range 611.213 Pa <= "
                                  "p <= 22.064 MPa");

  Real beta, beta2, e, f, g, d;
  beta = std::pow(pressure / 1.e6, 0.25);
  beta2 = beta * beta;
  e = beta2 + _n4[2] * beta + _n4[5];
  f = _n4[0] * beta2 + _n4[3] * beta + _n4[6];
  g = _n4[1] * beta2 + _n4[4] * beta + _n4[7];
  d = 2.0 * g / (-f - std::sqrt(f * f - 4.0 * e * g));

  return (_n4[9] + d - std::sqrt((_n4[9] + d) * (_n4[9] + d) - 4.0 * (_n4[8] + _n4[9] * d))) / 2.0;
}

Real
Water97FluidProperties::b23p(Real temperature) const
{
  // Check whether the input temperature is within the region of validity of this equation.
  // Valid for 623.15 K <= t <= 863.15 K
  if (temperature < 623.15 || temperature > 863.15)
    throw MooseException(name() +
                         ": b23p(): Temperature is outside range of 623.15 K <= T <= 863.15 K");

  return (_n23[0] + _n23[1] * temperature + _n23[2] * temperature * temperature) * 1.e6;
}

Real
Water97FluidProperties::b23T(Real pressure) const
{
  // Check whether the input pressure is within the region of validity of this equation.
  // Valid for 16.529 MPa <= p <= 100 MPa
  if (pressure < 16.529e6 || pressure > 100.0e6)
    throw MooseException(name() + ": b23T(): Pressure is outside range 16.529 MPa <= p <= 100 MPa");

  return _n23[3] + std::sqrt((pressure / 1.e6 - _n23[4]) / _n23[2]);
}

unsigned int
Water97FluidProperties::inRegion(Real pressure, Real temperature) const
{
  // Valid for 273.15 K <= T <= 1073.15 K, p <= 100 MPa
  //          1073.15 K <= T <= 2273.15 K, p <= 50 Mpa
  if (temperature >= 273.15 && temperature <= 1073.15)
  {
    if (pressure < vaporPressure(273.15) || pressure > 100.0e6)
      throw MooseException("Pressure " + Moose::stringify(pressure) + " is out of range in " +
                           name() + ": inRegion()");
  }
  else if (temperature > 1073.15 && temperature <= 2273.15)
  {
    if (pressure < 0.0 || pressure > 50.0e6)
      throw MooseException("Pressure " + Moose::stringify(pressure) + " is out of range in " +
                           name() + ": inRegion()");
  }
  else
    throw MooseException("Temperature " + Moose::stringify(temperature) + " is out of range in " +
                         name() + ": inRegion()");

  // Determine the phase region that the (P, T) point lies in
  unsigned int region;

  if (temperature >= 273.15 && temperature <= 623.15)
  {
    if (pressure > vaporPressure(temperature) && pressure <= 100.0e6)
      region = 1;
    else
      region = 2;
  }
  else if (temperature > 623.15 && temperature <= 863.15)
  {
    if (pressure <= b23p(temperature))
      region = 2;
    else
      region = 3;
  }
  else if (temperature > 863.15 && temperature <= 1073.15)
    region = 2;
  else
    region = 5;

  return region;
}

Real
Water97FluidProperties::gamma1(Real pi, Real tau) const
{
  Real sum = 0.0;
  for (std::size_t i = 0; i < _n1.size(); ++i)
    sum += _n1[i] * MathUtils::pow(7.1 - pi, _I1[i]) * MathUtils::pow(tau - 1.222, _J1[i]);

  return sum;
}

Real
Water97FluidProperties::dgamma1_dpi(Real pi, Real tau) const
{
  Real sum = 0.0;
  for (std::size_t i = 0; i < _n1.size(); ++i)
    sum += -_n1[i] * _I1[i] * MathUtils::pow(7.1 - pi, _I1[i] - 1) *
           MathUtils::pow(tau - 1.222, _J1[i]);

  return sum;
}

Real
Water97FluidProperties::d2gamma1_dpi2(Real pi, Real tau) const
{
  Real sum = 0.0;
  for (std::size_t i = 0; i < _n1.size(); ++i)
    sum += _n1[i] * _I1[i] * (_I1[i] - 1) * MathUtils::pow(7.1 - pi, _I1[i] - 2) *
           MathUtils::pow(tau - 1.222, _J1[i]);

  return sum;
}

Real
Water97FluidProperties::dgamma1_dtau(Real pi, Real tau) const
{
  Real g = 0.0;
  for (std::size_t i = 0; i < _n1.size(); ++i)
    g += _n1[i] * _J1[i] * MathUtils::pow(7.1 - pi, _I1[i]) *
         MathUtils::pow(tau - 1.222, _J1[i] - 1);

  return g;
}

Real
Water97FluidProperties::d2gamma1_dtau2(Real pi, Real tau) const
{
  Real dg = 0.0;
  for (std::size_t i = 0; i < _n1.size(); ++i)
    dg += _n1[i] * _J1[i] * (_J1[i] - 1) * MathUtils::pow(7.1 - pi, _I1[i]) *
          MathUtils::pow(tau - 1.222, _J1[i] - 2);

  return dg;
}

Real
Water97FluidProperties::d2gamma1_dpitau(Real pi, Real tau) const
{
  Real dg = 0.0;
  for (std::size_t i = 0; i < _n1.size(); ++i)
    dg += -_n1[i] * _I1[i] * _J1[i] * MathUtils::pow(7.1 - pi, _I1[i] - 1) *
          MathUtils::pow(tau - 1.222, _J1[i] - 1);

  return dg;
}

Real
Water97FluidProperties::gamma2(Real pi, Real tau) const
{
  // Ideal gas part of the Gibbs free energy
  Real sum0 = 0.0;
  for (std::size_t i = 0; i < _n02.size(); ++i)
    sum0 += _n02[i] * MathUtils::pow(tau, _J02[i]);

  Real g0 = std::log(pi) + sum0;

  // Residual part of the Gibbs free energy
  Real gr = 0.0;
  for (std::size_t i = 0; i < _n2.size(); ++i)
    gr += _n2[i] * MathUtils::pow(pi, _I2[i]) * MathUtils::pow(tau - 0.5, _J2[i]);

  return g0 + gr;
}

Real
Water97FluidProperties::dgamma2_dpi(Real pi, Real tau) const
{
  // Ideal gas part of the Gibbs free energy
  Real dg0 = 1.0 / pi;

  // Residual part of the Gibbs free energy
  Real dgr = 0.0;
  for (std::size_t i = 0; i < _n2.size(); ++i)
    dgr += _n2[i] * _I2[i] * MathUtils::pow(pi, _I2[i] - 1) * MathUtils::pow(tau - 0.5, _J2[i]);

  return dg0 + dgr;
}

Real
Water97FluidProperties::d2gamma2_dpi2(Real pi, Real tau) const
{
  // Ideal gas part of the Gibbs free energy
  Real dg0 = -1.0 / pi / pi;

  // Residual part of the Gibbs free energy
  Real dgr = 0.0;
  for (std::size_t i = 0; i < _n2.size(); ++i)
    dgr += _n2[i] * _I2[i] * (_I2[i] - 1) * MathUtils::pow(pi, _I2[i] - 2) *
           MathUtils::pow(tau - 0.5, _J2[i]);

  return dg0 + dgr;
}

Real
Water97FluidProperties::dgamma2_dtau(Real pi, Real tau) const
{
  // Ideal gas part of the Gibbs free energy
  Real dg0 = 0.0;
  for (std::size_t i = 0; i < _n02.size(); ++i)
    dg0 += _n02[i] * _J02[i] * MathUtils::pow(tau, _J02[i] - 1);

  // Residual part of the Gibbs free energy
  Real dgr = 0.0;
  for (std::size_t i = 0; i < _n2.size(); ++i)
    dgr += _n2[i] * _J2[i] * MathUtils::pow(pi, _I2[i]) * MathUtils::pow(tau - 0.5, _J2[i] - 1);

  return dg0 + dgr;
}

Real
Water97FluidProperties::d2gamma2_dtau2(Real pi, Real tau) const
{
  // Ideal gas part of the Gibbs free energy
  Real dg0 = 0.0;
  for (std::size_t i = 0; i < _n02.size(); ++i)
    dg0 += _n02[i] * _J02[i] * (_J02[i] - 1) * MathUtils::pow(tau, _J02[i] - 2);

  // Residual part of the Gibbs free energy
  Real dgr = 0.0;
  for (std::size_t i = 0; i < _n2.size(); ++i)
    dgr += _n2[i] * _J2[i] * (_J2[i] - 1) * MathUtils::pow(pi, _I2[i]) *
           MathUtils::pow(tau - 0.5, _J2[i] - 2);

  return dg0 + dgr;
}

Real
Water97FluidProperties::d2gamma2_dpitau(Real pi, Real tau) const
{
  // Ideal gas part of the Gibbs free energy
  Real dg0 = 0.0;

  // Residual part of the Gibbs free energy
  Real dgr = 0.0;
  for (std::size_t i = 0; i < _n2.size(); ++i)
    dgr += _n2[i] * _I2[i] * _J2[i] * MathUtils::pow(pi, _I2[i] - 1) *
           MathUtils::pow(tau - 0.5, _J2[i] - 1);

  return dg0 + dgr;
}

Real
Water97FluidProperties::phi3(Real delta, Real tau) const
{
  Real sum = 0.0;
  for (std::size_t i = 1; i < _n3.size(); ++i)
    sum += _n3[i] * MathUtils::pow(delta, _I3[i]) * MathUtils::pow(tau, _J3[i]);

  return _n3[0] * std::log(delta) + sum;
}

Real
Water97FluidProperties::dphi3_ddelta(Real delta, Real tau) const
{
  Real sum = 0.0;
  for (std::size_t i = 1; i < _n3.size(); ++i)
    sum += _n3[i] * _I3[i] * MathUtils::pow(delta, _I3[i] - 1) * MathUtils::pow(tau, _J3[i]);

  return _n3[0] / delta + sum;
}

Real
Water97FluidProperties::d2phi3_ddelta2(Real delta, Real tau) const
{
  Real sum = 0.0;
  for (std::size_t i = 1; i < _n3.size(); ++i)
    sum += _n3[i] * _I3[i] * (_I3[i] - 1) * MathUtils::pow(delta, _I3[i] - 2) *
           MathUtils::pow(tau, _J3[i]);

  return -_n3[0] / delta / delta + sum;
}

Real
Water97FluidProperties::dphi3_dtau(Real delta, Real tau) const
{
  Real sum = 0.0;
  for (std::size_t i = 1; i < _n3.size(); ++i)
    sum += _n3[i] * _J3[i] * MathUtils::pow(delta, _I3[i]) * MathUtils::pow(tau, _J3[i] - 1);

  return sum;
}

Real
Water97FluidProperties::d2phi3_dtau2(Real delta, Real tau) const
{
  Real sum = 0.0;
  for (std::size_t i = 1; i < _n3.size(); ++i)
    sum += _n3[i] * _J3[i] * (_J3[i] - 1) * MathUtils::pow(delta, _I3[i]) *
           MathUtils::pow(tau, _J3[i] - 2);

  return sum;
}

Real
Water97FluidProperties::d2phi3_ddeltatau(Real delta, Real tau) const
{
  Real sum = 0.0;
  for (std::size_t i = 1; i < _n3.size(); ++i)
    sum += _n3[i] * _I3[i] * _J3[i] * MathUtils::pow(delta, _I3[i] - 1) *
           MathUtils::pow(tau, _J3[i] - 1);

  return sum;
}

Real
Water97FluidProperties::gamma5(Real pi, Real tau) const
{
  // Ideal gas part of the Gibbs free energy
  Real sum0 = 0.0;
  for (std::size_t i = 0; i < _n05.size(); ++i)
    sum0 += _n05[i] * MathUtils::pow(tau, _J05[i]);

  Real g0 = std::log(pi) + sum0;

  // Residual part of the Gibbs free energy
  Real gr = 0.0;
  for (std::size_t i = 0; i < _n5.size(); ++i)
    gr += _n5[i] * MathUtils::pow(pi, _I5[i]) * MathUtils::pow(tau, _J5[i]);

  return g0 + gr;
}

Real
Water97FluidProperties::dgamma5_dpi(Real pi, Real tau) const
{
  // Ideal gas part of the Gibbs free energy
  Real dg0 = 1.0 / pi;

  // Residual part of the Gibbs free energy
  Real dgr = 0.0;
  for (std::size_t i = 0; i < _n5.size(); ++i)
    dgr += _n5[i] * _I5[i] * MathUtils::pow(pi, _I5[i] - 1) * MathUtils::pow(tau, _J5[i]);

  return dg0 + dgr;
}

Real
Water97FluidProperties::d2gamma5_dpi2(Real pi, Real tau) const
{
  // Ideal gas part of the Gibbs free energy
  Real dg0 = -1.0 / pi / pi;

  // Residual part of the Gibbs free energy
  Real dgr = 0.0;
  for (std::size_t i = 0; i < _n5.size(); ++i)
    dgr += _n5[i] * _I5[i] * (_I5[i] - 1) * MathUtils::pow(pi, _I5[i] - 2) *
           MathUtils::pow(tau, _J5[i]);

  return dg0 + dgr;
}

Real
Water97FluidProperties::dgamma5_dtau(Real pi, Real tau) const
{
  // Ideal gas part of the Gibbs free energy
  Real dg0 = 0.0;
  for (std::size_t i = 0; i < _n05.size(); ++i)
    dg0 += _n05[i] * _J05[i] * MathUtils::pow(tau, _J05[i] - 1);

  // Residual part of the Gibbs free energy
  Real dgr = 0.0;
  for (std::size_t i = 0; i < _n5.size(); ++i)
    dgr += _n5[i] * _J5[i] * MathUtils::pow(pi, _I5[i]) * MathUtils::pow(tau, _J5[i] - 1);

  return dg0 + dgr;
}

Real
Water97FluidProperties::d2gamma5_dtau2(Real pi, Real tau) const
{
  // Ideal gas part of the Gibbs free energy
  Real dg0 = 0.0;
  for (std::size_t i = 0; i < _n05.size(); ++i)
    dg0 += _n05[i] * _J05[i] * (_J05[i] - 1) * MathUtils::pow(tau, _J05[i] - 2);

  // Residual part of the Gibbs free energy
  Real dgr = 0.0;
  for (std::size_t i = 0; i < _n5.size(); ++i)
    dgr += _n5[i] * _J5[i] * (_J5[i] - 1) * MathUtils::pow(pi, _I5[i]) *
           MathUtils::pow(tau, _J5[i] - 2);

  return dg0 + dgr;
}

Real
Water97FluidProperties::d2gamma5_dpitau(Real pi, Real tau) const
{
  // Ideal gas part of the Gibbs free energy
  Real dg0 = 0.0;

  // Residual part of the Gibbs free energy
  Real dgr = 0.0;
  for (std::size_t i = 0; i < _n5.size(); ++i)
    dgr +=
        _n5[i] * _I5[i] * _J5[i] * MathUtils::pow(pi, _I5[i] - 1) * MathUtils::pow(tau, _J5[i] - 1);

  return dg0 + dgr;
}

unsigned int
Water97FluidProperties::subregion3(Real pressure, Real temperature) const
{
  Real pMPa = pressure / 1.0e6;
  const Real P3cd = 19.00881189173929;
  unsigned int subregion = 0;

  if (pMPa > 40.0 && pMPa <= 100.0)
  {
    if (temperature <= tempXY(pressure, AB))
      subregion = 0;
    else // (temperature > tempXY(pressure, AB))
      subregion = 1;
  }
  else if (pMPa > 25.0 && pMPa <= 40.0)
  {
    if (temperature <= tempXY(pressure, CD))
      subregion = 2;
    else if (temperature > tempXY(pressure, CD) && temperature <= tempXY(pressure, AB))
      subregion = 3;
    else if (temperature > tempXY(pressure, AB) && temperature <= tempXY(pressure, EF))
      subregion = 4;
    else // (temperature > tempXY(pressure, EF))
      subregion = 5;
  }
  else if (pMPa > 23.5 && pMPa <= 25.0)
  {
    if (temperature <= tempXY(pressure, CD))
      subregion = 2;
    else if (temperature > tempXY(pressure, CD) && temperature <= tempXY(pressure, GH))
      subregion = 6;
    else if (temperature > tempXY(pressure, GH) && temperature <= tempXY(pressure, EF))
      subregion = 7;
    else if (temperature > tempXY(pressure, EF) && temperature <= tempXY(pressure, IJ))
      subregion = 8;
    else if (temperature > tempXY(pressure, IJ) && temperature <= tempXY(pressure, JK))
      subregion = 9;
    else // (temperature > tempXY(pressure, JK))
      subregion = 10;
  }
  else if (pMPa > 23.0 && pMPa <= 23.5)
  {
    if (temperature <= tempXY(pressure, CD))
      subregion = 2;
    else if (temperature > tempXY(pressure, CD) && temperature <= tempXY(pressure, GH))
      subregion = 11;
    else if (temperature > tempXY(pressure, GH) && temperature <= tempXY(pressure, EF))
      subregion = 7;
    else if (temperature > tempXY(pressure, EF) && temperature <= tempXY(pressure, IJ))
      subregion = 8;
    else if (temperature > tempXY(pressure, IJ) && temperature <= tempXY(pressure, JK))
      subregion = 9;
    else // (temperature > tempXY(pressure, JK))
      subregion = 10;
  }
  else if (pMPa > 22.5 && pMPa <= 23.0)
  {
    if (temperature <= tempXY(pressure, CD))
      subregion = 2;
    else if (temperature > tempXY(pressure, CD) && temperature <= tempXY(pressure, GH))
      subregion = 11;
    else if (temperature > tempXY(pressure, GH) && temperature <= tempXY(pressure, MN))
      subregion = 12;
    else if (temperature > tempXY(pressure, MN) && temperature <= tempXY(pressure, EF))
      subregion = 13;
    else if (temperature > tempXY(pressure, EF) && temperature <= tempXY(pressure, OP))
      subregion = 14;
    else if (temperature > tempXY(pressure, OP) && temperature <= tempXY(pressure, IJ))
      subregion = 15;
    else if (temperature > tempXY(pressure, IJ) && temperature <= tempXY(pressure, JK))
      subregion = 9;
    else // (temperature > tempXY(pressure, JK))
      subregion = 10;
  }
  else if (pMPa > vaporPressure(643.15) * 1.0e-6 &&
           pMPa <= 22.5) // vaporPressure(643.15) = 21.04 MPa
  {
    if (temperature <= tempXY(pressure, CD))
      subregion = 2;
    else if (temperature > tempXY(pressure, CD) && temperature <= tempXY(pressure, QU))
      subregion = 16;
    else if (temperature > tempXY(pressure, QU) && temperature <= tempXY(pressure, RX))
    {
      if (pMPa > 22.11 && pMPa <= 22.5)
      {
        if (temperature <= tempXY(pressure, UV))
          subregion = 20;
        else if (temperature > tempXY(pressure, UV) && temperature <= tempXY(pressure, EF))
          subregion = 21;
        else if (temperature > tempXY(pressure, EF) && temperature <= tempXY(pressure, WX))
          subregion = 22;
        else // (temperature > tempXY(pressure, WX) && temperature <= tempXY(pressure, RX))
          subregion = 23;
      }
      else if (pMPa > 22.064 && pMPa <= 22.11)
      {
        if (temperature <= tempXY(pressure, UV))
          subregion = 20;
        else if (temperature > tempXY(pressure, UV) && temperature <= tempXY(pressure, EF))
          subregion = 24;
        else if (temperature > tempXY(pressure, EF) && temperature <= tempXY(pressure, WX))
          subregion = 25;
        else // (temperature > tempXY(pressure, WX) && temperature <= tempXY(pressure, RX))
          subregion = 23;
      }
      else if (temperature <= vaporTemperature(pressure))
      {
        if (pMPa > 21.93161551 && pMPa <= 22.064)
          if (temperature > tempXY(pressure, QU) && temperature <= tempXY(pressure, UV))
            subregion = 20;
          else
            subregion = 24;
        else // (pMPa > vaporPressure(643.15) * 1.0e-6 && pMPa <= 21.93161551)
          subregion = 20;
      }
      else if (temperature > vaporTemperature(pressure))
      {
        if (pMPa > 21.90096265 && pMPa <= 22.064)
        {
          if (temperature <= tempXY(pressure, WX))
            subregion = 25;
          else
            subregion = 23;
        }
        else
          subregion = 23;
      }
    }
    else if (temperature > tempXY(pressure, RX) && temperature <= tempXY(pressure, JK))
      subregion = 17;
    else
      subregion = 10;
  }
  else if (pMPa > 20.5 &&
           pMPa <= vaporPressure(643.15) * 1.0e-6) // vaporPressure(643.15) = 21.04 MPa
  {
    if (temperature <= tempXY(pressure, CD))
      subregion = 2;
    else if (temperature > tempXY(pressure, CD) && temperature <= vaporTemperature(pressure))
      subregion = 16;
    else if (temperature > vaporTemperature(pressure) && temperature <= tempXY(pressure, JK))
      subregion = 17;
    else // (temperature > tempXY(pressure, JK))
      subregion = 10;
  }
  else if (pMPa > P3cd && pMPa <= 20.5) // P3cd  = 19.00881189173929
  {
    if (temperature <= tempXY(pressure, CD))
      subregion = 2;
    else if (temperature > tempXY(pressure, CD) && temperature <= vaporTemperature(pressure))
      subregion = 18;
    else
      subregion = 19;
  }
  else if (pMPa > vaporPressure(623.15) * 1.0e-6 && pMPa <= P3cd)
  {
    if (temperature < vaporTemperature(pressure))
      subregion = 2;
    else
      subregion = 19;
  }
  else if (pMPa > 22.11 && pMPa <= 22.5)
  {
    if (temperature > tempXY(pressure, QU) && temperature <= tempXY(pressure, UV))
      subregion = 20;
    else if (temperature > tempXY(pressure, UV) && temperature <= tempXY(pressure, EF))
      subregion = 21;
    else if (temperature > tempXY(pressure, EF) && temperature <= tempXY(pressure, WX))
      subregion = 22;
    else // (temperature > tempXY(pressure, WX) && temperature <= tempXY(pressure, RX))
      subregion = 23;
  }
  else if (pMPa > 22.064 && pMPa <= 22.11)
  {
    if (temperature > tempXY(pressure, QU) && temperature <= tempXY(pressure, UV))
      subregion = 20;
    else if (temperature > tempXY(pressure, UV) && temperature <= tempXY(pressure, EF))
      subregion = 24;
    else if (temperature > tempXY(pressure, EF) && temperature <= tempXY(pressure, WX))
      subregion = 25;
    else // (temperature > tempXY(pressure, WX) && temperature <= tempXY(pressure, RX))
      subregion = 23;
  }
  else
    mooseError(name(), ": subregion3(): Shouldn't have got here!");

  return subregion;
}

Real
Water97FluidProperties::tempXY(Real pressure, subregionEnum xy) const
{
  Real pi = pressure / 1.0e6;

  // Choose the constants based on the string xy
  unsigned int row;

  switch (xy)
  {
    case AB:
      row = 0;
      break;
    case CD:
      row = 1;
      break;
    case GH:
      row = 2;
      break;
    case IJ:
      row = 3;
      break;
    case JK:
      row = 4;
      break;
    case MN:
      row = 5;
      break;
    case OP:
      row = 6;
      break;
    case QU:
      row = 7;
      break;
    case RX:
      row = 8;
      break;
    case UV:
      row = 9;
      break;
    case WX:
      row = 10;
      break;
    default:
      row = 0;
  }

  Real sum = 0.0;

  if (xy == AB || xy == OP || xy == WX)
    for (std::size_t i = 0; i < _tempXY_n[row].size(); ++i)
      sum += _tempXY_n[row][i] * MathUtils::pow(std::log(pi), _tempXY_I[row][i]);
  else if (xy == EF)
    sum += 3.727888004 * (pi - _p_critical / 1.0e6) + _T_critical;
  else
    for (std::size_t i = 0; i < _tempXY_n[row].size(); ++i)
      sum += _tempXY_n[row][i] * MathUtils::pow(pi, _tempXY_I[row][i]);

  return sum;
}

Real
Water97FluidProperties::subregionVolume(
    Real pi, Real theta, Real a, Real b, Real c, Real d, Real e, unsigned int sid) const
{
  Real sum = 0.0;

  for (std::size_t i = 0; i < _n3s[sid].size(); ++i)
    sum += _n3s[sid][i] * MathUtils::pow(std::pow(pi - a, c), _I3s[sid][i]) *
           MathUtils::pow(std::pow(theta - b, d), _J3s[sid][i]);

  return std::pow(sum, e);
}

Real
Water97FluidProperties::densityRegion3(Real pressure, Real temperature) const
{
  // Region 3 is subdivided into 26 subregions, each with a given backwards equation
  // to directly calculate density from pressure and temperature without the need for
  // expensive iterations. Find the subregion id that the point is in:
  unsigned int sid = subregion3(pressure, temperature);

  Real vstar, pi, theta, a, b, c, d, e;
  unsigned int N;

  vstar = _par3[sid][0];
  pi = pressure / _par3[sid][1] / 1.0e6;
  theta = temperature / _par3[sid][2];
  a = _par3[sid][3];
  b = _par3[sid][4];
  c = _par3[sid][5];
  d = _par3[sid][6];
  e = _par3[sid][7];
  N = _par3N[sid];

  Real sum = 0.0;
  Real volume = 0.0;

  // Note that subregion 13 is the only different formulation
  if (sid == 13)
  {
    for (std::size_t i = 0; i < N; ++i)
      sum += _n3s[sid][i] * MathUtils::pow(pi - a, _I3s[sid][i]) *
             MathUtils::pow(theta - b, _J3s[sid][i]);

    volume = vstar * std::exp(sum);
  }
  else
    volume = vstar * subregionVolume(pi, theta, a, b, c, d, e, sid);

  // Density is the inverse of volume
  return 1.0 / volume;
}

unsigned int
Water97FluidProperties::inRegionPH(Real pressure, Real enthalpy) const
{
  unsigned int region;

  // Need to calculate enthalpies at the boundaries to delineate regions
  Real p273 = vaporPressure(273.15);
  Real p623 = vaporPressure(623.15);

  if (pressure >= p273 && pressure <= p623)
  {
    if (enthalpy >= h_from_p_T(pressure, 273.15) &&
        enthalpy <= h_from_p_T(pressure, vaporTemperature(pressure)))
      region = 1;
    else if (enthalpy > h_from_p_T(pressure, vaporTemperature(pressure)) &&
             enthalpy <= h_from_p_T(pressure, 1073.15))
      region = 2;
    else if (enthalpy > h_from_p_T(pressure, 1073.15) && enthalpy <= h_from_p_T(pressure, 2273.15))
      region = 5;
    else
      throw MooseException("Enthalpy " + Moose::stringify(enthalpy) + " is out of range in " +
                           name() + ": inRegionPH()");
  }
  else if (pressure > p623 && pressure <= 50.0e6)
  {
    if (enthalpy >= h_from_p_T(pressure, 273.15) && enthalpy <= h_from_p_T(pressure, 623.15))
      region = 1;
    else if (enthalpy > h_from_p_T(pressure, 623.15) &&
             enthalpy <= h_from_p_T(pressure, b23T(pressure)))
      region = 3;
    else if (enthalpy > h_from_p_T(pressure, b23T(pressure)) &&
             enthalpy <= h_from_p_T(pressure, 1073.15))
      region = 2;
    else if (enthalpy > h_from_p_T(pressure, 1073.15) && enthalpy <= h_from_p_T(pressure, 2273.15))
      region = 5;
    else
      throw MooseException("Enthalpy " + Moose::stringify(enthalpy) + " is out of range in " +
                           name() + ": inRegionPH()");
  }
  else if (pressure > 50.0e6 && pressure <= 100.0e6)
  {
    if (enthalpy >= h_from_p_T(pressure, 273.15) && enthalpy <= h_from_p_T(pressure, 623.15))
      region = 1;
    else if (enthalpy > h_from_p_T(pressure, 623.15) &&
             enthalpy <= h_from_p_T(pressure, b23T(pressure)))
      region = 3;
    else if (enthalpy > h_from_p_T(pressure, b23T(pressure)) &&
             enthalpy <= h_from_p_T(pressure, 1073.15))
      region = 2;
    else
      throw MooseException("Enthalpy " + Moose::stringify(enthalpy) + " is out of range in " +
                           name() + ": inRegionPH()");
  }
  else
    throw MooseException("Pressure " + Moose::stringify(pressure) + " is out of range in " +
                         name() + ": inRegionPH()");

  return region;
}

unsigned int
Water97FluidProperties::subregion2ph(Real pressure, Real enthalpy) const
{
  unsigned int subregion;

  if (pressure <= 4.0e6)
    subregion = 1;
  else if (pressure > 4.0e6 && pressure < 6.5467e6)
    subregion = 2;
  else
  {
    if (enthalpy >= b2bc(pressure))
      subregion = 2;
    else
      subregion = 3;
  }

  return subregion;
}

unsigned int
Water97FluidProperties::subregion3ph(Real pressure, Real enthalpy) const
{
  unsigned int subregion;

  if (enthalpy <= b3ab(pressure))
    subregion = 1;
  else
    subregion = 2;

  return subregion;
}

Real
Water97FluidProperties::b2bc(Real pressure) const
{
  // Check whether the input pressure is within the region of validity of this equation.
  if (pressure < 6.5467e6 || pressure > 100.0e6)
    throw MooseException(name() +
                         ": b2bc(): Pressure is outside range of 6.5467 MPa <= p <= 100 MPa");

  Real pi = pressure / 1.0e6;

  return (0.26526571908428e4 + std::sqrt((pi - 0.45257578905948e1) / 0.12809002730136e-3)) * 1.0e3;
}

Real
Water97FluidProperties::temperature_from_ph(Real pressure, Real enthalpy) const
{
  Real temperature = 0.0;

  // Determine which region the point is in
  unsigned int region = inRegionPH(pressure, enthalpy);

  switch (region)
  {
    case 1:
      temperature = temperature_from_ph1(pressure, enthalpy);
      break;

    case 2:
    {
      // First, determine which subregion the point is in:
      unsigned int subregion = subregion2ph(pressure, enthalpy);

      if (subregion == 1)
        temperature = temperature_from_ph2a(pressure, enthalpy);
      else if (subregion == 2)
        temperature = temperature_from_ph2b(pressure, enthalpy);
      else
        temperature = temperature_from_ph2c(pressure, enthalpy);
      break;
    }

    case 3:
    {
      // First, determine which subregion the point is in:
      unsigned int subregion = subregion3ph(pressure, enthalpy);

      if (subregion == 1)
        temperature = temperature_from_ph3a(pressure, enthalpy);
      else
        temperature = temperature_from_ph3b(pressure, enthalpy);
      break;
    }

    case 5:
      mooseError(name(), ": temperature_from_ph() not implemented for region 5");
      break;

    default:
      mooseError(name(), ": inRegionPH() has given an incorrect region");
  }

  return temperature;
}

Real
Water97FluidProperties::temperature_from_ph1(Real pressure, Real enthalpy) const
{
  Real pi = pressure / 1.0e6;
  Real eta = enthalpy / 2500.0e3;
  Real sum = 0.0;

  for (std::size_t i = 0; i < _nph1.size(); ++i)
    sum += _nph1[i] * MathUtils::pow(pi, _Iph1[i]) * MathUtils::pow(eta + 1.0, _Jph1[i]);

  return sum;
}

Real
Water97FluidProperties::temperature_from_ph2a(Real pressure, Real enthalpy) const
{
  Real pi = pressure / 1.0e6;
  Real eta = enthalpy / 2000.0e3;
  Real sum = 0.0;

  for (std::size_t i = 0; i < _nph2a.size(); ++i)
    sum += _nph2a[i] * MathUtils::pow(pi, _Iph2a[i]) * MathUtils::pow(eta - 2.1, _Jph2a[i]);

  return sum;
}

Real
Water97FluidProperties::temperature_from_ph2b(Real pressure, Real enthalpy) const
{
  Real pi = pressure / 1.0e6;
  Real eta = enthalpy / 2000.0e3;
  Real sum = 0.0;

  for (std::size_t i = 0; i < _nph2b.size(); ++i)
    sum += _nph2b[i] * MathUtils::pow(pi - 2.0, _Iph2b[i]) * MathUtils::pow(eta - 2.6, _Jph2b[i]);

  return sum;
}

Real
Water97FluidProperties::temperature_from_ph2c(Real pressure, Real enthalpy) const
{
  Real pi = pressure / 1.0e6;
  Real eta = enthalpy / 2000.0e3;
  Real sum = 0.0;

  for (std::size_t i = 0; i < _nph2c.size(); ++i)
    sum += _nph2c[i] * MathUtils::pow(pi + 25.0, _Iph2c[i]) * MathUtils::pow(eta - 1.8, _Jph2c[i]);

  return sum;
}

Real
Water97FluidProperties::b3ab(Real pressure) const
{
  // Check whether the input pressure is within the region of validity of this equation.
  if (pressure < b23p(623.15) || pressure > 100.0e6)
    throw MooseException(name() +
                         ": b3ab(): Pressure is outside range of 16.529 MPa <= p <= 100 MPa");

  Real pi = pressure / 1.0e6;
  Real eta = 0.201464004206875e4 + 0.374696550136983e1 * pi - 0.219921901054187e-1 * pi * pi +
             0.87513168600995e-4 * pi * pi * pi;

  return eta * 1.0e3;
}

Real
Water97FluidProperties::temperature_from_ph3a(Real pressure, Real enthalpy) const
{
  Real pi = pressure / 100.0e6;
  Real eta = enthalpy / 2300.0e3;
  Real sum = 0.0;

  for (std::size_t i = 0; i < _nph3a.size(); ++i)
    sum +=
        _nph3a[i] * MathUtils::pow(pi + 0.24, _Iph3a[i]) * MathUtils::pow(eta - 0.615, _Jph3a[i]);

  return sum * 760.0;
}

Real
Water97FluidProperties::temperature_from_ph3b(Real pressure, Real enthalpy) const
{
  Real pi = pressure / 100.0e6;
  Real eta = enthalpy / 2800.0e3;
  Real sum = 0.0;

  for (std::size_t i = 0; i < _nph3b.size(); ++i)
    sum +=
        _nph3b[i] * MathUtils::pow(pi + 0.298, _Iph3b[i]) * MathUtils::pow(eta - 0.72, _Jph3b[i]);

  return sum * 860.0;
}
