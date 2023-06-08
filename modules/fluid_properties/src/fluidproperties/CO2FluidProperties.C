//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CO2FluidProperties.h"
#include "BrentsMethod.h"
#include "Conversion.h"
#include "MathUtils.h"
#include "libmesh/utility.h"

registerMooseObject("FluidPropertiesApp", CO2FluidProperties);

InputParameters
CO2FluidProperties::validParams()
{
  InputParameters params = HelmholtzFluidProperties::validParams();
  params.addClassDescription(
      "Fluid properties for carbon dioxide (CO2) using the Span & Wagner EOS");
  return params;
}

CO2FluidProperties::CO2FluidProperties(const InputParameters & parameters)
  : HelmholtzFluidProperties(parameters)
{
}

CO2FluidProperties::~CO2FluidProperties() {}

std::string
CO2FluidProperties::fluidName() const
{
  return "co2";
}

Real
CO2FluidProperties::molarMass() const
{
  return _Mco2;
}

Real
CO2FluidProperties::criticalPressure() const
{
  return _critical_pressure;
}

Real
CO2FluidProperties::criticalTemperature() const
{
  return _critical_temperature;
}

Real
CO2FluidProperties::criticalDensity() const
{
  return _critical_density;
}

Real
CO2FluidProperties::triplePointPressure() const
{
  return _triple_point_pressure;
}

Real
CO2FluidProperties::triplePointTemperature() const
{
  return _triple_point_temperature;
}

Real
CO2FluidProperties::meltingPressure(Real temperature) const
{
  if (temperature < _triple_point_temperature)
    throw MooseException("Temperature is below the triple point temperature in " + name() +
                         ": meltingPressure()");

  Real Tstar = temperature / _triple_point_temperature;

  return _triple_point_pressure *
         (1.0 + 1955.539 * (Tstar - 1.0) + 2055.4593 * Utility::pow<2>(Tstar - 1.0));
}

Real
CO2FluidProperties::sublimationPressure(Real temperature) const
{
  if (temperature > _triple_point_temperature)
    throw MooseException("Temperature is above the triple point temperature in " + name() +
                         ": sublimationPressure()");

  Real Tstar = temperature / _triple_point_temperature;

  Real pressure = _triple_point_pressure *
                  std::exp((-14.740846 * (1.0 - Tstar) + 2.4327015 * std::pow(1.0 - Tstar, 1.9) -
                            5.3061778 * std::pow(1.0 - Tstar, 2.9)) /
                           Tstar);

  return pressure;
}

Real
CO2FluidProperties::vaporPressure(Real temperature) const
{
  if (temperature < _triple_point_temperature || temperature > _critical_temperature)
    throw MooseException("Temperature is out of range in " + name() + ": vaporPressure()");

  Real Tstar = temperature / _critical_temperature;

  Real logpressure =
      (-7.0602087 * (1.0 - Tstar) + 1.9391218 * std::pow(1.0 - Tstar, 1.5) -
       1.6463597 * Utility::pow<2>(1.0 - Tstar) - 3.2995634 * Utility::pow<4>(1.0 - Tstar)) /
      Tstar;

  return _critical_pressure * std::exp(logpressure);
}

void
CO2FluidProperties::vaporPressure(Real, Real &, Real &) const
{
  mooseError("vaporPressure() is not implemented");
}

Real
CO2FluidProperties::saturatedLiquidDensity(Real temperature) const
{
  if (temperature < _triple_point_temperature || temperature > _critical_temperature)
    throw MooseException("Temperature is out of range in " + name() + ": saturatedLiquiDensity()");

  Real Tstar = temperature / _critical_temperature;

  Real logdensity = 1.9245108 * std::pow(1.0 - Tstar, 0.34) -
                    0.62385555 * std::pow(1.0 - Tstar, 0.5) -
                    0.32731127 * std::pow(1.0 - Tstar, 10.0 / 6.0) +
                    0.39245142 * std::pow(1.0 - Tstar, 11.0 / 6.0);

  return _critical_density * std::exp(logdensity);
}

Real
CO2FluidProperties::saturatedVaporDensity(Real temperature) const
{
  if (temperature < _triple_point_temperature || temperature > _critical_temperature)
    throw MooseException("Temperature is out of range in " + name() + ": saturatedVaporDensity()");

  Real Tstar = temperature / _critical_temperature;

  Real logdensity =
      (-1.7074879 * std::pow(1.0 - Tstar, 0.34) - 0.82274670 * std::pow(1.0 - Tstar, 0.5) -
       4.6008549 * (1.0 - Tstar) - 10.111178 * std::pow(1.0 - Tstar, 7.0 / 3.0) -
       29.742252 * std::pow(1.0 - Tstar, 14.0 / 3.0));

  return _critical_density * std::exp(logdensity);
}

Real
CO2FluidProperties::alpha(Real delta, Real tau) const
{
  // Ideal gas component of the Helmholtz free energy
  Real sum0 = 0.0;
  for (std::size_t i = 0; i < _a0.size(); ++i)
    sum0 += _a0[i] * std::log(1.0 - std::exp(-_theta0[i] * tau));

  Real phi0 = std::log(delta) + 8.37304456 - 3.70454304 * tau + 2.5 * std::log(tau) + sum0;

  // Residual component of the Helmholtz free energy
  Real theta, Delta, Psi;
  Real phir = 0.0;
  for (std::size_t i = 0; i < _n1.size(); ++i)
    phir += _n1[i] * MathUtils::pow(delta, _d1[i]) * std::pow(tau, _t1[i]);

  for (std::size_t i = 0; i < _n2.size(); ++i)
    phir += _n2[i] * MathUtils::pow(delta, _d2[i]) * std::pow(tau, _t2[i]) *
            std::exp(-MathUtils::pow(delta, _c2[i]));

  for (std::size_t i = 0; i < _n3.size(); ++i)
    phir += _n3[i] * MathUtils::pow(delta, _d3[i]) * MathUtils::pow(tau, _t3[i]) *
            std::exp(-_alpha3[i] * Utility::pow<2>(delta - _eps3[i]) -
                     _beta3[i] * Utility::pow<2>(tau - _gamma3[i]));

  for (std::size_t i = 0; i < _n4.size(); ++i)
  {
    theta = 1.0 - tau + _A4[i] * std::pow(Utility::pow<2>(delta - 1.0), 1.0 / (2.0 * _beta4[i]));
    Delta = Utility::pow<2>(theta) + _B4[i] * std::pow(Utility::pow<2>(delta - 1.0), _a4[i]);
    Psi = std::exp(-_C4[i] * Utility::pow<2>(delta - 1.0) - _D4[i] * Utility::pow<2>(tau - 1.0));
    phir += _n4[i] * std::pow(Delta, _b4[i]) * delta * Psi;
  }

  // The Helmholtz free energy is the sum of these components
  return phi0 + phir;
}

Real
CO2FluidProperties::dalpha_ddelta(Real delta, Real tau) const
{
  // Derivative of the ideal gas component wrt gamma
  Real dphi0dd = 1.0 / delta;

  // Derivative of the residual component wrt gamma
  Real theta, Delta, Psi, dDelta_dd, dPsi_dd;
  Real dphirdd = 0.0;

  for (std::size_t i = 0; i < _n1.size(); ++i)
    dphirdd += _n1[i] * _d1[i] * MathUtils::pow(delta, _d1[i] - 1.0) * std::pow(tau, _t1[i]);

  for (std::size_t i = 0; i < _n2.size(); ++i)
    dphirdd += _n2[i] * std::exp(-MathUtils::pow(delta, _c2[i])) *
               (MathUtils::pow(delta, _d2[i] - 1.0) * std::pow(tau, _t2[i]) *
                (_d2[i] - _c2[i] * MathUtils::pow(delta, _c2[i])));

  for (std::size_t i = 0; i < _n3.size(); ++i)
    dphirdd += _n3[i] * MathUtils::pow(delta, _d3[i]) * MathUtils::pow(tau, _t3[i]) *
               std::exp(-_alpha3[i] * Utility::pow<2>(delta - _eps3[i]) -
                        _beta3[i] * Utility::pow<2>(tau - _gamma3[i])) *
               (_d3[i] / delta - 2.0 * _alpha3[i] * (delta - _eps3[i]));

  for (std::size_t i = 0; i < _n4.size(); ++i)
  {
    theta = 1.0 - tau + _A4[i] * std::pow(Utility::pow<2>(delta - 1.0), 1.0 / (2.0 * _beta4[i]));
    Delta = Utility::pow<2>(theta) + _B4[i] * std::pow(Utility::pow<2>(delta - 1.0), _a4[i]);
    Psi = std::exp(-_C4[i] * Utility::pow<2>(delta - 1.0) - _D4[i] * Utility::pow<2>(tau - 1.0));
    dPsi_dd = -2.0 * _C4[i] * (delta - 1.0) * Psi;
    dDelta_dd = (delta - 1.0) *
                (_A4[i] * theta * 2.0 / _beta4[i] *
                     std::pow(Utility::pow<2>(delta - 1.0), 1.0 / (2.0 * _beta4[i]) - 1.0) +
                 2.0 * _B4[i] * _a4[i] * std::pow(Utility::pow<2>(delta - 1.0), _a4[i] - 1.0));

    dphirdd += _n4[i] * (std::pow(Delta, _b4[i]) * (Psi + delta * dPsi_dd) +
                         _b4[i] * std::pow(Delta, _b4[i] - 1.0) * dDelta_dd * delta * Psi);
  }

  // The derivative of the free energy wrt delta is the sum of these components
  return dphi0dd + dphirdd;
}

Real
CO2FluidProperties::dalpha_dtau(Real delta, Real tau) const
{
  // Derivative of the ideal gas component wrt tau
  Real sum0 = 0.0;
  for (std::size_t i = 0; i < _a0.size(); ++i)
    sum0 += _a0[i] * _theta0[i] * (1.0 / (1.0 - std::exp(-_theta0[i] * tau)) - 1.0);

  Real dphi0dt = -3.70454304 + 2.5 / tau + sum0;

  // Derivative of the residual component wrt tau
  Real theta, Delta, Psi, dDelta_dt, dPsi_dt;
  Real dphirdt = 0.0;
  for (std::size_t i = 0; i < _n1.size(); ++i)
    dphirdt += _n1[i] * _t1[i] * MathUtils::pow(delta, _d1[i]) * std::pow(tau, _t1[i] - 1.0);

  for (std::size_t i = 0; i < _n2.size(); ++i)
    dphirdt += _n2[i] * _t2[i] * MathUtils::pow(delta, _d2[i]) * std::pow(tau, _t2[i] - 1.0) *
               std::exp(-MathUtils::pow(delta, _c2[i]));

  for (std::size_t i = 0; i < _n3.size(); ++i)
    dphirdt += _n3[i] * MathUtils::pow(delta, _d3[i]) * MathUtils::pow(tau, _t3[i]) *
               std::exp(-_alpha3[i] * Utility::pow<2>(delta - _eps3[i]) -
                        _beta3[i] * Utility::pow<2>(tau - _gamma3[i])) *
               (_t3[i] / tau - 2.0 * _beta3[i] * (tau - _gamma3[i]));

  for (std::size_t i = 0; i < _n4.size(); ++i)
  {
    theta = 1.0 - tau + _A4[i] * std::pow(Utility::pow<2>(delta - 1.0), 1.0 / (2.0 * _beta4[i]));
    Delta = Utility::pow<2>(theta) + _B4[i] * std::pow(Utility::pow<2>(delta - 1.0), _a4[i]);
    Psi = std::exp(-_C4[i] * Utility::pow<2>(delta - 1.0) - _D4[i] * Utility::pow<2>(tau - 1.0));
    dDelta_dt = -2.0 * theta * _b4[i] * std::pow(Delta, _b4[i] - 1.0);
    dPsi_dt = -2.0 * _D4[i] * (tau - 1.0) * Psi;

    dphirdt += _n4[i] * delta * (Psi * dDelta_dt + std::pow(Delta, _b4[i]) * dPsi_dt);
  }

  // The derivative of the free energy wrt tau is the sum of these components
  return dphi0dt + dphirdt;
}

Real
CO2FluidProperties::d2alpha_ddelta2(Real delta, Real tau) const
{
  // Second derivative of the ideal gas component wrt gamma
  Real d2phi0dd2 = -1.0 / delta / delta;

  // Second derivative of the residual component wrt gamma
  Real d2phirdd2 = 0.0;
  Real theta, Delta, Psi, dDelta_dd, dPsi_dd, d2Delta_dd2, d2Psi_dd2;

  for (std::size_t i = 0; i < _n1.size(); ++i)
    d2phirdd2 += _n1[i] * _d1[i] * (_d1[i] - 1.0) * MathUtils::pow(delta, _d1[i] - 2) *
                 std::pow(tau, _t1[i]);

  for (std::size_t i = 0; i < _n2.size(); ++i)
    d2phirdd2 += _n2[i] * std::exp(-MathUtils::pow(delta, _c2[i])) *
                 MathUtils::pow(delta, _d2[i] - 2) * std::pow(tau, _t2[i]) *
                 ((_d2[i] - _c2[i] * MathUtils::pow(delta, _c2[i])) *
                      (_d2[i] - 1.0 - _c2[i] * MathUtils::pow(delta, _c2[i])) -
                  _c2[i] * _c2[i] * MathUtils::pow(delta, _c2[i]));

  for (std::size_t i = 0; i < _n3.size(); ++i)
    d2phirdd2 +=
        _n3[i] * MathUtils::pow(tau, _t3[i]) *
        std::exp(-_alpha3[i] * Utility::pow<2>(delta - _eps3[i]) -
                 _beta3[i] * Utility::pow<2>(tau - _gamma3[i])) *
        (-2.0 * _alpha3[i] * MathUtils::pow(delta, _d3[i]) +
         4.0 * _alpha3[i] * _alpha3[i] * MathUtils::pow(delta, _d3[i]) *
             Utility::pow<2>(delta - _eps3[i]) -
         4.0 * _d3[i] * _alpha3[i] * MathUtils::pow(delta, _d3[i] - 1.0) * (delta - _eps3[i]) +
         _d3[i] * (_d3[i] - 1.0) * MathUtils::pow(delta, _d3[i] - 2.0));

  for (std::size_t i = 0; i < _n4.size(); ++i)
  {
    theta = 1.0 - tau + _A4[i] * std::pow(Utility::pow<2>(delta - 1.0), 1.0 / (2.0 * _beta4[i]));
    Delta = Utility::pow<2>(theta) + _B4[i] * std::pow(Utility::pow<2>(delta - 1.0), _a4[i]);
    Psi = std::exp(-_C4[i] * Utility::pow<2>(delta - 1.0) - _D4[i] * Utility::pow<2>(tau - 1.0));
    dPsi_dd = -2.0 * _C4[i] * (delta - 1.0) * Psi;
    dDelta_dd = (delta - 1.0) *
                (_A4[i] * theta * 2.0 / _beta4[i] *
                     std::pow(Utility::pow<2>(delta - 1.0), 1.0 / (2.0 * _beta4[i]) - 1.0) +
                 2.0 * _B4[i] * _a4[i] * std::pow(Utility::pow<2>(delta - 1.0), _a4[i] - 1.0));
    d2Psi_dd2 = 3.0 * _D4[i] * Psi * (2.0 * _C4[i] * Utility::pow<2>(delta - 1.0) - 1.0);
    d2Delta_dd2 = 1.0 / (delta - 1.0) * dDelta_dd +
                  (delta - 1.0) * (delta - 1.0) *
                      (4.0 * _B4[i] * _a4[i] * (_a4[i] - 1.0) *
                           std::pow(Utility::pow<2>(delta - 1.0), _a4[i] - 2.0) +
                       2.0 * _A4[i] * _A4[i] *
                           Utility::pow<2>(std::pow(Utility::pow<2>(delta - 1.0),
                                                    1.0 / (2.0 * _beta4[i]) - 1.0)) /
                           _beta4[i] / _beta4[i] +
                       (4.0 / _beta4[i]) * _A4[i] * theta * (1.0 / (2.0 * _beta4[i]) - 1.0) *
                           std::pow(Utility::pow<2>(delta - 1.0), 1.0 / (2.0 * _beta4[i]) - 2.0));
    d2phirdd2 +=
        _n4[i] *
        (std::pow(Delta, _b4[i]) * (2.0 * dPsi_dd + delta * d2Psi_dd2) +
         2.0 * _b4[i] * std::pow(Delta, _b4[i] - 1.0) * dDelta_dd * (Psi + delta * dPsi_dd) +
         _b4[i] *
             (std::pow(Delta, _b4[i] - 1.0) * d2Delta_dd2 +
              (_b4[i] - 1.0) * std::pow(Delta, _b4[i] - 2.0) * Utility::pow<2>(dDelta_dd)) *
             delta * Psi);
  }
  // The second derivative of the free energy wrt delta is the sum of these components
  return d2phi0dd2 + d2phirdd2;
}

Real
CO2FluidProperties::d2alpha_dtau2(Real delta, Real tau) const
{
  // Second derivative of the ideal gas component wrt tau
  Real sum0 = 0.0;
  for (std::size_t i = 0; i < _a0.size(); ++i)
    sum0 += _a0[i] * _theta0[i] * _theta0[i] * std::exp(-_theta0[i] * tau) /
            Utility::pow<2>(1.0 - std::exp(-_theta0[i] * tau));

  Real d2phi0dt2 = -2.5 / tau / tau - sum0;

  // Second derivative of the residual component wrt tau
  Real d2phirdt2 = 0.0;
  Real theta, Delta, Psi, dPsi_dt, dDelta_dt, d2Delta_dt2, d2Psi_dt2;

  for (std::size_t i = 0; i < _n1.size(); ++i)
    d2phirdt2 += _n1[i] * _t1[i] * (_t1[i] - 1.0) * MathUtils::pow(delta, _d1[i]) *
                 std::pow(tau, _t1[i] - 2.0);

  for (std::size_t i = 0; i < _n2.size(); ++i)
    d2phirdt2 += _n2[i] * _t2[i] * (_t2[i] - 1.0) * MathUtils::pow(delta, _d2[i]) *
                 std::exp(-MathUtils::pow(delta, _c2[i])) * std::pow(tau, _t2[i] - 2.0);

  for (std::size_t i = 0; i < _n3.size(); ++i)
    d2phirdt2 += _n3[i] * MathUtils::pow(delta, _d3[i]) * MathUtils::pow(tau, _t3[i]) *
                 std::exp(-_alpha3[i] * Utility::pow<2>(delta - _eps3[i]) -
                          _beta3[i] * Utility::pow<2>(tau - _gamma3[i])) *
                 (Utility::pow<2>(_t3[i] / tau - 2.0 * _beta3[i] * (tau - _gamma3[i])) -
                  _t3[i] / tau / tau - 2.0 * _beta3[i]);

  for (std::size_t i = 0; i < _n4.size(); ++i)
  {
    theta = 1.0 - tau + _A4[i] * std::pow(Utility::pow<2>(delta - 1.0), 1.0 / (2.0 * _beta4[i]));
    Delta = Utility::pow<2>(theta) + _B4[i] * std::pow(Utility::pow<2>(delta - 1.0), _a4[i]);
    Psi = std::exp(-_C4[i] * Utility::pow<2>(delta - 1.0) - _D4[i] * Utility::pow<2>(tau - 1.0));
    dDelta_dt = -2.0 * theta * _b4[i] * std::pow(Delta, _b4[i] - 1.0);
    d2Delta_dt2 = 2.0 * _b4[i] * std::pow(Delta, _b4[i] - 1.0) +
                  4.0 * theta * theta * _b4[i] * (_b4[i] - 1.0) * std::pow(Delta, _b4[i] - 2.0);
    dPsi_dt = -2.0 * _D4[i] * (tau - 1.0) * Psi;
    d2Psi_dt2 = 2.0 * _D4[i] * (2.0 * _D4[i] * (tau - 1.0) * (tau - 1.0) - 1.0) * Psi;
    d2phirdt2 +=
        _n4[i] * delta *
        (Psi * d2Delta_dt2 + 2.0 * dDelta_dt * dPsi_dt + std::pow(Delta, _b4[i]) * d2Psi_dt2);
  }

  // The second derivative of the free energy wrt tau is the sum of these components
  return d2phi0dt2 + d2phirdt2;
}

Real
CO2FluidProperties::d2alpha_ddeltatau(Real delta, Real tau) const
{
  // Note: second derivative of the ideal gas component wrt delta and tau is 0
  // Derivative of the residual component wrt gamma
  Real theta, Delta, Psi, dDelta_dd, dPsi_dd, dDelta_dt, dPsi_dt, d2Delta_ddt, d2Psi_ddt;
  Real d2phirddt = 0.0;
  for (std::size_t i = 0; i < _n1.size(); ++i)
    d2phirddt += _n1[i] * _d1[i] * _t1[i] * MathUtils::pow(delta, _d1[i] - 1.0) *
                 std::pow(tau, _t1[i] - 1.0);

  for (std::size_t i = 0; i < _n2.size(); ++i)
    d2phirddt += _n2[i] * std::exp(-MathUtils::pow(delta, _c2[i])) *
                 (MathUtils::pow(delta, _d2[i] - 1.0) * _t2[i] * std::pow(tau, _t2[i] - 1.0) *
                  (_d2[i] - _c2[i] * MathUtils::pow(delta, _c2[i])));

  for (std::size_t i = 0; i < _n3.size(); ++i)
    d2phirddt += _n3[i] * MathUtils::pow(delta, _d3[i]) * MathUtils::pow(tau, _t3[i]) *
                 std::exp(-_alpha3[i] * Utility::pow<2>(delta - _eps3[i]) -
                          _beta3[i] * Utility::pow<2>(tau - _gamma3[i])) *
                 (_d3[i] / delta - 2.0 * _alpha3[i] * (delta - _eps3[i])) *
                 (_t3[i] / tau - 2.0 * _beta3[i] * (tau - _gamma3[i]));

  for (std::size_t i = 0; i < _n4.size(); ++i)
  {
    theta = 1.0 - tau + _A4[i] * std::pow(Utility::pow<2>(delta - 1.0), 1.0 / (2.0 * _beta4[i]));
    Delta = Utility::pow<2>(theta) + _B4[i] * std::pow(Utility::pow<2>(delta - 1.0), _a4[i]);
    Psi = std::exp(-_C4[i] * Utility::pow<2>(delta - 1.0) - _D4[i] * Utility::pow<2>(tau - 1.0));
    dPsi_dd = -2.0 * _C4[i] * (delta - 1.0) * Psi;
    dPsi_dt = -2.0 * _D4[i] * (tau - 1.0) * Psi;
    d2Psi_ddt = 4.0 * _C4[i] * _D4[i] * (delta - 1.0) * (tau - 1.0) * Psi;
    dDelta_dd = (delta - 1.0) *
                (_A4[i] * theta * 2.0 / _beta4[i] *
                     std::pow(Utility::pow<2>(delta - 1.0), 1.0 / (2.0 * _beta4[i]) - 1.0) +
                 2.0 * _B4[i] * _a4[i] * std::pow(Utility::pow<2>(delta - 1.0), _a4[i] - 1.0));
    dDelta_dt = -2.0 * theta * _b4[i] * std::pow(Delta, _b4[i] - 1.0);
    d2Delta_ddt = -2.0 * _A4[i] * _b4[i] / _beta4[i] * std::pow(Delta, _b4[i] - 1.0) *
                      (delta - 1.0) *
                      std::pow(Utility::pow<2>(delta - 1.0), 1.0 / (2.0 * _beta4[i]) - 1.0) -
                  2.0 * theta * _b4[i] * (_b4[i] - 1.0) * std::pow(Delta, _b4[i] - 2.0) * dDelta_dd;

    d2phirddt += _n4[i] * (std::pow(Delta, _b4[i]) * (dPsi_dt + delta * d2Psi_ddt) +
                           delta * _b4[i] * std::pow(Delta, _b4[i] - 1.0) * dDelta_dd * dPsi_dt +
                           dDelta_dt * (Psi + delta * dPsi_dd) + d2Delta_ddt * delta * Psi);
  }

  return d2phirddt;
}

Real
CO2FluidProperties::p_from_rho_T(Real density, Real temperature) const
{
  // Check that the input parameters are within the region of validity
  if (temperature < 216.0 || temperature > 1100.0 || density <= 0.0)
    throw MooseException("Parameters out of range in " + name() + ": pressure()");

  Real pressure = 0.0;

  if (temperature > _triple_point_temperature && temperature < _critical_temperature)
  {
    Real gas_density = saturatedVaporDensity(temperature);
    Real liquid_density = saturatedLiquidDensity(temperature);

    if (density < gas_density || density > liquid_density)
      pressure = HelmholtzFluidProperties::p_from_rho_T(density, temperature);
    else
      pressure = vaporPressure(temperature);
  }
  else
    pressure = HelmholtzFluidProperties::p_from_rho_T(density, temperature);

  return pressure;
}

Real
CO2FluidProperties::rho_from_p_T(Real pressure, Real temperature) const
{
  // Check that the input parameters are within the region of validity
  if (temperature < 216.0 || temperature > 1100.0 || pressure <= 0.0)
    throw MooseException("Parameters out of range in " + name() + ": rho_from_p_T()");

  // Also check that the pressure and temperature are not in the solid phase region
  if (((temperature > _triple_point_temperature) && (pressure > meltingPressure(temperature))) ||
      ((temperature < _triple_point_temperature) && (pressure > sublimationPressure(temperature))))
    throw MooseException("Input pressure and temperature in " + name() +
                         ": rho_from_p_T() correspond to solid CO2 phase");

  Real density;
  // Initial estimate of a bracketing interval for the density
  Real lower_density = 100.0;
  Real upper_density = 1000.0;

  // The density is found by finding the zero of the pressure calculated using the
  // Span and Wagner EOS minus the input pressure
  auto pressure_diff = [&pressure, &temperature, this](Real x)
  { return p_from_rho_T(x, temperature) - pressure; };

  BrentsMethod::bracket(pressure_diff, lower_density, upper_density);
  density = BrentsMethod::root(pressure_diff, lower_density, upper_density);

  return density;
}

void
CO2FluidProperties::rho_from_p_T(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  HelmholtzFluidProperties::rho_from_p_T(pressure, temperature, rho, drho_dp, drho_dT);
}

Real
CO2FluidProperties::mu_from_p_T(Real pressure, Real temperature) const
{
  Real rho = rho_from_p_T(pressure, temperature);
  return mu_from_rho_T(rho, temperature);
}

void
CO2FluidProperties::mu_from_p_T(
    Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  Real rho, drho_dp, drho_dT;
  HelmholtzFluidProperties::rho_from_p_T(pressure, temperature, rho, drho_dp, drho_dT);

  Real dmu_drho;
  mu_from_rho_T(rho, temperature, drho_dT, mu, dmu_drho, dmu_dT);
  dmu_dp = dmu_drho * drho_dp;
}

Real
CO2FluidProperties::mu_from_rho_T(Real density, Real temperature) const
{
  // Check that the input parameters are within the region of validity
  if (temperature < 216.0 || temperature > 1000.0 || density > 1400.0)
    throw MooseException("Parameters out of range in " + name() + ": mu_from_rho_T()");

  Real Tstar = temperature / 251.196;

  // Viscosity in the zero-density limit
  Real sum = 0.0;

  for (std::size_t i = 0; i < _mu_a.size(); ++i)
    sum += _mu_a[i] * MathUtils::pow(std::log(Tstar), i);

  Real mu0 = 1.00697 * std::sqrt(temperature) / std::exp(sum);

  // Excess viscosity due to finite density
  Real mue = _mu_d[0] * density + _mu_d[1] * Utility::pow<2>(density) +
             _mu_d[2] * Utility::pow<6>(density) / Utility::pow<3>(Tstar) +
             _mu_d[3] * Utility::pow<8>(density) + _mu_d[4] * Utility::pow<8>(density) / Tstar;

  return (mu0 + mue) * 1.0e-6; // convert to Pa.s
}

void
CO2FluidProperties::mu_from_rho_T(Real density,
                                  Real temperature,
                                  Real ddensity_dT,
                                  Real & mu,
                                  Real & dmu_drho,
                                  Real & dmu_dT) const
{
  // Check that the input parameters are within the region of validity
  if (temperature < 216.0 || temperature > 1000.0 || density > 1400.0)
    throw MooseException("Parameters out of range in " + name() + ": mu_drhoT()");

  Real Tstar = temperature / 251.196;
  Real dTstar_dT = 1.0 / 251.196;

  // Viscosity in the zero-density limit. Note this is only a function of T.
  // Start the sum at i = 1 so the derivative is defined
  Real sum0 = _mu_a[0], dsum0_dTstar = 0.0;

  for (std::size_t i = 1; i < _mu_a.size(); ++i)
  {
    sum0 += _mu_a[i] * MathUtils::pow(std::log(Tstar), i);
    dsum0_dTstar += i * _mu_a[i] * MathUtils::pow(std::log(Tstar), i - 1) / Tstar;
  }

  Real mu0 = 1.00697 * std::sqrt(temperature) / std::exp(sum0);
  Real dmu0_dT = (0.5 * 1.00697 / std::sqrt(temperature) -
                  1.00697 * std::sqrt(temperature) * dsum0_dTstar * dTstar_dT) /
                 std::exp(sum0);

  // Excess viscosity due to finite density
  Real mue = _mu_d[0] * density + _mu_d[1] * Utility::pow<2>(density) +
             _mu_d[2] * Utility::pow<6>(density) / Utility::pow<3>(Tstar) +
             _mu_d[3] * Utility::pow<8>(density) + _mu_d[4] * Utility::pow<8>(density) / Tstar;

  Real dmue_drho = _mu_d[0] + 2.0 * _mu_d[1] * density +
                   6.0 * _mu_d[2] * Utility::pow<5>(density) / Utility::pow<3>(Tstar) +
                   8.0 * _mu_d[3] * Utility::pow<7>(density) +
                   8.0 * _mu_d[4] * Utility::pow<7>(density) / Tstar;

  Real dmue_dT = (-3.0 * _mu_d[2] * Utility::pow<6>(density) / Utility::pow<4>(Tstar) -
                  _mu_d[4] * Utility::pow<8>(density) / Tstar / Tstar) *
                 dTstar_dT;

  // Viscosity in Pa.s is
  mu = (mu0 + mue) * 1.0e-6;
  dmu_drho = dmue_drho * 1.0e-6;
  dmu_dT = (dmu0_dT + dmue_dT) * 1.0e-6 + dmu_drho * ddensity_dT;
}

void
CO2FluidProperties::rho_mu_from_p_T(Real pressure, Real temperature, Real & rho, Real & mu) const
{
  rho = rho_from_p_T(pressure, temperature);
  mu = mu_from_rho_T(rho, temperature);
}

void
CO2FluidProperties::rho_mu_from_p_T(Real pressure,
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
CO2FluidProperties::partialDensity(Real temperature) const
{
  // This correlation uses temperature in C
  Real Tc = temperature - _T_c2k;
  // The parial volume
  Real V = 37.51 - 9.585e-2 * Tc + 8.74e-4 * Tc * Tc - 5.044e-7 * Tc * Tc * Tc;

  return 1.0e6 * _Mco2 / V;
}

std::vector<Real>
CO2FluidProperties::henryCoefficients() const
{
  return {-8.55445, 4.01195, 9.52345};
}

Real
CO2FluidProperties::k_from_p_T(Real pressure, Real temperature) const
{
  // Require density first
  Real density = rho_from_p_T(pressure, temperature);
  return k_from_rho_T(density, temperature);
}

void
CO2FluidProperties::k_from_p_T(
    Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const
{
  k = this->k_from_p_T(pressure, temperature);
  // Calculate derivatives using finite differences. Note: this will be slow as
  // multiple calculations of density are required
  const Real eps = 1.0e-6;
  const Real peps = pressure * eps;
  const Real Teps = temperature * eps;

  dk_dp = (this->k_from_p_T(pressure + peps, temperature) - k) / peps;
  dk_dT = (this->k_from_p_T(pressure, temperature + Teps) - k) / Teps;
}

Real
CO2FluidProperties::k_from_rho_T(Real density, Real temperature) const
{
  // Check the temperature is in the range of validity (216.592 K <= T <= 1000 K)
  if (temperature <= _triple_point_temperature || temperature >= 1000.0)
    throw MooseException("Temperature " + Moose::stringify(temperature) +
                         "K out of range (200K, 1000K) in " + name() + ": k()");

  // Scaled variables
  Real Tr = temperature / _critical_temperature;
  Real rhor = density / _critical_density;

  Real sum1 = 0.0;
  for (std::size_t i = 0; i < _k_n1.size(); ++i)
    sum1 += _k_n1[i] * std::pow(Tr, _k_g1[i]) * MathUtils::pow(rhor, _k_h1[i]);

  Real sum2 = 0.0;
  for (std::size_t i = 0; i < _k_n2.size(); ++i)
    sum2 += _k_n2[i] * std::pow(Tr, _k_g2[i]) * MathUtils::pow(rhor, _k_h2[i]);

  // Near-critical enhancement
  Real alpha =
      1.0 - _k_a[9] * std::acosh(1.0 + _k_a[10] * std::pow(Utility::pow<2>(1.0 - Tr), _k_a[11]));
  Real lambdac =
      rhor *
      std::exp(-std::pow(rhor, _k_a[0]) / _k_a[0] - Utility::pow<2>(_k_a[1] * (Tr - 1.0)) -
               Utility::pow<2>(_k_a[2] * (rhor - 1.0))) /
      std::pow(std::pow(Utility::pow<2>(
                            1.0 - 1.0 / Tr +
                            _k_a[3] * std::pow(Utility::pow<2>(rhor - 1.0), 1.0 / (2.0 * _k_a[4]))),
                        _k_a[5]) +
                   std::pow(Utility::pow<2>(_k_a[6] * (rhor - alpha)), _k_a[7]),
               _k_a[8]);

  return 4.81384 * (sum1 + std::exp(-5.0 * rhor * rhor) * sum2 + 0.775547504 * lambdac) / 1000.0;
}
