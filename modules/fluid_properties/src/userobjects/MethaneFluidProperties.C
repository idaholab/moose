/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "MethaneFluidProperties.h"

template <>
InputParameters
validParams<MethaneFluidProperties>()
{
  InputParameters params = validParams<SinglePhaseFluidPropertiesPT>();
  params.addParam<Real>("beta", 0, "Coefficient of thermal expansion");
  params.addClassDescription("Fluid properties for methane (CH4)");
  return params;
}

MethaneFluidProperties::MethaneFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidPropertiesPT(parameters),
    _Mch4(16.0425e-3),
    _p_critical(4.5992e6),
    _T_critical(190.564),
    _rho_critical(162.66),
    _beta(getParam<Real>("beta"))
{
}

MethaneFluidProperties::~MethaneFluidProperties() {}

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
MethaneFluidProperties::rho(Real pressure, Real temperature) const
{
  return pressure * _Mch4 / (_R * temperature);
}

void
MethaneFluidProperties::rho_dpT(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = this->rho(pressure, temperature);
  drho_dp = _Mch4 / (_R * temperature);
  drho_dT = -pressure * _Mch4 / (_R * temperature * temperature);
}

Real
MethaneFluidProperties::e(Real pressure, Real temperature) const
{
  return h(pressure, temperature) - _R * temperature / _Mch4;
}

void
MethaneFluidProperties::e_dpT(
    Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const
{
  e = this->e(pressure, temperature);
  Real enthalpy, enthalpy_dp, enthalpy_dT;
  h_dpT(pressure, temperature, enthalpy, enthalpy_dp, enthalpy_dT);
  de_dp = enthalpy_dp;
  de_dT = enthalpy_dT - _R / _Mch4;
}

void
MethaneFluidProperties::rho_e_dpT(Real pressure,
                                  Real temperature,
                                  Real & rho,
                                  Real & drho_dp,
                                  Real & drho_dT,
                                  Real & e,
                                  Real & de_dp,
                                  Real & de_dT) const
{
  Real density, ddensity_dp, ddensity_dT;
  rho_dpT(pressure, temperature, density, ddensity_dp, ddensity_dT);
  rho = density;
  drho_dp = ddensity_dp;
  drho_dT = ddensity_dT;

  Real energy, denergy_dp, denergy_dT;
  e_dpT(pressure, temperature, energy, denergy_dp, denergy_dT);
  e = energy;
  de_dp = denergy_dp;
  de_dT = denergy_dT;
}

Real
MethaneFluidProperties::c(Real pressure, Real temperature) const
{
  return std::sqrt(gamma(pressure, temperature) * _R * temperature / _Mch4);
}

Real
MethaneFluidProperties::cp(Real /*pressure*/, Real temperature) const
{
  // Check the temperature is in the range of validity (280 K <= T <= 1080 K)
  if (temperature <= 280.0 || temperature >= 1080.0)
    mooseError("Temperature ", temperature, "K out of range (280K, 1080K) in ", name(), ":cp()");

  std::vector<Real> a;
  if (temperature < 755.0)
    a = {1.9165258, -1.09269e-3, 8.696605e-6, -5.2291144e-9, 0.0, 0.0, 0.0};
  else
    a = {1.04356e1, -4.2025284e-2, 8.849006e-5, -8.4304566e-8, 3.9030203e-11, -7.1345169e-15, 0.0};

  Real specific_heat = 0.0;
  for (unsigned int i = 0; i < a.size(); ++i)
    specific_heat += a[i] * std::pow(temperature, i);

  // convert to J/kg/K by multiplying by 1000
  return specific_heat * 1000.0;
}

Real
MethaneFluidProperties::cv(Real pressure, Real temperature) const
{
  return cp(pressure, temperature) - _R / _Mch4;
}

Real
MethaneFluidProperties::mu(Real /*density*/, Real temperature) const
{
  // Check the temperature is in the range of validity (200 K <= T <= 1000 K)
  if (temperature <= 200.0 || temperature >= 1000.0)
    mooseError("Temperature ", temperature, "K out of range (200K, 1000K) in ", name(), ":mu()");

  const std::vector<Real> a{
      2.968267e-1, 3.711201e-2, 1.218298e-5, -7.02426e-8, 7.543269e-11, -2.7237166e-14};

  Real viscosity = 0.0;
  for (unsigned int i = 0; i < a.size(); ++i)
    viscosity += a[i] * std::pow(temperature, i);

  return viscosity * 1.e-6;
}

void
MethaneFluidProperties::mu_drhoT(
    Real density, Real temperature, Real & mu, Real & dmu_drho, Real & dmu_dT) const
{
  const std::vector<Real> a{
      2.968267e-1, 3.711201e-2, 1.218298e-5, -7.02426e-8, 7.543269e-11, -2.7237166e-14};

  mu = this->mu(density, temperature);
  dmu_drho = 0.0;

  Real dmudt = 0.0;
  for (unsigned int i = 0; i < a.size(); ++i)
    dmudt += i * a[i] * std::pow(temperature, i - 1.0);
  dmu_dT = dmudt * 1.e-6;
}

Real
MethaneFluidProperties::k(Real /*pressure*/, Real temperature) const
{
  // Check the temperature is in the range of validity (200 K <= T <= 1000 K)
  if (temperature <= 200.0 || temperature >= 1000.0)
    mooseError("Temperature ", temperature, "K out of range (200K, 1000K) in ", name(), ":k()");

  const std::vector<Real> a{-1.3401499e-2,
                            3.663076e-4,
                            -1.82248608e-6,
                            5.93987998e-9,
                            -9.1405505e-12,
                            6.7896889e-15,
                            -1.95048736e-18};

  Real kt = 0.0;
  for (unsigned int i = 0; i < a.size(); ++i)
    kt += a[i] * std::pow(temperature, i);

  return kt;
}

Real
MethaneFluidProperties::s(Real /*pressure*/, Real temperature) const
{
  // Check the temperature is in the range of validity (280 K <= t <= 1080 K)
  if (temperature <= 280.0 || temperature >= 1080.0)
    mooseError("Temperature ", temperature, "K out of range (280K, 1080K) in ", name(), ":s()");

  std::vector<Real> a;
  if (temperature < 755.0)
    a = {1.9165258, -1.09269e-3, 8.696605e-6, -5.2291144e-9, 0.0, 0.0, 0.0};
  else
    a = {1.04356e1, -4.2025284e-2, 8.849006e-5, -8.4304566e-8, 3.9030203e-11, -7.1345169e-15, 0.0};

  Real entropy = a[0] * std::log(temperature);
  for (unsigned int i = 1; i < a.size(); ++i)
    entropy += a[i] * std::pow(temperature, i) / static_cast<Real>(i);

  // convert to J/kg/K by multiplying by 1000
  return entropy * 1000.0;
}

Real
MethaneFluidProperties::h(Real /*pressure*/, Real temperature) const
{
  // Check the temperature is in the range of validity (280 K <= t <= 1080 K)
  if (temperature <= 280.0 || temperature >= 1080.0)
    mooseError("Temperature ", temperature, "K out of range (280K, 1080K) in ", name(), ":cp()");

  std::vector<Real> a;
  if (temperature < 755.0)
    a = {1.9165258, -1.09269e-3, 8.696605e-6, -5.2291144e-9, 0.0, 0.0, 0.0};
  else
    a = {1.04356e1, -4.2025284e-2, 8.849006e-5, -8.4304566e-8, 3.9030203e-11, -7.1345169e-15, 0.0};

  Real enthalpy = 0.0;
  for (unsigned int i = 0; i < a.size(); ++i)
    enthalpy += a[i] * std::pow(temperature, i + 1) / (i + 1.0);

  // convert to J/kg by multiplying by 1000
  return enthalpy * 1000.0;
}

void
MethaneFluidProperties::h_dpT(
    Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const
{
  h = this->h(pressure, temperature);
  // Enthalpy doesn't depend on pressure
  dh_dp = 0.0;

  std::vector<Real> a;
  if (temperature < 755.0)
    a = {1.9165258, -1.09269e-3, 8.696605e-6, -5.2291144e-9, 0.0, 0.0, 0.0};
  else
    a = {1.04356e1, -4.2025284e-2, 8.849006e-5, -8.4304566e-8, 3.9030203e-11, -7.1345169e-15, 0.0};

  Real dhdt = 0.0;
  for (unsigned int i = 0; i < a.size(); ++i)
    dhdt += a[i] * std::pow(temperature, i);

  // convert to J/kg/K by multiplying by 1000
  dh_dT = dhdt * 1000.0;
}

Real MethaneFluidProperties::beta(Real /*pressure*/, Real /*temperature*/) const { return _beta; }

Real
MethaneFluidProperties::henryConstant(Real temperature) const
{
  return henryConstantIAPWS(temperature, -10.44708, 4.66491, 12.12986);
}

void
MethaneFluidProperties::henryConstant_dT(Real temperature, Real & Kh, Real & dKh_dT) const
{
  henryConstantIAPWS_dT(temperature, Kh, dKh_dT, -10.44708, 4.66491, 12.12986);
}
