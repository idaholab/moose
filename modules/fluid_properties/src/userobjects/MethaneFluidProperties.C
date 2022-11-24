//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MethaneFluidProperties.h"
#include "MathUtils.h"
#include "libmesh/utility.h"

registerMooseObject("FluidPropertiesApp", MethaneFluidProperties);

InputParameters
MethaneFluidProperties::validParams()
{
  InputParameters params = HelmholtzFluidProperties::validParams();
  params.addClassDescription("Fluid properties for methane (CH4)");
  return params;
}

MethaneFluidProperties::MethaneFluidProperties(const InputParameters & parameters)
  : HelmholtzFluidProperties(parameters),
    _Mch4(16.0425e-3),
    _p_critical(4.5992e6),
    _T_critical(190.564),
    _rho_critical(162.66),
    _p_triple(1.169e4),
    _T_triple(90.6941)
{
}

MethaneFluidProperties::~MethaneFluidProperties() {}

std::string
MethaneFluidProperties::fluidName() const
{
  return "methane";
}

Real
MethaneFluidProperties::molarMass() const
{
  return _Mch4;
}

Real
MethaneFluidProperties::criticalPressure() const
{
  return _p_critical;
}

Real
MethaneFluidProperties::criticalTemperature() const
{
  return _T_critical;
}

Real
MethaneFluidProperties::criticalDensity() const
{
  return _rho_critical;
}

Real
MethaneFluidProperties::triplePointPressure() const
{
  return _p_triple;
}

Real
MethaneFluidProperties::triplePointTemperature() const
{
  return _T_triple;
}

Real
MethaneFluidProperties::mu_from_p_T(Real /*pressure*/, Real temperature) const
{
  // Check the temperature is in the range of validity (200 K <= T <= 1000 K)
  if (temperature <= 200.0 || temperature >= 1000.0)
    mooseException(
        "Temperature ", temperature, "K out of range (200K, 1000K) in ", name(), ": mu_from_p_T()");

  Real viscosity = 0.0;
  for (std::size_t i = 0; i < _a.size(); ++i)
    viscosity += _a[i] * MathUtils::pow(temperature, i);

  return viscosity * 1.e-6;
}

void
MethaneFluidProperties::mu_from_p_T(
    Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{

  mu = this->mu_from_p_T(pressure, temperature);
  dmu_dp = 0.0;

  Real dmudt = 0.0;
  for (std::size_t i = 0; i < _a.size(); ++i)
    dmudt += i * _a[i] * MathUtils::pow(temperature, i) / temperature;
  dmu_dT = dmudt * 1.e-6;
}

Real
MethaneFluidProperties::k_from_p_T(Real /*pressure*/, Real temperature) const
{
  // Check the temperature is in the range of validity (200 K <= T <= 1000 K)
  if (temperature <= 200.0 || temperature >= 1000.0)
    mooseException(
        "Temperature ", temperature, "K out of range (200K, 1000K) in ", name(), ": k()");

  Real kt = 0.0;
  for (std::size_t i = 0; i < _b.size(); ++i)
    kt += _b[i] * MathUtils::pow(temperature, i);

  return kt;
}

void
MethaneFluidProperties::k_from_p_T(
    Real /*pressure*/, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const
{
  // Check the temperature is in the range of validity (200 K <= T <= 1000 K)
  if (temperature <= 200.0 || temperature >= 1000.0)
    mooseException(
        "Temperature ", temperature, "K out of range (200K, 1000K) in ", name(), ": k()");

  Real kt = 0.0, dkt_dT = 0.0;

  for (std::size_t i = 0; i < _b.size(); ++i)
    kt += _b[i] * MathUtils::pow(temperature, i);

  for (std::size_t i = 1; i < _b.size(); ++i)
    dkt_dT += i * _b[i] * MathUtils::pow(temperature, i) / temperature;

  k = kt;
  dk_dp = 0.0;
  dk_dT = dkt_dT;
}

Real
MethaneFluidProperties::vaporPressure(Real temperature) const
{
  if (temperature < _T_triple || temperature > _T_critical)
    mooseException("Temperature is out of range in ", name(), ": vaporPressure()");

  const Real Tr = temperature / _T_critical;
  const Real theta = 1.0 - Tr;

  const Real logpressure = (-6.036219 * theta + 1.409353 * std::pow(theta, 1.5) -
                            0.4945199 * Utility::pow<2>(theta) - 1.443048 * std::pow(theta, 4.5)) /
                           Tr;

  return _p_critical * std::exp(logpressure);
}

void
MethaneFluidProperties::vaporPressure(Real, Real &, Real &) const
{
  mooseError("vaporPressure() is not implemented");
}

std::vector<Real>
MethaneFluidProperties::henryCoefficients() const
{
  return {-10.44708, 4.66491, 12.12986};
}

Real
MethaneFluidProperties::saturatedLiquidDensity(Real temperature) const
{
  if (temperature < _T_triple || temperature > _T_critical)
    mooseException(
        "Temperature ", temperature, " is out of range in ", name(), ": saturatedLiquidDensity()");

  const Real Tr = temperature / _T_critical;
  const Real theta = 1.0 - Tr;

  const Real logdensity = 1.9906389 * std::pow(theta, 0.354) - 0.78756197 * std::sqrt(theta) +
                          0.036976723 * std::pow(theta, 2.5);

  return _rho_critical * std::exp(logdensity);
}

Real
MethaneFluidProperties::saturatedVaporDensity(Real temperature) const
{
  if (temperature < _T_triple || temperature > _T_critical)
    mooseException(
        "Temperature ", temperature, " is out of range in ", name(), ": saturatedVaporDensity()");

  const Real Tr = temperature / _T_critical;
  const Real theta = 1.0 - Tr;

  const Real logdensity =
      -1.880284 * std::pow(theta, 0.354) - 2.8526531 * std::pow(theta, 5.0 / 6.0) -
      3.000648 * std::pow(theta, 1.5) - 5.251169 * std::pow(theta, 2.5) -
      13.191859 * std::pow(theta, 25.0 / 6.0) - 37.553961 * std::pow(theta, 47.0 / 6.0);

  return _rho_critical * std::exp(logdensity);
}

Real
MethaneFluidProperties::alpha(Real delta, Real tau) const
{
  // Ideal gas component of the Helmholtz free energy
  Real alpha0 = std::log(delta) + 9.91243972 - 6.33270087 * tau + 3.0016 * std::log(tau);

  for (std::size_t i = 0; i < _a0.size(); ++i)
    alpha0 += _a0[i] * std::log(1.0 - std::exp(-_b0[i] * tau));

  // Residual component of the Helmholtz free energy
  Real alphar = 0.0;

  for (std::size_t i = 0; i < _t1.size(); ++i)
    alphar += _N1[i] * MathUtils::pow(delta, _d1[i]) * std::pow(tau, _t1[i]);

  for (std::size_t i = 0; i < _t2.size(); ++i)
    alphar += _N2[i] * MathUtils::pow(delta, _d2[i]) * std::pow(tau, _t2[i]) *
              std::exp(-MathUtils::pow(delta, _c2[i]));

  for (std::size_t i = 0; i < _t3.size(); ++i)
    alphar += _N3[i] * MathUtils::pow(delta, _d3[i]) * std::pow(tau, _t3[i]) *
              std::exp(-_alpha3[i] * Utility::pow<2>(delta - _D3[i]) -
                       _beta3[i] * Utility::pow<2>(tau - _gamma3[i]));

  // The Helmholtz free energy is the sum of these two
  return alpha0 + alphar;
}

Real
MethaneFluidProperties::dalpha_ddelta(Real delta, Real tau) const
{
  // Ideal gas component of the Helmholtz free energy
  Real dalpha0 = 1.0 / delta;

  // Residual component of the Helmholtz free energy
  Real dalphar = 0.0;

  for (std::size_t i = 0; i < _t1.size(); ++i)
    dalphar += _N1[i] * _d1[i] * MathUtils::pow(delta, _d1[i]) * std::pow(tau, _t1[i]);

  for (std::size_t i = 0; i < _t2.size(); ++i)
    dalphar += _N2[i] * MathUtils::pow(delta, _d2[i]) * std::pow(tau, _t2[i]) *
               std::exp(-MathUtils::pow(delta, _c2[i])) *
               (_d2[i] - _c2[i] * MathUtils::pow(delta, _c2[i]));

  for (std::size_t i = 0; i < _t3.size(); ++i)
    dalphar += _N3[i] * MathUtils::pow(delta, _d3[i]) * std::pow(tau, _t3[i]) *
               std::exp(-_alpha3[i] * Utility::pow<2>(delta - _D3[i]) -
                        _beta3[i] * Utility::pow<2>(tau - _gamma3[i])) *
               (_d3[i] - delta * (2.0 * _alpha3[i] * (delta - _D3[i])));

  // The Helmholtz free energy is the sum of these two
  return dalpha0 + dalphar / delta;
}

Real
MethaneFluidProperties::dalpha_dtau(Real delta, Real tau) const
{
  // Ideal gas component of the Helmholtz free energy
  Real dalpha0 = -6.33270087 + 3.0016 / tau;

  for (std::size_t i = 0; i < _a0.size(); ++i)
    dalpha0 += _a0[i] * _b0[i] * (1.0 / (1.0 - std::exp(-_b0[i] * tau)) - 1.0);

  // Residual component of the Helmholtz free energy
  Real dalphar = 0.0;

  for (std::size_t i = 0; i < _t1.size(); ++i)
    dalphar += _N1[i] * _t1[i] * MathUtils::pow(delta, _d1[i]) * std::pow(tau, _t1[i]);

  for (std::size_t i = 0; i < _t2.size(); ++i)
    dalphar += _N2[i] * _t2[i] * MathUtils::pow(delta, _d2[i]) * std::pow(tau, _t2[i]) *
               std::exp(-MathUtils::pow(delta, _c2[i]));

  for (std::size_t i = 0; i < _t3.size(); ++i)
    dalphar += _N3[i] * MathUtils::pow(delta, _d3[i]) * std::pow(tau, _t3[i]) *
               std::exp(-_alpha3[i] * Utility::pow<2>(delta - _D3[i]) -
                        _beta3[i] * Utility::pow<2>(tau - _gamma3[i])) *
               (_t3[i] + tau * (2.0 * _beta3[i] * (tau - _gamma3[i])));

  // The Helmholtz free energy is the sum of these two
  return dalpha0 + dalphar / tau;
}

Real
MethaneFluidProperties::d2alpha_ddelta2(Real delta, Real tau) const
{
  // Ideal gas component of the Helmholtz free energy
  Real dalpha0 = -1.0 / delta / delta;

  // Residual component of the Helmholtz free energy
  Real dalphar = 0.0;

  for (std::size_t i = 0; i < _t1.size(); ++i)
    dalphar +=
        _N1[i] * _d1[i] * (_d1[i] - 1.0) * MathUtils::pow(delta, _d1[i]) * std::pow(tau, _t1[i]);

  for (std::size_t i = 0; i < _t2.size(); ++i)
    dalphar += _N2[i] * MathUtils::pow(delta, _d2[i]) * std::pow(tau, _t2[i]) *
               std::exp(-MathUtils::pow(delta, _c2[i])) *
               ((_d2[i] - _c2[i] * MathUtils::pow(delta, _c2[i])) *
                    (_d2[i] - 1.0 - _c2[i] * MathUtils::pow(delta, _c2[i])) -
                _c2[i] * _c2[i] * MathUtils::pow(delta, _c2[i]));

  for (std::size_t i = 0; i < _t3.size(); ++i)
    dalphar += _N3[i] * std::pow(tau, _t3[i]) *
               std::exp(-_alpha3[i] * Utility::pow<2>(delta - _D3[i]) -
                        _beta3[i] * Utility::pow<2>(tau - _gamma3[i])) *
               (-2.0 * _alpha3[i] * MathUtils::pow(delta, _d3[i] + 2) +
                4.0 * _alpha3[i] * _alpha3[i] * MathUtils::pow(delta, _d3[i] + 2) *
                    MathUtils::pow(delta - _D3[i], 2) -
                4.0 * _d3[i] * _alpha3[i] * MathUtils::pow(delta, _d3[i] + 1) * (delta - _D3[i]) +
                _d3[i] * (_d3[i] - 1) * MathUtils::pow(delta, _d3[i]));

  // The Helmholtz free energy is the sum of these two
  return dalpha0 + dalphar / delta / delta;
}

Real
MethaneFluidProperties::d2alpha_dtau2(Real delta, Real tau) const
{
  // Ideal gas component of the Helmholtz free energy
  Real dalpha0 = -3.0016 / tau / tau;

  for (std::size_t i = 0; i < _a0.size(); ++i)
  {
    Real exptau = std::exp(-_b0[i] * tau);
    dalpha0 -= _a0[i] * (_b0[i] * _b0[i] * exptau / (1.0 - exptau) / (1.0 - exptau));
  }

  // Residual component of the Helmholtz free energy
  Real dalphar = 0.0;

  for (std::size_t i = 0; i < _t1.size(); ++i)
    dalphar +=
        _N1[i] * _t1[i] * (_t1[i] - 1.0) * MathUtils::pow(delta, _d1[i]) * std::pow(tau, _t1[i]);

  for (std::size_t i = 0; i < _t2.size(); ++i)
    dalphar += _N2[i] * _t2[i] * (_t2[i] - 1.0) * MathUtils::pow(delta, _d2[i]) *
               std::pow(tau, _t2[i]) * std::exp(-MathUtils::pow(delta, _c2[i]));

  for (std::size_t i = 0; i < _t3.size(); ++i)
    dalphar += _N3[i] * MathUtils::pow(delta, _d3[i]) * std::pow(tau, _t3[i]) *
               std::exp(-_alpha3[i] * Utility::pow<2>(delta - _D3[i]) -
                        _beta3[i] * Utility::pow<2>(tau - _gamma3[i])) *
               (tau * _t3[i] - _beta3[i] * tau * tau * MathUtils::pow(tau - _gamma3[i], 2) -
                _t3[i] - 2.0 * tau * tau * _beta3[i]);

  // The Helmholtz free energy is the sum of these two
  return dalpha0 + dalphar / tau / tau;
}

Real
MethaneFluidProperties::d2alpha_ddeltatau(Real delta, Real tau) const
{
  // Residual component of the Helmholtz free energy
  Real dalphar = 0.0;

  for (std::size_t i = 0; i < _t1.size(); ++i)
    dalphar += _N1[i] * _d1[i] * _t1[i] * std::pow(delta, _d1[i]) * std::pow(tau, _t1[i]);

  for (std::size_t i = 0; i < _t2.size(); ++i)
    dalphar += _N2[i] * _t2[i] * std::pow(delta, _d2[i]) * std::pow(tau, _t2[i]) *
               std::exp(-MathUtils::pow(delta, _c2[i])) *
               (_d2[i] - _c2[i] * MathUtils::pow(delta, _c2[i]));

  for (std::size_t i = 0; i < _t3.size(); ++i)
    dalphar += _N3[i] * std::pow(delta, _d3[i]) * std::pow(tau, _t3[i]) *
               std::exp(-_alpha3[i] * Utility::pow<2>(delta - _D3[i]) -
                        _beta3[i] * Utility::pow<2>(tau - _gamma3[i])) *
               (_d3[i] - 2.0 * _alpha3[i] * delta * (delta - _D3[i]) *
                             (_t3[i] - 2.0 * _beta3[i] * tau * (tau - _gamma3[i])));

  // The Helmholtz free energy is the sum of these two
  return dalphar / delta / tau;
}
