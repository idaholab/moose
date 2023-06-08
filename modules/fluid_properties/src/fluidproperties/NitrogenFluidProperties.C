//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NitrogenFluidProperties.h"
#include "Conversion.h"
#include "MathUtils.h"
#include "libmesh/utility.h"

registerMooseObject("FluidPropertiesApp", NitrogenFluidProperties);

InputParameters
NitrogenFluidProperties::validParams()
{
  InputParameters params = HelmholtzFluidProperties::validParams();
  params.addClassDescription("Fluid properties for Nitrogen (N2)");
  return params;
}

NitrogenFluidProperties::NitrogenFluidProperties(const InputParameters & parameters)
  : HelmholtzFluidProperties(parameters),
    _Mn2(28.01348e-3),
    _p_critical(3.3958e6),
    _T_critical(126.192),
    _rho_molar_critical(11.1839),
    _rho_critical(313.3),
    _p_triple(12.523e3),
    _T_triple(63.151)
{
}

std::string
NitrogenFluidProperties::fluidName() const
{
  return "nitrogen";
}

Real
NitrogenFluidProperties::molarMass() const
{
  return _Mn2;
}

Real
NitrogenFluidProperties::criticalPressure() const
{
  return _p_critical;
}

Real
NitrogenFluidProperties::criticalTemperature() const
{
  return _T_critical;
}

Real
NitrogenFluidProperties::criticalDensity() const
{
  return _rho_critical;
}

Real
NitrogenFluidProperties::triplePointPressure() const
{
  return _p_triple;
}

Real
NitrogenFluidProperties::triplePointTemperature() const
{
  return _T_triple;
}

Real
NitrogenFluidProperties::mu_from_rho_T(Real density, Real temperature) const
{
  // Scale the input density and temperature
  const Real delta = density / _rho_critical;
  const Real tau = _T_critical / temperature;
  const Real logTstar = std::log(temperature / 98.94);

  // The dilute gas component
  Real logOmega = 0.0;
  for (std::size_t i = 0; i < _bmu.size(); ++i)
    logOmega += _bmu[i] * MathUtils::pow(logTstar, i);

  const Real mu0 =
      0.0266958 * std::sqrt(1000.0 * _Mn2 * temperature) / (0.3656 * 0.3656 * std::exp(logOmega));

  // The residual component
  Real mur = 0.0;
  for (std::size_t i = 0; i < _Nmu.size(); ++i)
    mur += _Nmu[i] * std::pow(tau, _tmu[i]) * MathUtils::pow(delta, _dmu[i]) *
           std::exp(-_gammamu[i] * MathUtils::pow(delta, _lmu[i]));

  // The viscosity in Pa.s
  return (mu0 + mur) * 1.0e-6;
}

void
NitrogenFluidProperties::mu_from_rho_T(Real density,
                                       Real temperature,
                                       Real ddensity_dT,
                                       Real & mu,
                                       Real & dmu_drho,
                                       Real & dmu_dT) const
{
  // Scale the input density and temperature
  const Real delta = density / _rho_critical;
  const Real tau = _T_critical / temperature;
  const Real logTstar = std::log(temperature / 98.94);

  // The dilute gas component
  Real logOmega = 0.0, dlogOmega_dT = 0.0;
  for (std::size_t i = 0; i < _bmu.size(); ++i)
  {
    logOmega += _bmu[i] * MathUtils::pow(logTstar, i);
    dlogOmega_dT += i * _bmu[i] * MathUtils::pow(logTstar, i) / (temperature * logTstar);
  }

  const Real mu0 =
      0.0266958 * std::sqrt(1000.0 * _Mn2 * temperature) / (0.3656 * 0.3656 * std::exp(logOmega));
  const Real dmu0_dT = 26.6958 * _Mn2 * (1.0 - 2.0 * temperature * dlogOmega_dT) *
                       std::exp(-logOmega) /
                       (2.0 * std::sqrt(1000.0 * _Mn2 * temperature) * 0.3656 * 0.3656);

  // The residual component
  Real mur = 0.0, dmur_drho = 0.0, dmur_dT = 0.0;
  Real term;
  for (std::size_t i = 0; i < _Nmu.size(); ++i)
  {
    term = _Nmu[i] * std::pow(tau, _tmu[i]) * MathUtils::pow(delta, _dmu[i]) *
           std::exp(-_gammamu[i] * MathUtils::pow(delta, _lmu[i]));
    mur += term;
    dmur_drho += (_dmu[i] - _lmu[i] * _gammamu[i] * MathUtils::pow(delta, _lmu[i])) * term / delta /
                 _rho_molar_critical / (1000.0 * _Mn2);
    dmur_dT += -_tmu[i] * term / temperature;
  }

  // The viscosity in Pa.s
  mu = (mu0 + mur) * 1.0e-6;
  dmu_drho = dmur_drho * 1.0e-6;
  dmu_dT = (dmu0_dT + dmur_dT) * 1.0e-6 + dmu_drho * ddensity_dT;
}

Real
NitrogenFluidProperties::mu_from_p_T(Real pressure, Real temperature) const
{
  // Require density first
  const Real density = rho_from_p_T(pressure, temperature);
  return mu_from_rho_T(density, temperature);
}

void
NitrogenFluidProperties::mu_from_p_T(
    Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  Real rho, drho_dp, drho_dT;
  rho_from_p_T(pressure, temperature, rho, drho_dp, drho_dT);

  Real dmu_drho;
  mu_from_rho_T(rho, temperature, drho_dT, mu, dmu_drho, dmu_dT);
  dmu_dp = dmu_drho * drho_dp;
}

void
NitrogenFluidProperties::rho_mu_from_p_T(Real pressure,
                                         Real temperature,
                                         Real & rho,
                                         Real & mu) const
{
  rho = rho_from_p_T(pressure, temperature);
  mu = mu_from_rho_T(rho, temperature);
}

void
NitrogenFluidProperties::rho_mu_from_p_T(Real pressure,
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
NitrogenFluidProperties::k_from_rho_T(Real density, Real temperature) const
{
  // Scale the input density and temperature
  const Real delta = density / _rho_critical;
  const Real tau = _T_critical / temperature;

  // The dilute gas component
  const Real lambda0 =
      1.511 * mu_from_rho_T(0.0, temperature) * 1.0e6 + 2.117 / tau - 3.332 * std::pow(tau, -0.7);

  // The residual component
  Real lambdar = 0.0;
  for (std::size_t i = 0; i < _Nk.size(); ++i)
    lambdar += _Nk[i] * std::pow(tau, _tk[i]) * MathUtils::pow(delta, _dk[i]) *
               std::exp(-_gammak[i] * MathUtils::pow(delta, _lk[i]));

  // The thermal conductivity (note: critical enhancement not implemented)
  return (lambda0 + lambdar) * 1.0e-3;
}

Real
NitrogenFluidProperties::k_from_p_T(Real pressure, Real temperature) const
{
  // Require density first
  const Real density = rho_from_p_T(pressure, temperature);
  return k_from_rho_T(density, temperature);
}

void
NitrogenFluidProperties::k_from_p_T(
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
NitrogenFluidProperties::henryCoefficients() const
{
  return {-9.67578, 4.72162, 11.70585};
}

Real
NitrogenFluidProperties::vaporPressure(Real temperature) const
{
  if (temperature < _T_triple || temperature > _T_critical)
    throw MooseException("Temperature is out of range in " + name() + ": vaporPressure()");

  const Real Tr = temperature / _T_critical;
  const Real theta = 1.0 - Tr;

  const Real logpressure =
      (-6.12445284 * theta + 1.2632722 * std::pow(theta, 1.5) - 0.765910082 * std::pow(theta, 2.5) -
       1.77570564 * Utility::pow<5>(theta)) /
      Tr;

  return _p_critical * std::exp(logpressure);
}

void
NitrogenFluidProperties::vaporPressure(Real, Real &, Real &) const
{
  mooseError("vaporPressure() is not implemented");
}

Real
NitrogenFluidProperties::saturatedLiquidDensity(Real temperature) const
{
  if (temperature < _T_triple || temperature > _T_critical)
    throw MooseException("Temperature is out of range in " + name() + ": vaporPressure()");

  const Real Tr = temperature / _T_critical;
  const Real theta = 1.0 - Tr;

  const Real logpressure =
      1.48654237 * std::pow(theta, 0.3294) - 0.280476066 * std::pow(theta, 2.0 / 3.0) +
      0.0894143085 * std::pow(theta, 8.0 / 3.0) - 0.119879866 * std::pow(theta, 35.0 / 6.0);

  return _rho_critical * std::exp(logpressure);
}

Real
NitrogenFluidProperties::saturatedVaporDensity(Real temperature) const
{
  if (temperature < _T_triple || temperature > _T_critical)
    throw MooseException("Temperature is out of range in " + name() + ": vaporPressure()");

  const Real Tr = temperature / _T_critical;
  const Real theta = 1.0 - Tr;

  const Real logpressure =
      (-1.70127164 * std::pow(theta, 0.34) - 3.70402649 * std::pow(theta, 5.0 / 6.0) +
       1.29859383 * std::pow(theta, 7.0 / 6.0) - 0.561424977 * std::pow(theta, 13.0 / 6.0) -
       2.68505381 * std::pow(theta, 14.0 / 3.0)) /
      Tr;

  return _rho_critical * std::exp(logpressure);
}

Real
NitrogenFluidProperties::alpha(Real delta, Real tau) const
{
  // Ideal gas component of the Helmholtz free energy
  const Real alpha0 = std::log(delta) + _a[0] * std::log(tau) + _a[1] + _a[2] * tau + _a[3] / tau +
                      _a[4] / Utility::pow<2>(tau) + _a[5] / Utility::pow<3>(tau) +
                      _a[6] * std::log(1.0 - std::exp(-_a[7] * tau));

  // Residual component of the Helmholtz free energy
  Real alphar = 0.0;

  for (std::size_t i = 0; i < _N1.size(); ++i)
    alphar += _N1[i] * MathUtils::pow(delta, _i1[i]) * std::pow(tau, _j1[i]);

  for (std::size_t i = 0; i < _N2.size(); ++i)
    alphar += _N2[i] * MathUtils::pow(delta, _i2[i]) * std::pow(tau, _j2[i]) *
              std::exp(-MathUtils::pow(delta, _l2[i]));

  for (std::size_t i = 0; i < _N3.size(); ++i)
    alphar += _N3[i] * MathUtils::pow(delta, _i3[i]) * std::pow(tau, _j3[i]) *
              std::exp(-_phi3[i] * Utility::pow<2>(delta - 1.0) -
                       _beta3[i] * Utility::pow<2>(tau - _gamma3[i]));

  // The Helmholtz free energy is the sum of these two
  return alpha0 + alphar;
}

Real
NitrogenFluidProperties::dalpha_ddelta(Real delta, Real tau) const
{
  // Ideal gas component of the Helmholtz free energy
  const Real dalpha0 = 1.0 / delta;

  // Residual component of the Helmholtz free energy
  Real dalphar = 0.0;

  for (std::size_t i = 0; i < _N1.size(); ++i)
    dalphar += _N1[i] * _i1[i] * MathUtils::pow(delta, _i1[i]) * std::pow(tau, _j1[i]);

  for (std::size_t i = 0; i < _N2.size(); ++i)
    dalphar += _N2[i] * MathUtils::pow(delta, _i2[i]) * std::pow(tau, _j2[i]) *
               std::exp(-MathUtils::pow(delta, _l2[i])) *
               (_i2[i] - _l2[i] * MathUtils::pow(delta, _l2[i]));

  for (std::size_t i = 0; i < _N3.size(); ++i)
    dalphar += _N3[i] * MathUtils::pow(delta, _i3[i]) * std::pow(tau, _j3[i]) *
               std::exp(-_phi3[i] * Utility::pow<2>(delta - 1.0) -
                        _beta3[i] * Utility::pow<2>(tau - _gamma3[i])) *
               (_i3[i] - 2.0 * delta * _phi3[i] * (delta - 1.0));

  // The Helmholtz free energy is the sum of these two
  return dalpha0 + dalphar / delta;
}

Real
NitrogenFluidProperties::dalpha_dtau(Real delta, Real tau) const
{
  // Ideal gas component of the Helmholtz free energy
  const Real dalpha0 = _a[0] + _a[2] * tau - _a[3] / tau - 2.0 * _a[4] / Utility::pow<2>(tau) -
                       3.0 * _a[5] / Utility::pow<3>(tau) +
                       _a[6] * _a[7] * tau / (std::exp(_a[7] * tau) - 1.0);

  // Residual component of the Helmholtz free energy
  Real dalphar = 0.0;

  for (std::size_t i = 0; i < _N1.size(); ++i)
    dalphar += _N1[i] * _j1[i] * MathUtils::pow(delta, _i1[i]) * std::pow(tau, _j1[i]);

  for (std::size_t i = 0; i < _N2.size(); ++i)
    dalphar += _N2[i] * _j2[i] * MathUtils::pow(delta, _i2[i]) * std::pow(tau, _j2[i]) *
               std::exp(-MathUtils::pow(delta, _l2[i]));

  for (std::size_t i = 0; i < _N3.size(); ++i)
    dalphar += _N3[i] * MathUtils::pow(delta, _i3[i]) * std::pow(tau, _j3[i]) *
               std::exp(-_phi3[i] * Utility::pow<2>(delta - 1.0) -
                        _beta3[i] * Utility::pow<2>(tau - _gamma3[i])) *
               (_j3[i] - 2.0 * tau * _beta3[i] * (tau - _gamma3[i]));

  // The Helmholtz free energy is the sum of these two
  return (dalpha0 + dalphar) / tau;
}

Real
NitrogenFluidProperties::d2alpha_ddelta2(Real delta, Real tau) const
{
  // Ideal gas component of the Helmholtz free energy
  const Real dalpha0 = -1.0 / delta / delta;

  // Residual component of the Helmholtz free energy
  Real dalphar = 0.0;

  for (std::size_t i = 0; i < _N1.size(); ++i)
    dalphar +=
        _N1[i] * _i1[i] * (_i1[i] - 1.0) * MathUtils::pow(delta, _i1[i]) * std::pow(tau, _j1[i]);

  for (std::size_t i = 0; i < _N2.size(); ++i)
    dalphar += _N2[i] * MathUtils::pow(delta, _i2[i]) * std::pow(tau, _j2[i]) *
               std::exp(-MathUtils::pow(delta, _l2[i])) *
               ((_i2[i] - _l2[i] * MathUtils::pow(delta, _l2[i])) *
                    (_i2[i] - 1.0 - _l2[i] * MathUtils::pow(delta, _l2[i])) -
                _l2[i] * _l2[i] * MathUtils::pow(delta, _l2[i]));

  for (std::size_t i = 0; i < _N3.size(); ++i)
    dalphar += _N3[i] * MathUtils::pow(delta, _i3[i]) * std::pow(tau, _j3[i]) *
               std::exp(-_phi3[i] * Utility::pow<2>(delta - 1.0) -
                        _beta3[i] * Utility::pow<2>(tau - _gamma3[i])) *
               (Utility::pow<2>(_i3[i] - 2.0 * delta * _phi3[i] * (delta - 1.0)) - _i3[i] -
                2.0 * delta * delta * _phi3[i]);

  // The Helmholtz free energy
  return dalpha0 + dalphar / delta / delta;
}

Real
NitrogenFluidProperties::d2alpha_dtau2(Real delta, Real tau) const
{
  // Ideal gas component of the Helmholtz free energy
  const Real dalpha0 = -_a[0] + 2.0 * _a[3] / tau + 6.0 * _a[4] / Utility::pow<2>(tau) +
                       12.0 * _a[5] / Utility::pow<3>(tau) -
                       _a[6] * _a[7] * _a[7] * tau * tau * std::exp(_a[7] * tau) /
                           Utility::pow<2>(std::exp(_a[7] * tau) - 1.0);

  // Residual component of the Helmholtz free energy
  Real dalphar = 0.0;

  for (std::size_t i = 0; i < _N1.size(); ++i)
    dalphar +=
        _N1[i] * _j1[i] * (_j1[i] - 1.0) * MathUtils::pow(delta, _i1[i]) * std::pow(tau, _j1[i]);

  for (std::size_t i = 0; i < _N2.size(); ++i)
    dalphar += _N2[i] * _j2[i] * (_j2[i] - 1.0) * MathUtils::pow(delta, _i2[i]) *
               std::pow(tau, _j2[i]) * std::exp(-MathUtils::pow(delta, _l2[i]));

  for (std::size_t i = 0; i < _N3.size(); ++i)
    dalphar += _N3[i] * MathUtils::pow(delta, _i3[i]) * std::pow(tau, _j3[i]) *
               std::exp(-_phi3[i] * Utility::pow<2>(delta - 1.0) -
                        _beta3[i] * Utility::pow<2>(tau - _gamma3[i])) *
               (Utility::pow<2>(_j3[i] - 2.0 * tau * _beta3[i] * (tau - _gamma3[i])) - _j3[i] -
                2.0 * tau * tau * _beta3[i]);

  // The Helmholtz free energy is the sum of these two
  return (dalpha0 + dalphar) / tau / tau;
}

Real
NitrogenFluidProperties::d2alpha_ddeltatau(Real delta, Real tau) const
{
  // Residual component of the Helmholtz free energy (second derivative of ideal
  // component wrt delta and tau is 0)
  Real dalphar = 0.0;

  for (std::size_t i = 0; i < _N1.size(); ++i)
    dalphar += _N1[i] * _i1[i] * _j1[i] * MathUtils::pow(delta, _i1[i]) * std::pow(tau, _j1[i]);

  for (std::size_t i = 0; i < _N2.size(); ++i)
    dalphar += _N2[i] * _j2[i] * MathUtils::pow(delta, _i2[i]) * std::pow(tau, _j2[i]) *
               std::exp(-MathUtils::pow(delta, _l2[i])) *
               (_i2[i] - _l2[i] * MathUtils::pow(delta, _l2[i]));

  for (std::size_t i = 0; i < _N3.size(); ++i)
    dalphar += _N3[i] * MathUtils::pow(delta, _i3[i]) * std::pow(tau, _j3[i]) *
               std::exp(-_phi3[i] * Utility::pow<2>(delta - 1.0) -
                        _beta3[i] * Utility::pow<2>(tau - _gamma3[i])) *
               (_i3[i] - 2.0 * delta * _phi3[i] * (delta - 1.0)) *
               (_j3[i] - 2.0 * tau * _beta3[i] * (tau - _gamma3[i]));

  // The Helmholtz free energy
  return dalphar / delta / tau;
}
