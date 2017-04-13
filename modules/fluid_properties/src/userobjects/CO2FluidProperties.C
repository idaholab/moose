/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CO2FluidProperties.h"
#include "BrentsMethod.h"

template <>
InputParameters
validParams<CO2FluidProperties>()
{
  InputParameters params = validParams<SinglePhaseFluidPropertiesPT>();
  params.addClassDescription(
      "Fluid properties for carbon dioxide (CO2) using the Span & Wagner EOS");
  return params;
}

CO2FluidProperties::CO2FluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidPropertiesPT(parameters)
{
}

CO2FluidProperties::~CO2FluidProperties() {}

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
    mooseError(
        "Temperature is below the triple point temperature in CO2FLuidProperties::meltingPressure");

  Real Tstar = temperature / _triple_point_temperature;

  return _triple_point_pressure *
         (1.0 + 1955.539 * (Tstar - 1.0) + 2055.4593 * std::pow(Tstar - 1.0, 2.0));
}

Real
CO2FluidProperties::sublimationPressure(Real temperature) const
{
  if (temperature > _triple_point_temperature)
    mooseError("Temperature is above the triple point temperature in "
               "CO2FLuidProperties::sublimationPressure");

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
    mooseError("Temperature is out of range in CO2FLuidProperties::vaporPressure");

  Real Tstar = temperature / _critical_temperature;

  Real logpressure =
      (-7.0602087 * (1.0 - Tstar) + 1.9391218 * std::pow(1.0 - Tstar, 1.5) -
       1.6463597 * std::pow(1.0 - Tstar, 2.0) - 3.2995634 * std::pow(1.0 - Tstar, 4.0)) /
      Tstar;

  return _critical_pressure * std::exp(logpressure);
}

Real
CO2FluidProperties::saturatedLiquidDensity(Real temperature) const
{
  if (temperature < _triple_point_temperature || temperature > _critical_temperature)
    mooseError("Temperature is out of range in CO2FLuidProperties::saturatedLiquiDensity");

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
    mooseError("Temperature is out of range in CO2FLuidProperties::saturatedVaporDensity");

  Real Tstar = temperature / _critical_temperature;

  Real logdensity =
      (-1.7074879 * std::pow(1.0 - Tstar, 0.34) - 0.82274670 * std::pow(1.0 - Tstar, 0.5) -
       4.6008549 * (1.0 - Tstar) - 10.111178 * std::pow(1.0 - Tstar, 7.0 / 3.0) -
       29.742252 * std::pow(1.0 - Tstar, 14.0 / 3.0));

  return _critical_density * std::exp(logdensity);
}

Real
CO2FluidProperties::phiSW(Real delta, Real tau) const
{
  // Ideal gas component of the Helmholtz free energy
  Real sum0 = 0.0;
  for (unsigned int i = 0; i < _a0.size(); ++i)
    sum0 += _a0[i] * std::log(1.0 - std::exp(-_theta0[i] * tau));

  Real phi0 = std::log(delta) + 8.37304456 - 3.70454304 * tau + 2.5 * std::log(tau) + sum0;

  // Residual component of the Helmholtz free energy
  Real theta, Delta, Psi;
  Real phir = 0.0;
  for (unsigned int i = 0; i < _n1.size(); ++i)
    phir += _n1[i] * std::pow(delta, _d1[i]) * std::pow(tau, _t1[i]);

  for (unsigned int i = 0; i < _n2.size(); ++i)
    phir += _n2[i] * std::pow(delta, _d2[i]) * std::pow(tau, _t2[i]) *
            std::exp(-std::pow(delta, _c2[i]));

  for (unsigned int i = 0; i < _n3.size(); ++i)
    phir += _n3[i] * std::pow(delta, _d3[i]) * std::pow(tau, _t3[i]) *
            std::exp(-_alpha3[i] * std::pow(delta - _eps3[i], 2.0) -
                     _beta3[i] * std::pow(tau - _gamma3[i], 2.0));

  for (unsigned int i = 0; i < _n4.size(); ++i)
  {
    theta = 1.0 - tau + _A4[i] * std::pow(std::pow(delta - 1.0, 2.0), 1.0 / (2.0 * _beta4[i]));
    Delta = std::pow(theta, 2) + _B4[i] * std::pow(std::pow(delta - 1.0, 2.0), _a4[i]);
    Psi = std::exp(-_C4[i] * std::pow(delta - 1.0, 2.0) - _D4[i] * std::pow(tau - 1.0, 2.0));
    phir += _n4[i] * std::pow(Delta, _b4[i]) * delta * Psi;
  }

  // The Helmholtz free energy is the sum of these components
  return phi0 + phir;
}

Real
CO2FluidProperties::dphiSW_dd(Real delta, Real tau) const
{
  // Derivative of the ideal gas component wrt gamma
  Real dphi0dd = 1.0 / delta;

  // Derivative of the residual component wrt gamma
  Real theta, Delta, Psi, dDelta_dd, dPsi_dd;
  Real dphirdd = 0.0;

  for (unsigned int i = 0; i < _n1.size(); ++i)
    dphirdd += _n1[i] * _d1[i] * std::pow(delta, _d1[i] - 1.0) * std::pow(tau, _t1[i]);

  for (unsigned int i = 0; i < _n2.size(); ++i)
    dphirdd += _n2[i] * std::exp(-std::pow(delta, _c2[i])) *
               (std::pow(delta, _d2[i] - 1.0) * std::pow(tau, _t2[i]) *
                (_d2[i] - _c2[i] * std::pow(delta, _c2[i])));

  for (unsigned int i = 0; i < _n3.size(); ++i)
    dphirdd += _n3[i] * std::pow(delta, _d3[i]) * std::pow(tau, _t3[i]) *
               std::exp(-_alpha3[i] * std::pow(delta - _eps3[i], 2.0) -
                        _beta3[i] * std::pow(tau - _gamma3[i], 2.0)) *
               (_d3[i] / delta - 2.0 * _alpha3[i] * (delta - _eps3[i]));

  for (unsigned int i = 0; i < _n4.size(); ++i)
  {
    theta = 1.0 - tau + _A4[i] * std::pow(std::pow(delta - 1.0, 2.0), 1.0 / (2.0 * _beta4[i]));
    Delta = std::pow(theta, 2.0) + _B4[i] * std::pow(std::pow(delta - 1.0, 2.0), _a4[i]);
    Psi = std::exp(-_C4[i] * std::pow(delta - 1.0, 2.0) - _D4[i] * std::pow(tau - 1.0, 2.0));
    dPsi_dd = -2.0 * _C4[i] * (delta - 1.0) * Psi;
    dDelta_dd = (delta - 1.0) *
                (_A4[i] * theta * 2.0 / _beta4[i] *
                     std::pow(std::pow(delta - 1.0, 2.0), 1.0 / (2.0 * _beta4[i]) - 1.0) +
                 2.0 * _B4[i] * _a4[i] * std::pow(std::pow(delta - 1.0, 2.0), _a4[i] - 1.0));

    dphirdd += _n4[i] * (std::pow(Delta, _b4[i]) * (Psi + delta * dPsi_dd) +
                         _b4[i] * std::pow(Delta, _b4[i] - 1.0) * dDelta_dd * delta * Psi);
  }

  // The derivative of the free energy wrt delta is the sum of these components
  return dphi0dd + dphirdd;
}

Real
CO2FluidProperties::dphiSW_dt(Real delta, Real tau) const
{
  // Derivative of the ideal gas component wrt tau
  Real sum0 = 0.0;
  for (unsigned int i = 0; i < _a0.size(); ++i)
    sum0 += _a0[i] * _theta0[i] * (1.0 / (1.0 - std::exp(-_theta0[i] * tau)) - 1.0);

  Real dphi0dt = -3.70454304 + 2.5 / tau + sum0;

  // Derivative of the residual component wrt tau
  Real theta, Delta, Psi, dDelta_dt, dPsi_dt;
  Real dphirdt = 0.0;
  for (unsigned int i = 0; i < _n1.size(); ++i)
    dphirdt += _n1[i] * _t1[i] * std::pow(delta, _d1[i]) * std::pow(tau, _t1[i] - 1.0);

  for (unsigned int i = 0; i < _n2.size(); ++i)
    dphirdt += _n2[i] * _t2[i] * std::pow(delta, _d2[i]) * std::pow(tau, _t2[i] - 1.0) *
               std::exp(-std::pow(delta, _c2[i]));

  for (unsigned int i = 0; i < _n3.size(); ++i)
    dphirdt += _n3[i] * std::pow(delta, _d3[i]) * std::pow(tau, _t3[i]) *
               std::exp(-_alpha3[i] * std::pow(delta - _eps3[i], 2.0) -
                        _beta3[i] * std::pow(tau - _gamma3[i], 2.0)) *
               (_t3[i] / tau - 2.0 * _beta3[i] * (tau - _gamma3[i]));

  for (unsigned int i = 0; i < _n4.size(); ++i)
  {
    theta = 1.0 - tau + _A4[i] * std::pow(std::pow(delta - 1.0, 2.0), 1.0 / (2.0 * _beta4[i]));
    Delta = std::pow(theta, 2.0) + _B4[i] * std::pow(std::pow(delta - 1.0, 2.0), _a4[i]);
    Psi = std::exp(-_C4[i] * std::pow(delta - 1.0, 2.0) - _D4[i] * std::pow(tau - 1.0, 2.0));
    dDelta_dt = -2.0 * theta * _b4[i] * std::pow(Delta, _b4[i] - 1.0);
    dPsi_dt = -2.0 * _D4[i] * (tau - 1.0) * Psi;

    dphirdt += _n4[i] * delta * (Psi * dDelta_dt + std::pow(Delta, _b4[i]) * dPsi_dt);
  }

  // The derivative of the free energy wrt tau is the sum of these components
  return dphi0dt + dphirdt;
}

Real
CO2FluidProperties::d2phiSW_dd2(Real delta, Real tau) const
{
  // Second derivative of the ideal gas component wrt gamma
  Real d2phi0dd2 = -1.0 / delta / delta;

  // Second derivative of the residual component wrt gamma
  Real d2phirdd2 = 0.0;
  Real theta, Delta, Psi, dDelta_dd, dPsi_dd, d2Delta_dd2, d2Psi_dd2;

  for (unsigned int i = 0; i < _n1.size(); ++i)
    d2phirdd2 +=
        _n1[i] * _d1[i] * (_d1[i] - 1.0) * std::pow(delta, _d1[i] - 2.0) * std::pow(tau, _t1[i]);

  for (unsigned int i = 0; i < _n2.size(); ++i)
    d2phirdd2 += _n2[i] * std::exp(-std::pow(delta, _c2[i])) * std::pow(delta, _d2[i] - 2.0) *
                 std::pow(tau, _t2[i]) * ((_d2[i] - _c2[i] * std::pow(delta, _c2[i])) *
                                              (_d2[i] - 1.0 - _c2[i] * std::pow(delta, _c2[i])) -
                                          _c2[i] * _c2[i] * std::pow(delta, _c2[i]));

  for (unsigned int i = 0; i < _n3.size(); ++i)
    d2phirdd2 +=
        _n3[i] * std::pow(tau, _t3[i]) * std::exp(-_alpha3[i] * std::pow(delta - _eps3[i], 2.0) -
                                                  _beta3[i] * std::pow(tau - _gamma3[i], 2.0)) *
        (-2.0 * _alpha3[i] * std::pow(delta, _d3[i]) +
         4.0 * _alpha3[i] * _alpha3[i] * std::pow(delta, _d3[i]) * std::pow(delta - _eps3[i], 2.0) -
         4.0 * _d3[i] * _alpha3[i] * std::pow(delta, _d3[i] - 1.0) * (delta - _eps3[i]) +
         _d3[i] * (_d3[i] - 1.0) * std::pow(delta, _d3[i] - 2.0));

  for (unsigned int i = 0; i < _n4.size(); ++i)
  {
    theta = 1.0 - tau + _A4[i] * std::pow(std::pow(delta - 1.0, 2.0), 1.0 / (2.0 * _beta4[i]));
    Delta = std::pow(theta, 2.0) + _B4[i] * std::pow(std::pow(delta - 1.0, 2.0), _a4[i]);
    Psi = std::exp(-_C4[i] * std::pow(delta - 1.0, 2.0) - _D4[i] * std::pow(tau - 1.0, 2.0));
    dPsi_dd = -2.0 * _C4[i] * (delta - 1.0) * Psi;
    dDelta_dd = (delta - 1.0) *
                (_A4[i] * theta * 2.0 / _beta4[i] *
                     std::pow(std::pow(delta - 1.0, 2.0), 1.0 / (2.0 * _beta4[i]) - 1.0) +
                 2.0 * _B4[i] * _a4[i] * std::pow(std::pow(delta - 1.0, 2.0), _a4[i] - 1.0));
    d2Psi_dd2 = 3.0 * _D4[i] * Psi * (2.0 * _C4[i] * std::pow(delta - 1.0, 2.0) - 1.0);
    d2Delta_dd2 =
        1.0 / (delta - 1.0) * dDelta_dd +
        (delta - 1.0) * (delta - 1.0) *
            (4.0 * _B4[i] * _a4[i] * (_a4[i] - 1.0) *
                 std::pow(std::pow(delta - 1.0, 2.0), _a4[i] - 2.0) +
             2.0 * _A4[i] * _A4[i] *
                 std::pow(std::pow(std::pow(delta - 1.0, 2.0), 1.0 / (2.0 * _beta4[i]) - 1.0),
                          2.0) /
                 _beta4[i] / _beta4[i] +
             (4.0 / _beta4[i]) * _A4[i] * theta * (1.0 / (2.0 * _beta4[i]) - 1.0) *
                 std::pow(std::pow(delta - 1.0, 2.0), 1.0 / (2.0 * _beta4[i]) - 2.0));
    d2phirdd2 +=
        _n4[i] *
        (std::pow(Delta, _b4[i]) * (2.0 * dPsi_dd + delta * d2Psi_dd2) +
         2.0 * _b4[i] * std::pow(Delta, _b4[i] - 1.0) * dDelta_dd * (Psi + delta * dPsi_dd) +
         _b4[i] * (std::pow(Delta, _b4[i] - 1.0) * d2Delta_dd2 +
                   (_b4[i] - 1.0) * std::pow(Delta, _b4[i] - 2.0) * std::pow(dDelta_dd, 2.0)) *
             delta * Psi);
  }
  // The second derivative of the free energy wrt delta is the sum of these components
  return d2phi0dd2 + d2phirdd2;
}

Real
CO2FluidProperties::d2phiSW_dt2(Real delta, Real tau) const
{
  // Second derivative of the ideal gas component wrt tau
  Real sum0 = 0.0;
  for (unsigned int i = 0; i < _a0.size(); ++i)
    sum0 += _a0[i] * _theta0[i] * _theta0[i] * std::exp(-_theta0[i] * tau) *
            std::pow(1.0 - std::exp(-_theta0[i] * tau), -2.0);

  Real d2phi0dt2 = -2.5 / tau / tau - sum0;

  // Second derivative of the residual component wrt tau
  Real d2phirdt2 = 0.0;
  Real theta, Delta, Psi, dPsi_dt, dDelta_dt, d2Delta_dt2, d2Psi_dt2;

  for (unsigned int i = 0; i < _n1.size(); ++i)
    d2phirdt2 +=
        _n1[i] * _t1[i] * (_t1[i] - 1.0) * std::pow(delta, _d1[i]) * std::pow(tau, _t1[i] - 2.0);

  for (unsigned int i = 0; i < _n2.size(); ++i)
    d2phirdt2 += _n2[i] * _t2[i] * (_t2[i] - 1.0) * std::pow(delta, _d2[i]) *
                 std::exp(-std::pow(delta, _c2[i])) * std::pow(tau, _t2[i] - 2.0);

  for (unsigned int i = 0; i < _n3.size(); ++i)
    d2phirdt2 += _n3[i] * std::pow(delta, _d3[i]) * std::pow(tau, _t3[i]) *
                 std::exp(-_alpha3[i] * std::pow(delta - _eps3[i], 2.0) -
                          _beta3[i] * std::pow(tau - _gamma3[i], 2.0)) *
                 (std::pow(_t3[i] / tau - 2.0 * _beta3[i] * (tau - _gamma3[i]), 2.0) -
                  _t3[i] / tau / tau - 2.0 * _beta3[i]);

  for (unsigned int i = 0; i < _n4.size(); ++i)
  {
    theta = 1.0 - tau + _A4[i] * std::pow(std::pow(delta - 1.0, 2.0), 1.0 / (2.0 * _beta4[i]));
    Delta = std::pow(theta, 2.0) + _B4[i] * std::pow(std::pow(delta - 1.0, 2.0), _a4[i]);
    Psi = std::exp(-_C4[i] * std::pow(delta - 1.0, 2.0) - _D4[i] * std::pow(tau - 1.0, 2.0));
    dDelta_dt = -2.0 * theta * _b4[i] * std::pow(Delta, _b4[i] - 1.0);
    d2Delta_dt2 = 2.0 * _b4[i] * std::pow(Delta, _b4[i] - 1.0) +
                  4.0 * theta * theta * _b4[i] * (_b4[i] - 1.0) * std::pow(Delta, _b4[i] - 2.0);
    dPsi_dt = -2.0 * _D4[i] * (tau - 1.0) * Psi;
    d2Psi_dt2 = 2.0 * _D4[i] * (2.0 * _D4[i] * (tau - 1.0) * (tau - 1.0) - 1.0) * Psi;
    d2phirdt2 += _n4[i] * delta * (Psi * d2Delta_dt2 + 2.0 * dDelta_dt * dPsi_dt +
                                   std::pow(Delta, _b4[i]) * d2Psi_dt2);
  }

  // The second derivative of the free energy wrt tau is the sum of these components
  return d2phi0dt2 + d2phirdt2;
}

Real
CO2FluidProperties::d2phiSW_ddt(Real delta, Real tau) const
{
  // Note: second derivative of the ideal gas component wrt delta and tau is 0
  // Derivative of the residual component wrt gamma
  Real theta, Delta, Psi, dDelta_dd, dPsi_dd, dDelta_dt, dPsi_dt, d2Delta_ddt, d2Psi_ddt;
  Real d2phirddt = 0.0;
  for (unsigned int i = 0; i < _n1.size(); ++i)
    d2phirddt +=
        _n1[i] * _d1[i] * _t1[i] * std::pow(delta, _d1[i] - 1.0) * std::pow(tau, _t1[i] - 1.0);

  for (unsigned int i = 0; i < _n2.size(); ++i)
    d2phirddt += _n2[i] * std::exp(-std::pow(delta, _c2[i])) *
                 (std::pow(delta, _d2[i] - 1.0) * _t2[i] * std::pow(tau, _t2[i] - 1.0) *
                  (_d2[i] - _c2[i] * std::pow(delta, _c2[i])));

  for (unsigned int i = 0; i < _n3.size(); ++i)
    d2phirddt += _n3[i] * std::pow(delta, _d3[i]) * std::pow(tau, _t3[i]) *
                 std::exp(-_alpha3[i] * std::pow(delta - _eps3[i], 2.0) -
                          _beta3[i] * std::pow(tau - _gamma3[i], 2.0)) *
                 (_d3[i] / delta - 2.0 * _alpha3[i] * (delta - _eps3[i])) *
                 (_t3[i] / tau - 2.0 * _beta3[i] * (tau - _gamma3[i]));

  for (unsigned int i = 0; i < _n4.size(); ++i)
  {
    theta = 1.0 - tau + _A4[i] * std::pow(std::pow(delta - 1.0, 2.0), 1.0 / (2.0 * _beta4[i]));
    Delta = std::pow(theta, 2.0) + _B4[i] * std::pow(std::pow(delta - 1.0, 2.0), _a4[i]);
    Psi = std::exp(-_C4[i] * std::pow(delta - 1.0, 2.0) - _D4[i] * std::pow(tau - 1.0, 2.0));
    dPsi_dd = -2.0 * _C4[i] * (delta - 1.0) * Psi;
    dPsi_dt = -2.0 * _D4[i] * (tau - 1.0) * Psi;
    d2Psi_ddt = 4.0 * _C4[i] * _D4[i] * (delta - 1.0) * (tau - 1.0) * Psi;
    dDelta_dd = (delta - 1.0) *
                (_A4[i] * theta * 2.0 / _beta4[i] *
                     std::pow(std::pow(delta - 1.0, 2.0), 1.0 / (2.0 * _beta4[i]) - 1.0) +
                 2.0 * _B4[i] * _a4[i] * std::pow(std::pow(delta - 1.0, 2.0), _a4[i] - 1.0));
    dDelta_dt = -2.0 * theta * _b4[i] * std::pow(Delta, _b4[i] - 1.0);
    d2Delta_ddt = -2.0 * _A4[i] * _b4[i] / _beta4[i] * std::pow(Delta, _b4[i] - 1.0) *
                      (delta - 1.0) *
                      std::pow(std::pow(delta - 1.0, 2.0), 1.0 / (2.0 * _beta4[i]) - 1.0) -
                  2.0 * theta * _b4[i] * (_b4[i] - 1.0) * std::pow(Delta, _b4[i] - 2.0) * dDelta_dd;

    d2phirddt += _n4[i] * (std::pow(Delta, _b4[i]) * (dPsi_dt + delta * d2Psi_ddt) +
                           delta * _b4[i] * std::pow(Delta, _b4[i] - 1.0) * dDelta_dd * dPsi_dt +
                           dDelta_dt * (Psi + delta * dPsi_dd) + d2Delta_ddt * delta * Psi);
  }

  return d2phirddt;
}

Real
CO2FluidProperties::pressureSW(Real density, Real temperature) const
{
  // Scale the input density and temperature
  Real delta = density / _critical_density;
  Real tau = _critical_temperature / temperature;

  return _Rco2 * temperature * density * delta * dphiSW_dd(delta, tau);
}

Real
CO2FluidProperties::pressure(Real density, Real temperature) const
{
  // Check that the input parameters are within the region of validity
  if (temperature < 216.0 || temperature > 1100.0 || density <= 0.0)
    mooseError("Parameters out of range in CO2FLuidProperties::pressure");

  Real pressure = 0.0;

  if (temperature > _triple_point_temperature && temperature < _critical_temperature)
  {
    Real gas_density = saturatedVaporDensity(temperature);
    Real liquid_density = saturatedLiquidDensity(temperature);

    if (density < gas_density || density > liquid_density)
      pressure = pressureSW(density, temperature);
    else
      pressure = vaporPressure(temperature);
  }
  else
    pressure = pressureSW(density, temperature);

  return pressure;
}

Real
CO2FluidProperties::rho(Real pressure, Real temperature) const
{
  // Check that the input parameters are within the region of validity
  if (temperature < 216.0 || temperature > 1100.0 || pressure <= 0.0)
    mooseError("Parameters out of range in CO2FLuidProperties::rho");

  // Also check that the pressure and temperature are not in the solid phase region
  if (((temperature > _triple_point_temperature) && (pressure > meltingPressure(temperature))) ||
      ((temperature < _triple_point_temperature) && (pressure > sublimationPressure(temperature))))
    mooseError(
        "Input pressure and temperature in CO2FLuidProperties::rho correspond to solid CO2 phase");

  Real density;
  // Initial estimate of a bracketing interval for the density
  Real lower_density = 100.0;
  Real upper_density = 1000.0;

  // The density is found by finding the zero of the pressure calculated using the
  // Span and Wagner EOS minus the input pressure
  auto pressure_diff = [&pressure, &temperature, this](Real x) {
    return this->pressure(x, temperature) - pressure;
  };

  BrentsMethod::bracket(pressure_diff, lower_density, upper_density);
  density = BrentsMethod::root(pressure_diff, lower_density, upper_density);

  return density;
}

void
CO2FluidProperties::rho_dpT(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = this->rho(pressure, temperature);
  // Scale the density and temperature
  Real delta = rho / _critical_density;
  Real tau = _critical_temperature / temperature;
  Real dpdd = dphiSW_dd(delta, tau);
  Real d2pdd2 = d2phiSW_dd2(delta, tau);

  drho_dp = 1.0 / (_Rco2 * temperature * delta * (2.0 * dpdd + delta * d2pdd2));
  drho_dT =
      rho * (tau * d2phiSW_ddt(delta, tau) - dpdd) / temperature / (2.0 * dpdd + delta * d2pdd2);
}

Real
CO2FluidProperties::mu(Real density, Real temperature) const
{
  // Check that the input parameters are within the region of validity
  if (temperature < 216.0 || temperature > 1000.0 || density > 1400.0)
    mooseError("Parameters out of range in CO2FLuidProperties::mu");

  Real Tstar = temperature / 251.196;
  const std::vector<Real> a{0.235156, -0.491266, 5.211155e-2, 5.347906e-2, -1.537102e-2};
  const std::vector<Real> d{
      0.4071119e-2, 0.7198037e-4, 0.2411697e-16, 0.2971072e-22, -0.1627888e-22};

  // Viscosity in the zero-density limit
  Real sum = 0.0;

  for (unsigned int i = 0; i < a.size(); ++i)
    sum += a[i] * std::pow(std::log(Tstar), i);

  Real mu0 = 1.00697 * std::sqrt(temperature) / std::exp(sum);

  // Excess viscosity due to finite density
  Real mue = d[0] * density + d[1] * std::pow(density, 2) +
             d[2] * std::pow(density, 6) / std::pow(Tstar, 3) + d[3] * std::pow(density, 8) +
             d[4] * std::pow(density, 8) / Tstar;

  return (mu0 + mue) * 1e-6; // convert to Pa.s
}

void
CO2FluidProperties::mu_drhoT(
    Real density, Real temperature, Real & mu, Real & dmu_drho, Real & dmu_dT) const
{
  // Check that the input parameters are within the region of validity
  if (temperature < 216.0 || temperature > 1000.0 || density > 1400.0)
    mooseError("Parameters out of range in CO2FLuidProperties::mu_drhoT");

  Real Tstar = temperature / 251.196;
  const std::vector<Real> a{0.235156, -0.491266, 5.211155e-2, 5.347906e-2, -1.537102e-2};
  const std::vector<Real> d{
      0.4071119e-2, 0.7198037e-4, 0.2411697e-16, 0.2971072e-22, -0.1627888e-22};

  // Viscosity in the zero-density limit. Note this is only a function of T
  Real sum0 = 0.0;

  for (unsigned int i = 0; i < a.size(); ++i)
    sum0 += a[i] * std::pow(std::log(Tstar), i);

  Real mu0 = 1.00697 * std::sqrt(temperature) / std::exp(sum0);

  // Excess viscosity due to finite density
  Real mue = d[0] * density + d[1] * std::pow(density, 2) +
             d[2] * std::pow(density, 6) / std::pow(Tstar, 3) + d[3] * std::pow(density, 8) +
             d[4] * std::pow(density, 8) / Tstar;

  Real dmuedrho = d[0] + 2.0 * d[1] * density +
                  6.0 * d[2] * std::pow(density, 5) / std::pow(Tstar, 3) +
                  8.0 * d[3] * std::pow(density, 7) + 8.0 * d[4] * std::pow(density, 7) / Tstar;

  // Use finite difference for derivative wrt T for now, as drho_dT is required
  // to calculate analytical derivative
  Real eps = 1.0e-8;
  Real Teps = temperature * eps;
  Real mu2T = this->mu(density, temperature + Teps);

  // Viscosity in Pa.s is
  mu = (mu0 + mue) * 1e-6;
  dmu_drho = dmuedrho * 1e-6;
  dmu_dT = (mu2T - mu) / Teps;
}

Real
CO2FluidProperties::partialDensity(Real temperature) const
{
  // This correlation uses temperature in C
  Real Tc = temperature - _T_c2k;
  // The parial volume
  Real V = 37.51 - 9.585e-2 * Tc + 8.74e-4 * Tc * Tc - 5.044e-7 * Tc * Tc * Tc;

  return 1.e6 * _Mco2 / V;
}

Real
CO2FluidProperties::henryConstant(Real temperature) const
{
  return henryConstantIAPWS(temperature, -8.55445, 4.01195, 9.52345);
}

void
CO2FluidProperties::henryConstant_dT(Real temperature, Real & Kh, Real & dKh_dT) const
{
  henryConstantIAPWS_dT(temperature, Kh, dKh_dT, -8.55445, 4.01195, 9.52345);
}

Real
CO2FluidProperties::e(Real pressure, Real temperature) const
{
  // Require density first
  Real density = rho(pressure, temperature);
  // Scale the input density and temperature
  Real delta = density / _critical_density;
  Real tau = _critical_temperature / temperature;

  return _Rco2 * temperature * tau * dphiSW_dt(delta, tau);
}

void
CO2FluidProperties::e_dpT(
    Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const
{
  // Require density first
  Real density = rho(pressure, temperature);
  // Scale the input density and temperature
  Real delta = density / _critical_density;
  Real tau = _critical_temperature / temperature;

  e = _Rco2 * temperature * tau * dphiSW_dt(delta, tau);
  Real dpdd = dphiSW_dd(delta, tau);
  Real d2pdd2 = d2phiSW_dd2(delta, tau);
  Real d2pddt = d2phiSW_ddt(delta, tau);

  de_dp = tau * d2pddt / (density * (2.0 * dpdd + delta * d2pdd2));
  de_dT = -_Rco2 * (delta * tau * d2pddt * (dpdd - tau * d2pddt) / (2.0 * dpdd + delta * d2pdd2) +
                    tau * tau * d2phiSW_dt2(delta, tau));
}

void
CO2FluidProperties::rho_e_dpT(Real pressure,
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
CO2FluidProperties::c(Real pressure, Real temperature) const
{
  // Require density first
  Real density = rho(pressure, temperature);
  // Scale the input density and temperature
  Real delta = density / _critical_density;
  Real tau = _critical_temperature / temperature;

  Real speed2 =
      2.0 * delta * dphiSW_dd(delta, tau) + delta * delta * d2phiSW_dd2(delta, tau) -
      std::pow(delta * dphiSW_dd(delta, tau) - delta * tau * d2phiSW_ddt(delta, tau), 2.0) /
          (tau * tau * d2phiSW_dt2(delta, tau));

  return std::sqrt(_Rco2 * temperature * speed2);
}

Real
CO2FluidProperties::cp(Real pressure, Real temperature) const
{
  // Require density first
  Real density = rho(pressure, temperature);
  // Scale the input density and temperature
  Real delta = density / _critical_density;
  Real tau = _critical_temperature / temperature;

  Real heat_capacity =
      -tau * tau * d2phiSW_dt2(delta, tau) +
      std::pow(delta * dphiSW_dd(delta, tau) - delta * tau * d2phiSW_ddt(delta, tau), 2.0) /
          (2.0 * delta * dphiSW_dd(delta, tau) + delta * delta * d2phiSW_dd2(delta, tau));

  return _Rco2 * heat_capacity;
}

Real
CO2FluidProperties::cv(Real pressure, Real temperature) const
{
  // Require density first
  Real density = rho(pressure, temperature);
  // Scale the input density and temperature
  Real delta = density / _critical_density;
  Real tau = _critical_temperature / temperature;

  return -_Rco2 * tau * tau * d2phiSW_dt2(delta, tau);
}

Real
CO2FluidProperties::k(Real density, Real temperature) const
{
  // Check the temperature is in the range of validity (216.592 K <= T <= 1000 K)
  if (temperature <= _triple_point_temperature || temperature >= 1000.0)
    mooseError("Temperature ", temperature, "K out of range (200K, 1000K) in ", name(), ":k()");

  const std::vector<Real> g1{0.0, 0.0, 1.5};
  const std::vector<Real> g2{0.0, 1.0, 1.5, 1.5, 1.5, 3.5, 5.5};
  const std::vector<Real> h1{1.0, 5.0, 1.0};
  const std::vector<Real> h2{1.0, 2.0, 0.0, 5.0, 9.0, 0.0, 0.0};
  const std::vector<Real> n1{7.69857587, 0.159885811, 1.56918621};
  const std::vector<Real> n2{
      -6.73400790, 16.3890156, 3.69415242, 22.3205514, 66.1420950, -0.171779133, 0.00433043347};
  const std::vector<Real> a{
      3.0, 6.70697, 0.94604, 0.3, 0.3, 0.39751, 0.33791, 0.77963, 0.79857, 0.9, 0.02, 0.2};

  // Scaled variables
  Real Tr = temperature / _critical_temperature;
  Real rhor = density / _critical_density;

  Real sum1 = 0.0;
  for (unsigned int i = 0; i < n1.size(); ++i)
    sum1 += n1[i] * std::pow(Tr, g1[i]) * std::pow(rhor, h1[i]);

  Real sum2 = 0.0;
  for (unsigned int i = 0; i < n2.size(); ++i)
    sum2 += n2[i] * std::pow(Tr, g2[i]) * std::pow(rhor, h2[i]);

  // Near-critical enhancement
  Real alpha = 1.0 - a[9] * std::acosh(1.0 + a[10] * std::pow(std::pow(1.0 - Tr, 2.0), a[11]));
  Real lambdac =
      rhor * std::exp(-std::pow(rhor, a[0]) / a[0] - std::pow(a[1] * (Tr - 1.0), 2.0) -
                      std::pow(a[2] * (rhor - 1.0), 2.0)) /
      std::pow(std::pow(std::pow(1.0 - 1.0 / Tr +
                                     a[3] * std::pow(std::pow(rhor - 1.0, 2.0), 1.0 / (2.0 * a[4])),
                                 2.0),
                        a[5]) +
                   std::pow(std::pow(a[6] * (rhor - alpha), 2.0), a[7]),
               a[8]);

  return 4.81384 * (sum1 + std::exp(-5.0 * rhor * rhor) * sum2 + 0.775547504 * lambdac) / 1000.0;
}

Real
CO2FluidProperties::s(Real pressure, Real temperature) const
{
  // Require density first
  Real density = rho(pressure, temperature);
  // Scale the input density and temperature
  Real delta = density / _critical_density;
  Real tau = _critical_temperature / temperature;

  return _Rco2 * (tau * dphiSW_dt(delta, tau) - phiSW(delta, tau));
}

Real
CO2FluidProperties::h(Real pressure, Real temperature) const
{
  // Require density first
  Real density = rho(pressure, temperature);
  // Scale the input density and temperature
  Real delta = density / _critical_density;
  Real tau = _critical_temperature / temperature;

  return _Rco2 * temperature * (tau * dphiSW_dt(delta, tau) + delta * dphiSW_dd(delta, tau));
}

void
CO2FluidProperties::h_dpT(
    Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const
{
  // Require density first
  Real density = rho(pressure, temperature);
  // Scale the input density and temperature
  Real delta = density / _critical_density;
  Real tau = _critical_temperature / temperature;
  Real dpdd = dphiSW_dd(delta, tau);
  Real d2pdd2 = d2phiSW_dd2(delta, tau);
  Real d2pddt = d2phiSW_ddt(delta, tau);

  h = _Rco2 * temperature * (tau * dphiSW_dt(delta, tau) + delta * dpdd);
  dh_dp = (dpdd + delta * d2pdd2 + tau * d2pddt) / (density * (2.0 * dpdd + delta * d2pdd2));
  dh_dT = _Rco2 * delta * dpdd * (1.0 - tau * d2pddt / dpdd) * (1.0 - tau * d2pddt / dpdd) /
              (2.0 + delta * d2pdd2 / dpdd) -
          _Rco2 * tau * tau * d2phiSW_dt2(delta, tau);
}

Real CO2FluidProperties::beta(Real /*pressure*/, Real /*temperature*/) const
{
  mooseError("CO2FluidProperties::beta not implemented yet");
  return 0.0;
}
