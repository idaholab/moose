/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "Water97FluidProperties.h"

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
Water97FluidProperties::rho(Real pressure, Real temperature) const
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
      mooseError("Water97FluidProperties::inRegion has given an incorrect region");
  }
  return density;
}

void
Water97FluidProperties::rho_dpT(
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
      mooseError("Water97FluidProperties::inRegion has given an incorrect region");
  }

  rho = this->rho(pressure, temperature);
  drho_dp = ddensity_dp;
  drho_dT = ddensity_dT;
}

Real
Water97FluidProperties::e(Real pressure, Real temperature) const
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
      mooseError("Water97FluidProperties::inRegion has given an incorrect region");
  }
  // Output in J/kg
  return internal_energy;
}

void
Water97FluidProperties::e_dpT(
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
      mooseError("Water97FluidProperties::inRegion has given an incorrect region");
  }

  e = this->e(pressure, temperature);
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
  rho_dpT(pressure, temperature, rho, drho_dp, drho_dT);
  e_dpT(pressure, temperature, e, de_dp, de_dT);
}

Real
Water97FluidProperties::c(Real pressure, Real temperature) const
{
  Real speed2, pi, tau, delta;

  // Determine which region the point is in
  unsigned int region = inRegion(pressure, temperature);
  switch (region)
  {
    case 1:
      pi = pressure / _p_star[0];
      tau = _T_star[0] / temperature;
      speed2 = _Rw * temperature * std::pow(dgamma1_dpi(pi, tau), 2.0) /
               (std::pow(dgamma1_dpi(pi, tau) - tau * d2gamma1_dpitau(pi, tau), 2.0) /
                    (tau * tau * d2gamma1_dtau2(pi, tau)) -
                d2gamma1_dpi2(pi, tau));
      break;

    case 2:
      pi = pressure / _p_star[1];
      tau = _T_star[1] / temperature;
      speed2 = _Rw * temperature * std::pow(pi * dgamma2_dpi(pi, tau), 2.0) /
               ((-pi * pi * d2gamma2_dpi2(pi, tau)) +
                std::pow(pi * dgamma2_dpi(pi, tau) - tau * pi * d2gamma2_dpitau(pi, tau), 2.0) /
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
           std::pow(delta * dphi3_ddelta(delta, tau) - delta * tau * d2phi3_ddeltatau(delta, tau),
                    2.0) /
               (tau * tau * d2phi3_dtau2(delta, tau)));
      break;
    }

    case 5:
      pi = pressure / _p_star[4];
      tau = _T_star[4] / temperature;
      speed2 = _Rw * temperature * std::pow(pi * dgamma5_dpi(pi, tau), 2.0) /
               ((-pi * pi * d2gamma5_dpi2(pi, tau)) +
                std::pow(pi * dgamma5_dpi(pi, tau) - tau * pi * d2gamma5_dpitau(pi, tau), 2.0) /
                    (tau * tau * d2gamma5_dtau2(pi, tau)));
      break;

    default:
      mooseError("Water97FluidProperties::inRegion has given an incorrect region");
  }

  return std::sqrt(speed2);
}

Real
Water97FluidProperties::cp(Real pressure, Real temperature) const
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
      mooseError("Water97FluidProperties::inRegion has given an incorrect region");
  }
  return specific_heat;
}

Real
Water97FluidProperties::cv(Real pressure, Real temperature) const
{
  Real specific_heat, pi, tau, delta;

  // Determine which region the point is in
  unsigned int region = inRegion(pressure, temperature);
  switch (region)
  {
    case 1:
      pi = pressure / _p_star[0];
      tau = _T_star[0] / temperature;
      specific_heat = _Rw * (-tau * tau * d2gamma1_dtau2(pi, tau) +
                             std::pow(dgamma1_dpi(pi, tau) - tau * d2gamma1_dpitau(pi, tau), 2) /
                                 d2gamma1_dpi2(pi, tau));
      break;

    case 2:
      pi = pressure / _p_star[1];
      tau = _T_star[1] / temperature;
      specific_heat = _Rw * (-tau * tau * d2gamma2_dtau2(pi, tau) +
                             std::pow(dgamma2_dpi(pi, tau) - tau * d2gamma2_dpitau(pi, tau), 2) /
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
      specific_heat = _Rw * (-tau * tau * d2gamma5_dtau2(pi, tau) +
                             std::pow(dgamma5_dpi(pi, tau) - tau * d2gamma5_dpitau(pi, tau), 2) /
                                 d2gamma5_dpi2(pi, tau));
      break;

    default:
      mooseError("Water97FluidProperties::inRegion has given an incorrect region");
  }
  return specific_heat;
}

Real
Water97FluidProperties::mu(Real density, Real temperature) const
{
  // Constants from Release on the IAPWS Formulation 2008 for the Viscosity of
  // Ordinary Water Substance
  const std::vector<int> I{0, 1, 2, 3, 0, 1, 2, 3, 5, 0, 1, 2, 3, 4, 0, 1, 0, 3, 4, 3, 5};
  const std::vector<int> J{0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 4, 4, 5, 6, 6};
  const std::vector<Real> H0{1.67752, 2.20462, 0.6366564, -0.241605};
  const std::vector<Real> H1{
      5.20094e-1, 8.50895e-2, -1.08374,    -2.89555e-1, 2.22531e-1,  9.99115e-1,  1.88797,
      1.26613,    1.20573e-1, -2.81378e-1, -9.06851e-1, -7.72479e-1, -4.89837e-1, -2.57040e-1,
      1.61913e-1, 2.57399e-1, -3.25372e-2, 6.98452e-2,  8.72102e-3,  -4.35673e-3, -5.93264e-4};

  Real mu_star = 1.e-6;
  Real rhobar = density / _rho_critical;
  Real Tbar = temperature / _T_critical;

  // Viscosity in limit of zero density
  Real sum0 = 0.0;
  for (unsigned int i = 0; i < H0.size(); ++i)
    sum0 += H0[i] / std::pow(Tbar, i);

  Real mu0 = 100.0 * std::sqrt(Tbar) / sum0;

  // Residual component due to finite density
  Real sum1 = 0.0;
  for (unsigned int i = 0; i < H1.size(); ++i)
    sum1 += std::pow(1.0 / Tbar - 1.0, I[i]) * H1[i] * std::pow(rhobar - 1.0, J[i]);

  Real mu1 = std::exp(rhobar * sum1);

  // The water viscosity (in Pa.s) is then given by
  return mu_star * mu0 * mu1;
}

void
Water97FluidProperties::mu_drhoT(
    Real density, Real temperature, Real & mu, Real & dmu_drho, Real & dmu_dT) const
{
  // Constants from Release on the IAPWS Formulation 2008 for the Viscosity of
  // Ordinary Water Substance
  const std::vector<int> I{0, 1, 2, 3, 0, 1, 2, 3, 5, 0, 1, 2, 3, 4, 0, 1, 0, 3, 4, 3, 5};
  const std::vector<int> J{0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 4, 4, 5, 6, 6};
  const std::vector<Real> H0{1.67752, 2.20462, 0.6366564, -0.241605};
  const std::vector<Real> H1{
      5.20094e-1, 8.50895e-2, -1.08374,    -2.89555e-1, 2.22531e-1,  9.99115e-1,  1.88797,
      1.26613,    1.20573e-1, -2.81378e-1, -9.06851e-1, -7.72479e-1, -4.89837e-1, -2.57040e-1,
      1.61913e-1, 2.57399e-1, -3.25372e-2, 6.98452e-2,  8.72102e-3,  -4.35673e-3, -5.93264e-4};

  Real mu_star = 1.0e-6;
  Real rhobar = density / _rho_critical;
  Real Tbar = temperature / _T_critical;
  Real drhobar_drho = 1.0 / _rho_critical;

  // Limit of zero density. Derivative wrt rho is 0
  Real sum0 = 0.0;
  for (unsigned int i = 0; i < H0.size(); ++i)
    sum0 += H0[i] / std::pow(Tbar, i);

  Real mu0 = 100.0 * std::sqrt(Tbar) / sum0;

  // Residual component due to finite density
  Real sum1 = 0.0;
  Real dsum1_drho = 0.0;
  for (unsigned int i = 0; i < H1.size(); ++i)
  {
    sum1 += std::pow(1.0 / Tbar - 1.0, I[i]) * H1[i] * std::pow(rhobar - 1.0, J[i]);
    dsum1_drho +=
        std::pow(1.0 / Tbar - 1.0, I[i]) * H1[i] * J[i] * std::pow(rhobar - 1.0, J[i] - 1);
  }

  Real mu1 = std::exp(rhobar * sum1);
  Real dmu1_drho = (sum1 + rhobar * dsum1_drho) * mu1;

  // Use finite difference for derivative wrt T for now, as drho_dT is required
  // to calculate analytical derivative
  Real eps = 1.0e-8;
  Real Teps = temperature * eps;
  Real mu2T = this->mu(density, temperature + Teps);

  // Viscosity and its derivatives are then
  mu = mu_star * mu0 * mu1;
  dmu_drho = mu_star * mu0 * dmu1_drho * drhobar_drho;
  dmu_dT = (mu2T - mu) / Teps;
}

Real
Water97FluidProperties::k(Real density, Real temperature) const
{
  // Scale the density and temperature. Note that the scales are slightly
  // different to the critical values used in IAPWS-IF97
  Real Tbar = temperature / 647.26;
  Real rhobar = density / 317.7;
  std::vector<Real> a{0.0102811, 0.0299621, 0.0156146, -0.00422464};

  // Ideal gas component
  Real sum0 = 0.0;

  for (unsigned int i = 0; i < a.size(); ++i)
    sum0 += a[i] * std::pow(Tbar, i);

  Real lambda0 = std::sqrt(Tbar) * sum0;

  // The contribution due to finite density
  Real lambda1 =
      -0.39707 + 0.400302 * rhobar + 1.06 * std::exp(-0.171587 * std::pow(rhobar + 2.392190, 2.0));

  // Critical enhancement
  Real DeltaT = std::abs(Tbar - 1.0) + 0.00308976;
  Real Q = 2.0 + 0.0822994 / std::pow(DeltaT, 0.6);
  Real S = (Tbar >= 1.0 ? 1.0 / DeltaT : 10.0932 / std::pow(DeltaT, 0.6));

  Real lambda2 = (0.0701309 / std::pow(Tbar, 10.0) + 0.011852) * std::pow(rhobar, 1.8) *
                     std::exp(0.642857 * (1.0 - std::pow(rhobar, 2.8))) +
                 0.00169937 * S * std::pow(rhobar, Q) *
                     std::exp((Q / (1.0 + Q)) * (1.0 - std::pow(rhobar, 1.0 + Q))) -
                 1.02 * std::exp(-4.11717 * std::pow(Tbar, 1.5) - 6.17937 / std::pow(rhobar, 5.0));

  return lambda0 + lambda1 + lambda2;
}

Real
Water97FluidProperties::s(Real pressure, Real temperature) const
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
      mooseError("Water97FluidProperties::inRegion has given an incorrect region");
  }
  return entropy;
}

Real
Water97FluidProperties::h(Real pressure, Real temperature) const
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
Water97FluidProperties::h_dpT(
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

Real Water97FluidProperties::beta(Real /*pressure*/, Real /*temperature*/) const
{
  mooseError("Water97FluidProperties::beta not implemented yet");
  return 0.0;
}

Real
Water97FluidProperties::pSat(Real temperature) const
{
  // Check whether the input temperature is within the region of validity of this equation.
  // Valid for 273.15 K <= t <= 647.096 K
  if (temperature < 273.15 || temperature > _T_critical)
    mooseError(
        "Water97FluidProperties::pSat: Temperature is outside range 273.15 K <= T <= 647.096 K");

  // Constants for region 4 (the saturation curve up to the critical point)
  const std::vector<Real> n4{0.11670521452767e4,
                             -0.72421316703206e6,
                             -0.17073846940092e2,
                             0.12020824702470e5,
                             -0.32325550322333e7,
                             0.14915108613530e2,
                             -0.48232657361591e4,
                             0.40511340542057e6,
                             -0.238555575678490,
                             0.65017534844798e3};

  Real theta, theta2, a, b, c, p;
  theta = temperature + n4[8] / (temperature - n4[9]);
  theta2 = theta * theta;
  a = theta2 + n4[0] * theta + n4[1];
  b = n4[2] * theta2 + n4[3] * theta + n4[4];
  c = n4[5] * theta2 + n4[6] * theta + n4[7];
  p = std::pow(2.0 * c / (-b + std::sqrt(b * b - 4.0 * a * c)), 4.0);

  return p * 1.e6;
}

void
Water97FluidProperties::pSat_dT(Real temperature, Real & psat, Real & dpsat_dT) const
{
  // Check whether the input temperature is within the region of validity of this equation.
  // Valid for 273.15 K <= t <= 647.096 K
  if (temperature < 273.15 || temperature > _T_critical)
    mooseError(
        "Water97FluidProperties::pSat_dT: Temperature is outside range 273.15 K <= T <= 647.096 K");

  // Constants for region 4 (the saturation curve up to the critical point)
  const std::vector<Real> n4{0.11670521452767e4,
                             -0.72421316703206e6,
                             -0.17073846940092e2,
                             0.12020824702470e5,
                             -0.32325550322333e7,
                             0.14915108613530e2,
                             -0.48232657361591e4,
                             0.40511340542057e6,
                             -0.238555575678490,
                             0.65017534844798e3};

  Real theta, dtheta_dT, theta2, a, b, c, da_dtheta, db_dtheta, dc_dtheta;
  theta = temperature + n4[8] / (temperature - n4[9]);
  dtheta_dT = 1.0 - n4[8] / (temperature - n4[9]) / (temperature - n4[9]);
  theta2 = theta * theta;

  a = theta2 + n4[0] * theta + n4[1];
  b = n4[2] * theta2 + n4[3] * theta + n4[4];
  c = n4[5] * theta2 + n4[6] * theta + n4[7];

  da_dtheta = 2.0 * theta + n4[0];
  db_dtheta = 2.0 * n4[2] * theta + n4[3];
  dc_dtheta = 2.0 * n4[5] * theta + n4[6];

  Real denominator = -b + std::sqrt(b * b - 4.0 * a * c);

  psat = std::pow(2.0 * c / denominator, 4.0) * 1.0e6;

  // The derivative wrt temperature is given by the chain rule
  Real dpsat = 4.0 * std::pow(2.0 * c / denominator, 3.0);
  dpsat *= (2.0 * dc_dtheta / denominator -
            2.0 * c / denominator / denominator *
                (-db_dtheta +
                 std::pow(b * b - 4.0 * a * c, -0.5) *
                     (b * db_dtheta - 2.0 * da_dtheta * c - 2.0 * a * dc_dtheta)));
  dpsat_dT = dpsat * dtheta_dT * 1.0e6;
}

Real
Water97FluidProperties::TSat(Real pressure) const
{
  // Check whether the input pressure is within the region of validity of this equation.
  // Valid for 611.213 Pa <= p <= 22.064 MPa
  if (pressure < 611.23 || pressure > _p_critical)
    mooseError(
        "Water97FluidProperties::TSat: Pressure is outside range 611.213 Pa <= p <= 22.064 MPa");

  // Constants for region 4 (the saturation curve up to the critical point)
  const std::vector<Real> n4{0.11670521452767e4,
                             -0.72421316703206e6,
                             -0.17073846940092e2,
                             0.12020824702470e5,
                             -0.32325550322333e7,
                             0.14915108613530e2,
                             -0.48232657361591e4,
                             0.40511340542057e6,
                             -0.238555575678490,
                             0.65017534844798e3};

  Real beta, beta2, e, f, g, d;
  beta = std::pow(pressure / 1.e6, 0.25);
  beta2 = beta * beta;
  e = beta2 + n4[2] * beta + n4[5];
  f = n4[0] * beta2 + n4[3] * beta + n4[6];
  g = n4[1] * beta2 + n4[4] * beta + n4[7];
  d = 2.0 * g / (-f - std::sqrt(f * f - 4.0 * e * g));

  return (n4[9] + d - std::sqrt((n4[9] + d) * (n4[9] + d) - 4.0 * (n4[8] + n4[9] * d))) / 2.0;
}

Real
Water97FluidProperties::b23p(Real temperature) const
{
  // Check whether the input temperature is within the region of validity of this equation.
  // Valid for 623.15 K <= t <= 863.15 K
  if (temperature < 623.15 || temperature > 863.15)
    mooseError(
        "Water97FluidProperties::b23p: Temperature is outside range of 623.15 K <= T <= 863.15 K");

  // Constants for the boundary between regions 2 and 3
  const std::vector<Real> n23{0.34805185628969e3,
                              -0.11671859879975e1,
                              0.10192970039326e-2,
                              0.57254459862746e3,
                              0.13918839778870e2};

  return (n23[0] + n23[1] * temperature + n23[2] * temperature * temperature) * 1.e6;
}

Real
Water97FluidProperties::b23T(Real pressure) const
{
  // Check whether the input pressure is within the region of validity of this equation.
  // Valid for 16.529 MPa <= p <= 100 MPa
  if (pressure < 16.529e6 || pressure > 100.0e6)
    mooseError(
        "Water97FluidProperties::b23T: Pressure is outside range 16.529 MPa <= p <= 100 MPa");

  // Constants for the boundary between regions 2 and 3
  const std::vector<Real> n23{0.34805185628969e3,
                              -0.11671859879975e1,
                              0.10192970039326e-2,
                              0.57254459862746e3,
                              0.13918839778870e2};

  return n23[3] + std::sqrt((pressure / 1.e6 - n23[4]) / n23[2]);
}

unsigned int
Water97FluidProperties::inRegion(Real pressure, Real temperature) const
{
  // Valid for 273.15 K <= T <= 1073.15 K, p <= 100 MPa
  //          1073.15 K <= T <= 2273.15 K, p <= 50 Mpa
  if (temperature > 273.15 && temperature <= 1073.15)
  {
    if (pressure < 0.0 || pressure > 100.0e6)
      mooseError("Pressure ", pressure, " is out of range in Water97FluidProperties::inRegion");
  }
  else if (temperature > 1073.15 && temperature <= 2273.15)
  {
    if (pressure < 0.0 || pressure > 50.0e6)
      mooseError("Pressure ", pressure, " is out of range in Water97FluidProperties::inRegion");
  }
  else
    mooseError("Temperature ", temperature, " is out of range in Water97FluidProperties::inRegion");

  // Determine the phase region that the (P, T) point lies in
  unsigned int region;

  if (temperature >= 273.15 && temperature <= 623.15)
  {
    if (pressure > pSat(temperature) && pressure <= 100.0e6)
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
  for (unsigned int i = 0; i < _n1.size(); ++i)
    sum += _n1[i] * std::pow(7.1 - pi, _I1[i]) * std::pow(tau - 1.222, _J1[i]);

  return sum;
}

Real
Water97FluidProperties::dgamma1_dpi(Real pi, Real tau) const
{
  Real sum = 0.0;
  for (unsigned int i = 0; i < _n1.size(); ++i)
    sum += -_n1[i] * _I1[i] * std::pow(7.1 - pi, _I1[i] - 1) * std::pow(tau - 1.222, _J1[i]);

  return sum;
}

Real
Water97FluidProperties::d2gamma1_dpi2(Real pi, Real tau) const
{
  Real sum = 0.0;
  for (unsigned int i = 0; i < _n1.size(); ++i)
    sum += _n1[i] * _I1[i] * (_I1[i] - 1) * std::pow(7.1 - pi, _I1[i] - 2) *
           std::pow(tau - 1.222, _J1[i]);

  return sum;
}

Real
Water97FluidProperties::dgamma1_dtau(Real pi, Real tau) const
{
  Real g = 0.0;
  for (unsigned int i = 0; i < _n1.size(); ++i)
    g += _n1[i] * _J1[i] * std::pow(7.1 - pi, _I1[i]) * std::pow(tau - 1.222, _J1[i] - 1);

  return g;
}

Real
Water97FluidProperties::d2gamma1_dtau2(Real pi, Real tau) const
{
  Real dg = 0.0;
  for (unsigned int i = 0; i < _n1.size(); ++i)
    dg += _n1[i] * _J1[i] * (_J1[i] - 1) * std::pow(7.1 - pi, _I1[i]) *
          std::pow(tau - 1.222, _J1[i] - 2);

  return dg;
}

Real
Water97FluidProperties::d2gamma1_dpitau(Real pi, Real tau) const
{
  Real dg = 0.0;
  for (unsigned int i = 0; i < _n1.size(); ++i)
    dg += -_n1[i] * _I1[i] * _J1[i] * std::pow(7.1 - pi, _I1[i] - 1) *
          std::pow(tau - 1.222, _J1[i] - 1);

  return dg;
}

Real
Water97FluidProperties::gamma2(Real pi, Real tau) const
{
  // Ideal gas part of the Gibbs free energy
  Real sum0 = 0.0;
  for (unsigned int i = 0; i < _n02.size(); ++i)
    sum0 += _n02[i] * std::pow(tau, _J02[i]);

  Real g0 = std::log(pi) + sum0;

  // Residual part of the Gibbs free energy
  Real gr = 0.0;
  for (unsigned int i = 0; i < _n2.size(); ++i)
    gr += _n2[i] * std::pow(pi, _I2[i]) * std::pow(tau - 0.5, _J2[i]);

  return g0 + gr;
}

Real
Water97FluidProperties::dgamma2_dpi(Real pi, Real tau) const
{
  // Ideal gas part of the Gibbs free energy
  Real dg0 = 1.0 / pi;

  // Residual part of the Gibbs free energy
  Real dgr = 0.0;
  for (unsigned int i = 0; i < _n2.size(); ++i)
    dgr += _n2[i] * _I2[i] * std::pow(pi, _I2[i] - 1) * std::pow(tau - 0.5, _J2[i]);

  return dg0 + dgr;
}

Real
Water97FluidProperties::d2gamma2_dpi2(Real pi, Real tau) const
{
  // Ideal gas part of the Gibbs free energy
  Real dg0 = -1.0 / pi / pi;

  // Residual part of the Gibbs free energy
  Real dgr = 0.0;
  for (unsigned int i = 0; i < _n2.size(); ++i)
    dgr += _n2[i] * _I2[i] * (_I2[i] - 1) * std::pow(pi, _I2[i] - 2) * std::pow(tau - 0.5, _J2[i]);

  return dg0 + dgr;
}

Real
Water97FluidProperties::dgamma2_dtau(Real pi, Real tau) const
{
  // Ideal gas part of the Gibbs free energy
  Real dg0 = 0.0;
  for (unsigned int i = 0; i < _n02.size(); ++i)
    dg0 += _n02[i] * _J02[i] * std::pow(tau, _J02[i] - 1);

  // Residual part of the Gibbs free energy
  Real dgr = 0.0;
  for (unsigned int i = 0; i < _n2.size(); ++i)
    dgr += _n2[i] * _J2[i] * std::pow(pi, _I2[i]) * std::pow(tau - 0.5, _J2[i] - 1);

  return dg0 + dgr;
}

Real
Water97FluidProperties::d2gamma2_dtau2(Real pi, Real tau) const
{
  // Ideal gas part of the Gibbs free energy
  Real dg0 = 0.0;
  for (unsigned int i = 0; i < _n02.size(); ++i)
    dg0 += _n02[i] * _J02[i] * (_J02[i] - 1) * std::pow(tau, _J02[i] - 2);

  // Residual part of the Gibbs free energy
  Real dgr = 0.0;
  for (unsigned int i = 0; i < _n2.size(); ++i)
    dgr += _n2[i] * _J2[i] * (_J2[i] - 1) * std::pow(pi, _I2[i]) * std::pow(tau - 0.5, _J2[i] - 2);

  return dg0 + dgr;
}

Real
Water97FluidProperties::d2gamma2_dpitau(Real pi, Real tau) const
{
  // Ideal gas part of the Gibbs free energy
  Real dg0 = 0.0;

  // Residual part of the Gibbs free energy
  Real dgr = 0.0;
  for (unsigned int i = 0; i < _n2.size(); ++i)
    dgr += _n2[i] * _I2[i] * _J2[i] * std::pow(pi, _I2[i] - 1) * std::pow(tau - 0.5, _J2[i] - 1);

  return dg0 + dgr;
}

Real
Water97FluidProperties::phi3(Real delta, Real tau) const
{
  Real sum = 0.0;
  for (unsigned int i = 1; i < _n3.size(); ++i)
    sum += _n3[i] * std::pow(delta, _I3[i]) * std::pow(tau, _J3[i]);

  return _n3[0] * std::log(delta) + sum;
}

Real
Water97FluidProperties::dphi3_ddelta(Real delta, Real tau) const
{
  Real sum = 0.0;
  for (unsigned int i = 1; i < _n3.size(); ++i)
    sum += _n3[i] * _I3[i] * std::pow(delta, _I3[i] - 1) * std::pow(tau, _J3[i]);

  return _n3[0] / delta + sum;
}

Real
Water97FluidProperties::d2phi3_ddelta2(Real delta, Real tau) const
{
  Real sum = 0.0;
  for (unsigned int i = 1; i < _n3.size(); ++i)
    sum += _n3[i] * _I3[i] * (_I3[i] - 1) * std::pow(delta, _I3[i] - 2) * std::pow(tau, _J3[i]);

  return -_n3[0] / delta / delta + sum;
}

Real
Water97FluidProperties::dphi3_dtau(Real delta, Real tau) const
{
  Real sum = 0.0;
  for (unsigned int i = 1; i < _n3.size(); ++i)
    sum += _n3[i] * _J3[i] * std::pow(delta, _I3[i]) * std::pow(tau, _J3[i] - 1);

  return sum;
}

Real
Water97FluidProperties::d2phi3_dtau2(Real delta, Real tau) const
{
  Real sum = 0.0;
  for (unsigned int i = 1; i < _n3.size(); ++i)
    sum += _n3[i] * _J3[i] * (_J3[i] - 1) * std::pow(delta, _I3[i]) * std::pow(tau, _J3[i] - 2);

  return sum;
}

Real
Water97FluidProperties::d2phi3_ddeltatau(Real delta, Real tau) const
{
  Real sum = 0.0;
  for (unsigned int i = 1; i < _n3.size(); ++i)
    sum += _n3[i] * _I3[i] * _J3[i] * std::pow(delta, _I3[i] - 1) * std::pow(tau, _J3[i] - 1);

  return sum;
}

Real
Water97FluidProperties::gamma5(Real pi, Real tau) const
{
  // Ideal gas part of the Gibbs free energy
  Real sum0 = 0.0;
  for (unsigned int i = 0; i < _n05.size(); ++i)
    sum0 += _n05[i] * std::pow(tau, _J05[i]);

  Real g0 = std::log(pi) + sum0;

  // Residual part of the Gibbs free energy
  Real gr = 0.0;
  for (unsigned int i = 0; i < _n5.size(); ++i)
    gr += _n5[i] * std::pow(pi, _I5[i]) * std::pow(tau, _J5[i]);

  return g0 + gr;
}

Real
Water97FluidProperties::dgamma5_dpi(Real pi, Real tau) const
{
  // Ideal gas part of the Gibbs free energy
  Real dg0 = 1.0 / pi;

  // Residual part of the Gibbs free energy
  Real dgr = 0.0;
  for (unsigned int i = 0; i < _n5.size(); ++i)
    dgr += _n5[i] * _I5[i] * std::pow(pi, _I5[i] - 1) * std::pow(tau, _J5[i]);

  return dg0 + dgr;
}

Real
Water97FluidProperties::d2gamma5_dpi2(Real pi, Real tau) const
{
  // Ideal gas part of the Gibbs free energy
  Real dg0 = -1.0 / pi / pi;

  // Residual part of the Gibbs free energy
  Real dgr = 0.0;
  for (unsigned int i = 0; i < _n5.size(); ++i)
    dgr += _n5[i] * _I5[i] * (_I5[i] - 1) * std::pow(pi, _I5[i] - 2) * std::pow(tau, _J5[i]);

  return dg0 + dgr;
}

Real
Water97FluidProperties::dgamma5_dtau(Real pi, Real tau) const
{
  // Ideal gas part of the Gibbs free energy
  Real dg0 = 0.0;
  for (unsigned int i = 0; i < _n05.size(); ++i)
    dg0 += _n05[i] * _J05[i] * std::pow(tau, _J05[i] - 1);

  // Residual part of the Gibbs free energy
  Real dgr = 0.0;
  for (unsigned int i = 0; i < _n5.size(); ++i)
    dgr += _n5[i] * _J5[i] * std::pow(pi, _I5[i]) * std::pow(tau, _J5[i] - 1);

  return dg0 + dgr;
}

Real
Water97FluidProperties::d2gamma5_dtau2(Real pi, Real tau) const
{
  // Ideal gas part of the Gibbs free energy
  Real dg0 = 0.0;
  for (unsigned int i = 0; i < _n05.size(); ++i)
    dg0 += _n05[i] * _J05[i] * (_J05[i] - 1) * std::pow(tau, _J05[i] - 2);

  // Residual part of the Gibbs free energy
  Real dgr = 0.0;
  for (unsigned int i = 0; i < _n5.size(); ++i)
    dgr += _n5[i] * _J5[i] * (_J5[i] - 1) * std::pow(pi, _I5[i]) * std::pow(tau, _J5[i] - 2);

  return dg0 + dgr;
}

Real
Water97FluidProperties::d2gamma5_dpitau(Real pi, Real tau) const
{
  // Ideal gas part of the Gibbs free energy
  Real dg0 = 0.0;

  // Residual part of the Gibbs free energy
  Real dgr = 0.0;
  for (unsigned int i = 0; i < _n5.size(); ++i)
    dgr += _n5[i] * _I5[i] * _J5[i] * std::pow(pi, _I5[i] - 1) * std::pow(tau, _J5[i] - 1);

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
  else if (pMPa > pSat(643.15) * 1.0e-6 && pMPa <= 22.5) // pSat(643.15) = 21.04 MPa
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
      else if (temperature <= TSat(pressure))
      {
        if (pMPa > 21.93161551 && pMPa <= 22.064)
          if (temperature > tempXY(pressure, QU) && temperature <= tempXY(pressure, UV))
            subregion = 20;
          else
            subregion = 24;
        else // (pMPa > pSat(643.15) * 1.0e-6 && pMPa <= 21.93161551)
          subregion = 20;
      }
      else if (temperature > TSat(pressure))
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
  else if (pMPa > 20.5 && pMPa <= pSat(643.15) * 1.0e-6) // pSat(643.15) = 21.04 MPa
  {
    if (temperature <= tempXY(pressure, CD))
      subregion = 2;
    else if (temperature > tempXY(pressure, CD) && temperature <= TSat(pressure))
      subregion = 16;
    else if (temperature > TSat(pressure) && temperature <= tempXY(pressure, JK))
      subregion = 17;
    else // (temperature > tempXY(pressure, JK))
      subregion = 10;
  }
  else if (pMPa > P3cd && pMPa <= 20.5) // P3cd  = 19.00881189173929
  {
    if (temperature <= tempXY(pressure, CD))
      subregion = 2;
    else if (temperature > tempXY(pressure, CD) && temperature <= TSat(pressure))
      subregion = 18;
    else
      subregion = 19;
  }
  else if (pMPa > pSat(623.15) * 1.0e-6 && pMPa <= P3cd)
  {
    if (temperature < TSat(pressure))
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
    mooseError("Water97FluidProperties::subregion3. Shouldn't have got here!");

  return subregion;
}

Real
Water97FluidProperties::tempXY(Real pressure, subregionEnum xy) const
{
  Real pi = pressure / 1.0e6;

  const std::vector<std::vector<int>> I{{0, 1, 2, -1, -2},
                                        {0, 1, 2, 3},
                                        {0, 1, 2, 3, 4},
                                        {0, 1, 2, 3, 4},
                                        {0, 1, 2, 3, 4},
                                        {0, 1, 2, 3},
                                        {0, 1, 2, -1, -2},
                                        {0, 1, 2, 3},
                                        {0, 1, 2, 3},
                                        {0, 1, 2, 3, 4},
                                        {0, 1, 2, -1, -2}};

  const std::vector<std::vector<Real>> n{
      {0.154793642129415e4,
       -0.187661219490113e3,
       0.213144632222113e2,
       -0.191887498864292e4,
       0.918419702359447e3},
      {0.585276966696349e3, 0.278233532206915e1, -0.127283549295878e-1, 0.159090746562729e-3},
      {-0.249284240900418e5,
       0.428143584791546e4,
       -0.269029173140130e3,
       0.751608051114157e1,
       -0.787105249910383e-1},
      {0.584814781649163e3,
       -0.616179320924617,
       0.260763050899562,
       -0.587071076864459e-2,
       0.515308185433082e-4},
      {0.617229772068439e3,
       -0.770600270141675e1,
       0.697072596851896,
       -0.157391839848015e-1,
       0.137897492684194e-3},
      {0.535339483742384e3, 0.761978122720128e1, -0.158365725441648, 0.192871054508108e-2},
      {0.969461372400213e3,
       -0.332500170441278e3,
       0.642859598466067e2,
       0.773845935768222e3,
       -0.152313732937084e4},
      {0.565603648239126e3, 0.529062258221222e1, -0.102020639611016, 0.122240301070145e-2},
      {0.584561202520006e3, -0.102961025163669e1, 0.243293362700452, -0.294905044740799e-2},
      {0.528199646263062e3, 0.890579602135307e1, -0.222814134903755, 0.286791682263697e-2},
      {0.728052609145380e1,
       0.973505869861952e2,
       0.147370491183191e2,
       0.329196213998375e3,
       0.873371668682417e3}};

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
    for (unsigned int i = 0; i < n[row].size(); ++i)
      sum += n[row][i] * std::pow(std::log(pi), I[row][i]);
  else if (xy == EF)
    sum += 3.727888004 * (pi - _p_critical / 1.0e6) + _T_critical;
  else
    for (unsigned int i = 0; i < n[row].size(); ++i)
      sum += n[row][i] * std::pow(pi, I[row][i]);

  return sum;
}

Real
Water97FluidProperties::subregionVolume(
    Real pi, Real theta, Real a, Real b, Real c, Real d, Real e, unsigned int sid) const
{
  Real sum = 0.0;

  for (unsigned int i = 0; i < _n3s[sid].size(); ++i)
    sum += _n3s[sid][i] * std::pow(std::pow(pi - a, c), _I3s[sid][i]) *
           std::pow(std::pow(theta - b, d), _J3s[sid][i]);

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
    for (unsigned int i = 0; i < N; ++i)
      sum += _n3s[sid][i] * std::pow(pi - a, _I3s[sid][i]) * std::pow(theta - b, _J3s[sid][i]);

    volume = vstar * std::exp(sum);
  }
  else
    volume = vstar * subregionVolume(pi, theta, a, b, c, d, e, sid);

  // Density is the inverse of volume
  return 1.0 / volume;
}

Real Water97FluidProperties::henryConstant(Real /*temperature*/) const
{
  mooseError("Water97FluidProperties::henryConstant() not defined");
  return 0.0;
}

void
Water97FluidProperties::henryConstant_dT(Real /* temperature */,
                                         Real & /* Kh */,
                                         Real & /* dKh_dT */) const
{
  mooseError("Water97FluidProperties::henryConstant_dT() not defined");
}
