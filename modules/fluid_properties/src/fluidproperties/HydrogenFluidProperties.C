//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HydrogenFluidProperties.h"
#include "Conversion.h"
#include "MathUtils.h"
#include "libmesh/utility.h"

registerMooseObject("FluidPropertiesApp", HydrogenFluidProperties);

InputParameters
HydrogenFluidProperties::validParams()
{
  InputParameters params = HelmholtzFluidProperties::validParams();
  params.addClassDescription("Fluid properties for Hydrogen (H2)");
  return params;
}

HydrogenFluidProperties::HydrogenFluidProperties(const InputParameters & parameters)
  : HelmholtzFluidProperties(parameters),
    _Mh2(2.01588e-3),
    _p_critical(1.315e6),
    _T_critical(33.19),
    _rho_molar_critical(15.508),
    _rho_critical(1000.0 * _rho_molar_critical * _Mh2),
    _p_triple(7.7e3),
    _T_triple(13.952)
{
}

std::string
HydrogenFluidProperties::fluidName() const
{
  return "hydrogen";
}

Real
HydrogenFluidProperties::molarMass() const
{
  return _Mh2;
}

Real
HydrogenFluidProperties::criticalPressure() const
{
  return _p_critical;
}

Real
HydrogenFluidProperties::criticalTemperature() const
{
  return _T_critical;
}

Real
HydrogenFluidProperties::criticalDensity() const
{
  return _rho_critical;
}

Real
HydrogenFluidProperties::triplePointPressure() const
{
  return _p_triple;
}

Real
HydrogenFluidProperties::triplePointTemperature() const
{
  return _T_triple;
}

Real
HydrogenFluidProperties::mu_from_rho_T(Real density, Real temperature) const
{
  // Scaled variables
  const Real Tstar = temperature / 30.41;
  const Real logTstar = std::log(Tstar);
  const Real Tr = temperature / _T_critical;
  const Real rhor = density / 90.5;

  // Ideal gas component
  Real sum = 0.0;
  for (std::size_t i = 0; i < _amu.size(); ++i)
    sum += _amu[i] * MathUtils::pow(logTstar, i);

  const Real mu0 =
      0.021357 * std::sqrt(1000.0 * _Mh2 * temperature) / (0.297 * 0.297 * std::exp(sum));

  // The excess contribution due to density
  Real sumr = 0.0;
  for (std::size_t i = 0; i < _bmu.size(); ++i)
    sumr += _bmu[i];

  const Real mu1 = MathUtils::pow(0.297, 3) * sumr * mu0 / Tstar;

  // The viscosity is then
  const Real mu =
      mu0 + mu1 * density +
      _cmu[0] * rhor * rhor *
          std::exp(_cmu[1] * Tr + _cmu[2] / Tr + _cmu[3] * rhor * rhor / (_cmu[4] + Tr) +
                   _cmu[5] * MathUtils::pow(rhor, 6));

  return mu * 1.0e-6;
}

void
HydrogenFluidProperties::mu_from_rho_T(Real density,
                                       Real temperature,
                                       Real ddensity_dT,
                                       Real & mu,
                                       Real & dmu_drho,
                                       Real & dmu_dT) const
{
  // Scaled variables
  const Real Tstar = temperature / 30.41;
  const Real logTstar = std::log(Tstar);
  const Real Tr = temperature / _T_critical;
  const Real rhor = density / 90.5;
  const Real drhor_drho = 1.0 / 90.5;
  const Real dTr_dT = 1.0 / _T_critical;

  // The dilute gas component
  Real sum = 0.0, dsum_dT = 0.0;
  for (std::size_t i = 0; i < _amu.size(); ++i)
  {
    sum += _amu[i] * MathUtils::pow(logTstar, i);
    dsum_dT += i * _amu[i] * MathUtils::pow(logTstar, i) / (temperature * logTstar);
  }

  const Real mu0 =
      0.021357 * std::sqrt(1000.0 * _Mh2 * temperature) / (0.297 * 0.297 * std::exp(sum));
  const Real dmu0_dT = 21.357 * _Mh2 * (1.0 - 2.0 * temperature * dsum_dT) * std::exp(-sum) /
                       (2.0 * std::sqrt(1000.0 * _Mh2 * temperature) * 0.297 * 0.297);

  // The excess contribution due to density
  Real sumr = 0.0;
  for (std::size_t i = 0; i < _bmu.size(); ++i)
    sumr += _bmu[i];

  const Real mu1 = MathUtils::pow(0.297, 3) * sumr * mu0 / Tstar;
  const Real dmu1_dT =
      MathUtils::pow(0.297, 3) * sumr * (dmu0_dT / Tstar - mu0 / (30.41 * Tstar * Tstar));

  // The viscosity and derivatives are then
  const Real exponent = _cmu[1] * Tr + _cmu[2] / Tr + _cmu[3] * rhor * rhor / (_cmu[4] + Tr) +
                        _cmu[5] * MathUtils::pow(rhor, 6);
  const Real dexponent_drho =
      (2.0 * _cmu[3] * rhor / (_cmu[4] + Tr) + 6.0 * _cmu[5] * MathUtils::pow(rhor, 5)) *
      drhor_drho;
  const Real dexponent_dT =
      (_cmu[1] - _cmu[2] / Tr / Tr - _cmu[3] * rhor * rhor / (_cmu[4] + Tr) / (_cmu[4] + Tr)) *
      dTr_dT;

  mu = (mu0 + mu1 * density + _cmu[0] * rhor * rhor * std::exp(exponent)) * 1.0e-6;
  dmu_drho =
      (mu1 + _cmu[0] * rhor * std::exp(exponent) * (2.0 * drhor_drho + rhor * dexponent_drho)) *
      1.0e-6;
  dmu_dT =
      (dmu0_dT + density * dmu1_dT + _cmu[0] * rhor * rhor * dexponent_dT * std::exp(exponent)) *
          1.0e-6 +
      dmu_drho * ddensity_dT;
}

Real
HydrogenFluidProperties::mu_from_p_T(Real pressure, Real temperature) const
{
  // Require density first
  const Real density = rho_from_p_T(pressure, temperature);
  return mu_from_rho_T(density, temperature);
}

void
HydrogenFluidProperties::mu_from_p_T(
    Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  Real rho, drho_dp, drho_dT;
  rho_from_p_T(pressure, temperature, rho, drho_dp, drho_dT);

  Real dmu_drho;
  mu_from_rho_T(rho, temperature, drho_dT, mu, dmu_drho, dmu_dT);
  dmu_dp = dmu_drho * drho_dp;
}

void
HydrogenFluidProperties::rho_mu_from_p_T(Real pressure,
                                         Real temperature,
                                         Real & rho,
                                         Real & mu) const
{
  rho = rho_from_p_T(pressure, temperature);
  mu = mu_from_rho_T(rho, temperature);
}

void
HydrogenFluidProperties::rho_mu_from_p_T(Real pressure,
                                         Real temperature,
                                         Real & rho,
                                         Real & drho_dp,
                                         Real & drho_dT,
                                         Real & mu,
                                         Real & dmu_dp,
                                         Real & dmu_dT) const
{
  rho_from_p_T(pressure, temperature, rho, drho_dp, drho_dT);
  Real dmu_drho;
  mu_from_rho_T(rho, temperature, drho_dT, mu, dmu_drho, dmu_dT);
  dmu_dp = dmu_drho * drho_dp;
}

Real
HydrogenFluidProperties::k_from_rho_T(Real density, Real temperature) const
{
  // // Scaled variables
  const Real Tr = temperature / 33.145;
  const Real rhor = density / 31.262;

  // The ideal gas component
  Real sum1 = 0.0;
  for (std::size_t i = 0; i < _a1k.size(); ++i)
    sum1 += _a1k[i] * MathUtils::pow(Tr, i);

  Real sum2 = 0.0;
  for (std::size_t i = 0; i < _a2k.size(); ++i)
    sum2 += _a2k[i] * MathUtils::pow(Tr, i);

  const Real lambda0 = sum1 / sum2;

  // The excess contribution due to density
  Real lambdah = 0.0;
  for (std::size_t i = 0; i < _b1k.size(); ++i)
    lambdah += (_b1k[i] + _b2k[i] * Tr) * MathUtils::pow(rhor, i + 1);

  // The critical enhancement
  const Real lambdac = 6.24e-4 / (-2.58e-7 + std::abs(Tr - 1.0)) *
                       std::exp(-MathUtils::pow(0.837 * (rhor - 1.0), 2));

  // The thermal conductivity
  return lambda0 + lambdah + lambdac;
}

Real
HydrogenFluidProperties::k_from_p_T(Real pressure, Real temperature) const
{
  // Require density first
  const Real density = rho_from_p_T(pressure, temperature);
  return k_from_rho_T(density, temperature);
}

void
HydrogenFluidProperties::k_from_p_T(
    Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const
{
  k = this->k_from_p_T(pressure, temperature);
  // Calculate derivatives using finite differences
  const Real eps = 1.0e-6;
  const Real peps = pressure * eps;
  const Real Teps = temperature * eps;

  dk_dp = (this->k_from_p_T(pressure + peps, temperature) - k) / peps;
  dk_dT = (this->k_from_p_T(pressure, temperature + Teps) - k) / Teps;
}

std::vector<Real>
HydrogenFluidProperties::henryCoefficients() const
{
  return {-4.73284, 6.08954, 6.06066};
}

Real
HydrogenFluidProperties::vaporPressure(Real temperature) const
{
  if (temperature < _T_triple || temperature > _T_critical)
    throw MooseException("Temperature is out of range in " + name() + ": vaporPressure()");

  const Real Tr = temperature / _T_critical;
  const Real theta = 1.0 - Tr;

  const Real logpressure = (-4.89789 * theta + 0.988588 * std::pow(theta, 1.5) +
                            0.349689 * Utility::pow<2>(theta) + 0.499356 * std::pow(theta, 2.85)) /
                           Tr;

  return _p_critical * std::exp(logpressure);
}

void
HydrogenFluidProperties::vaporPressure(Real, Real &, Real &) const
{
  mooseError("vaporPressure() is not implemented");
}

Real
HydrogenFluidProperties::alpha(Real delta, Real tau) const
{
  // Ideal gas component of the Helmholtz free energy
  Real alpha0 = std::log(delta) + 1.5 * std::log(tau) - 1.4579856475 + 1.888076782 * tau;

  for (std::size_t i = 0; i < _a.size(); ++i)
    alpha0 += _a[i] * std::log(1.0 - std::exp(_b[i] * tau));

  // Residual component of the Helmholtz free energy
  Real alphar = 0.0;

  for (std::size_t i = 0; i < _t1.size(); ++i)
    alphar += _N1[i] * MathUtils::pow(delta, _d1[i]) * std::pow(tau, _t1[i]);

  for (std::size_t i = 0; i < _t2.size(); ++i)
    alphar += _N2[i] * MathUtils::pow(delta, _d2[i]) * std::pow(tau, _t2[i]) * std::exp(-delta);

  for (std::size_t i = 0; i < _t3.size(); ++i)
    alphar += _N3[i] * MathUtils::pow(delta, _d3[i]) * std::pow(tau, _t3[i]) *
              std::exp(_phi3[i] * Utility::pow<2>(delta - _D3[i]) +
                       _beta3[i] * Utility::pow<2>(tau - _gamma3[i]));

  // The Helmholtz free energy is the sum of these two
  return alpha0 + alphar;
}

Real
HydrogenFluidProperties::dalpha_ddelta(Real delta, Real tau) const
{
  // Ideal gas component of the Helmholtz free energy
  Real dalpha0 = 1.0 / delta;

  // Residual component of the Helmholtz free energy
  Real dalphar = 0.0;

  for (std::size_t i = 0; i < _t1.size(); ++i)
    dalphar += _N1[i] * _d1[i] * MathUtils::pow(delta, _d1[i]) * std::pow(tau, _t1[i]);

  for (std::size_t i = 0; i < _t2.size(); ++i)
    dalphar += _N2[i] * MathUtils::pow(delta, _d2[i]) * std::pow(tau, _t2[i]) * std::exp(-delta) *
               (_d2[i] - delta);

  for (std::size_t i = 0; i < _t3.size(); ++i)
    dalphar += _N3[i] * MathUtils::pow(delta, _d3[i]) * std::pow(tau, _t3[i]) *
               std::exp(_phi3[i] * Utility::pow<2>(delta - _D3[i]) +
                        _beta3[i] * Utility::pow<2>(tau - _gamma3[i])) *
               (_d3[i] + delta * (2.0 * _phi3[i] * (delta - _D3[i])));

  // The Helmholtz free energy is the sum of these two
  return dalpha0 + dalphar / delta;
}

Real
HydrogenFluidProperties::dalpha_dtau(Real delta, Real tau) const
{
  // Ideal gas component of the Helmholtz free energy
  Real dalpha0 = 1.5 / tau + 1.888076782;

  for (std::size_t i = 0; i < _a.size(); ++i)
    dalpha0 += _a[i] * _b[i] * (1.0 - 1.0 / (1.0 - std::exp(_b[i] * tau)));

  // Residual component of the Helmholtz free energy
  Real dalphar = 0.0;

  for (std::size_t i = 0; i < _t1.size(); ++i)
    dalphar += _N1[i] * _t1[i] * MathUtils::pow(delta, _d1[i]) * std::pow(tau, _t1[i]);

  for (std::size_t i = 0; i < _t2.size(); ++i)
    dalphar +=
        _N2[i] * _t2[i] * MathUtils::pow(delta, _d2[i]) * std::pow(tau, _t2[i]) * std::exp(-delta);

  for (std::size_t i = 0; i < _t3.size(); ++i)
    dalphar += _N3[i] * MathUtils::pow(delta, _d3[i]) * std::pow(tau, _t3[i]) *
               std::exp(_phi3[i] * Utility::pow<2>(delta - _D3[i]) +
                        _beta3[i] * Utility::pow<2>(tau - _gamma3[i])) *
               (_t3[i] + tau * (2.0 * _beta3[i] * (tau - _gamma3[i])));

  // The Helmholtz free energy is the sum of these two
  return dalpha0 + dalphar / tau;
}

Real
HydrogenFluidProperties::d2alpha_ddelta2(Real delta, Real tau) const
{
  // Ideal gas component of the Helmholtz free energy
  Real dalpha0 = -1.0 / delta / delta;

  // Residual component of the Helmholtz free energy
  Real dalphar = 0.0;

  for (std::size_t i = 0; i < _t1.size(); ++i)
    dalphar +=
        _N1[i] * _d1[i] * (_d1[i] - 1.0) * MathUtils::pow(delta, _d1[i]) * std::pow(tau, _t1[i]);

  for (std::size_t i = 0; i < _t2.size(); ++i)
    dalphar += _N2[i] * MathUtils::pow(delta, _d2[i]) * std::pow(tau, _t2[i]) * std::exp(-delta) *
               (delta * delta - 2.0 * _d2[i] * delta + _d2[i] * (_d2[i] - 1.0));

  for (std::size_t i = 0; i < _t3.size(); ++i)
    dalphar += _N3[i] * MathUtils::pow(delta, _d3[i]) * std::pow(tau, _t3[i]) *
               std::exp(_phi3[i] * Utility::pow<2>(delta - _D3[i]) +
                        _beta3[i] * Utility::pow<2>(tau - _gamma3[i])) *
               (_d3[i] * _d3[i] +
                2.0 * delta * delta * _phi3[i] *
                    (1.0 + 2.0 * _phi3[i] * (_D3[i] - delta) * (_D3[i] - delta)) +
                _d3[i] * (4.0 * delta * _phi3[i] * (delta - _D3[i]) - 1.0));

  // The Helmholtz free energy is the sum of these two
  return dalpha0 + dalphar / delta / delta;
}

Real
HydrogenFluidProperties::d2alpha_dtau2(Real delta, Real tau) const
{
  // Ideal gas component of the Helmholtz free energy
  Real dalpha0 = -1.5 / tau / tau;

  for (std::size_t i = 0; i < _a.size(); ++i)
  {
    Real exptau = std::exp(_b[i] * tau);
    dalpha0 -= _a[i] * (_b[i] * _b[i] * exptau / (1.0 - exptau) * (exptau / (1.0 - exptau) + 1.0));
  }

  // Residual component of the Helmholtz free energy
  Real dalphar = 0.0;

  for (std::size_t i = 0; i < _t1.size(); ++i)
    dalphar +=
        _N1[i] * _t1[i] * (_t1[i] - 1.0) * MathUtils::pow(delta, _d1[i]) * std::pow(tau, _t1[i]);

  for (std::size_t i = 0; i < _t2.size(); ++i)
    dalphar += _N2[i] * _t2[i] * (_t2[i] - 1.0) * MathUtils::pow(delta, _d2[i]) *
               std::pow(tau, _t2[i]) * std::exp(-delta);

  for (std::size_t i = 0; i < _t3.size(); ++i)
    dalphar += _N3[i] * MathUtils::pow(delta, _d3[i]) * std::pow(tau, _t3[i]) *
               std::exp(_phi3[i] * Utility::pow<2>(delta - _D3[i]) +
                        _beta3[i] * Utility::pow<2>(tau - _gamma3[i])) *
               (_t3[i] * _t3[i] +
                2.0 * _beta3[i] * tau * tau *
                    (1.0 + 2.0 * _beta3[i] * MathUtils::pow(tau - _gamma3[i], 2)) -
                _t3[i] * (1.0 + 4.0 * _beta3[i] * tau * (tau - _gamma3[i])));

  // The Helmholtz free energy is the sum of these two
  return dalpha0 + dalphar / tau / tau;
}

Real
HydrogenFluidProperties::d2alpha_ddeltatau(Real delta, Real tau) const
{
  // Residual component of the Helmholtz free energy
  Real dalphar = 0.0;

  for (std::size_t i = 0; i < _t1.size(); ++i)
    dalphar += _N1[i] * _d1[i] * _t1[i] * std::pow(delta, _d1[i]) * std::pow(tau, _t1[i]);

  for (std::size_t i = 0; i < _t2.size(); ++i)
    dalphar += _N2[i] * _t2[i] * std::pow(delta, _d2[i]) * std::pow(tau, _t2[i]) *
               std::exp(-delta) * (_d2[i] - delta);

  for (std::size_t i = 0; i < _t3.size(); ++i)
    dalphar += _N3[i] * std::pow(delta, _d3[i]) * std::pow(tau, _t3[i]) *
               std::exp(_phi3[i] * Utility::pow<2>(delta - _D3[i]) +
                        _beta3[i] * Utility::pow<2>(tau - _gamma3[i])) *
               (_d3[i] + delta * (2.0 * _phi3[i] * (delta - _D3[i]))) *
               (_t3[i] + 2.0 * _beta3[i] * tau * (tau - _gamma3[i]));

  // The Helmholtz free energy is the sum of these two
  return dalphar / delta / tau;
}
