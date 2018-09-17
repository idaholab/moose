//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MethaneFluidProperties.h"
#include "Conversion.h"
#include "MathUtils.h"
#include "libmesh/utility.h"

registerMooseObject("FluidPropertiesApp", MethaneFluidProperties);

template <>
InputParameters
validParams<MethaneFluidProperties>()
{
  InputParameters params = validParams<SinglePhaseFluidPropertiesPT>();
  params.addClassDescription("Fluid properties for methane (CH4)");
  return params;
}

MethaneFluidProperties::MethaneFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidPropertiesPT(parameters),
    _Mch4(16.0425e-3),
    _p_critical(4.5992e6),
    _T_critical(190.564),
    _rho_critical(162.66),
    _p_triple(1.169e4),
    _T_triple(90.67)
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
MethaneFluidProperties::rho_from_p_T(Real pressure, Real temperature) const
{
  return pressure * _Mch4 / (_R * temperature);
}

void
MethaneFluidProperties::rho_from_p_T(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = this->rho_from_p_T(pressure, temperature);
  drho_dp = _Mch4 / (_R * temperature);
  drho_dT = -pressure * _Mch4 / (_R * temperature * temperature);
}

Real
MethaneFluidProperties::e_from_p_T(Real pressure, Real temperature) const
{
  return h_from_p_T(pressure, temperature) - _R * temperature / _Mch4;
}

void
MethaneFluidProperties::e_from_p_T(
    Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const
{
  e = this->e_from_p_T(pressure, temperature);
  Real enthalpy, enthalpy_dp, enthalpy_dT;
  h_from_p_T(pressure, temperature, enthalpy, enthalpy_dp, enthalpy_dT);
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
  rho_from_p_T(pressure, temperature, density, ddensity_dp, ddensity_dT);
  rho = density;
  drho_dp = ddensity_dp;
  drho_dT = ddensity_dT;

  Real energy, denergy_dp, denergy_dT;
  e_from_p_T(pressure, temperature, energy, denergy_dp, denergy_dT);
  e = energy;
  de_dp = denergy_dp;
  de_dT = denergy_dT;
}

Real
MethaneFluidProperties::c_from_p_T(Real pressure, Real temperature) const
{
  return std::sqrt(gamma_from_p_T(pressure, temperature) * _R * temperature / _Mch4);
}

Real
MethaneFluidProperties::cp_from_p_T(Real /*pressure*/, Real temperature) const
{
  // Check the temperature is in the range of validity (280 K <= T <= 1080 K)
  if (temperature <= 280.0 || temperature >= 1080.0)
    throw MooseException("Temperature " + Moose::stringify(temperature) +
                         "K out of range (280K, 1080K) in " + name() + ": cp_from_p_T()");

  Real specific_heat = 0.0;
  if (temperature < 755.0)
    for (std::size_t i = 0; i < _a0.size(); ++i)
      specific_heat += _a0[i] * MathUtils::pow(temperature, i);
  else
    for (std::size_t i = 0; i < _a1.size(); ++i)
      specific_heat += _a1[i] * MathUtils::pow(temperature, i);

  // convert to J/kg/K by multiplying by 1000
  return specific_heat * 1000.0;
}

Real
MethaneFluidProperties::cv_from_p_T(Real pressure, Real temperature) const
{
  return cp_from_p_T(pressure, temperature) - _R / _Mch4;
}

Real
MethaneFluidProperties::mu_from_p_T(Real /*pressure*/, Real temperature) const
{
  // Check the temperature is in the range of validity (200 K <= T <= 1000 K)
  if (temperature <= 200.0 || temperature >= 1000.0)
    throw MooseException("Temperature " + Moose::stringify(temperature) +
                         "K out of range (200K, 1000K) in " + name() + ": mu_from_p_T()");

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

void
MethaneFluidProperties::rho_mu(Real pressure, Real temperature, Real & rho, Real & mu) const
{
  rho = this->rho_from_p_T(pressure, temperature);
  mu = this->mu_from_p_T(pressure, temperature);
}

void
MethaneFluidProperties::rho_mu_dpT(Real pressure,
                                   Real temperature,
                                   Real & rho,
                                   Real & drho_dp,
                                   Real & drho_dT,
                                   Real & mu,
                                   Real & dmu_dp,
                                   Real & dmu_dT) const
{
  this->rho_from_p_T(pressure, temperature, rho, drho_dp, drho_dT);
  this->mu_from_p_T(pressure, temperature, mu, dmu_dp, dmu_dT);
}

Real
MethaneFluidProperties::k_from_p_T(Real /*pressure*/, Real temperature) const
{
  // Check the temperature is in the range of validity (200 K <= T <= 1000 K)
  if (temperature <= 200.0 || temperature >= 1000.0)
    throw MooseException("Temperature " + Moose::stringify(temperature) +
                         "K out of range (200K, 1000K) in " + name() + ": k()");

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
    throw MooseException("Temperature " + Moose::stringify(temperature) +
                         "K out of range (200K, 1000K) in " + name() + ": k()");

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
MethaneFluidProperties::s_from_p_T(Real /*pressure*/, Real temperature) const
{
  // Check the temperature is in the range of validity (280 K <= t <= 1080 K)
  if (temperature <= 280.0 || temperature >= 1080.0)
    throw MooseException("Temperature " + Moose::stringify(temperature) +
                         "K out of range (280K, 1080K) in " + name() + ": s()");

  Real entropy;
  if (temperature < 755.0)
  {
    entropy = _a0[0] * std::log(temperature);
    for (std::size_t i = 1; i < _a0.size(); ++i)
      entropy += _a0[i] * MathUtils::pow(temperature, i) / static_cast<Real>(i);
  }
  else
  {
    entropy = _a1[0] * std::log(temperature);
    for (std::size_t i = 1; i < _a1.size(); ++i)
      entropy += _a1[i] * MathUtils::pow(temperature, i) / static_cast<Real>(i);

    // As entropy is obtained from cp by integration in this formulation, a zero
    // shift is required in this regime, see Irvine and Liley (1984)
    entropy -= 39.768;
  }

  // convert to J/kg/K by multiplying by 1000
  return entropy * 1000.0;
}

void
MethaneFluidProperties::s_from_p_T(Real p, Real T, Real & s, Real & ds_dp, Real & ds_dT) const
{
  SinglePhaseFluidProperties::s_from_p_T(p, T, s, ds_dp, ds_dT);
}

Real
MethaneFluidProperties::h_from_p_T(Real /*pressure*/, Real temperature) const
{
  // Check the temperature is in the range of validity (280 K <= t <= 1080 K)
  if (temperature <= 280.0 || temperature >= 1080.0)
    throw MooseException("Temperature " + Moose::stringify(temperature) +
                         "K out of range (280K, 1080K) in " + name() + ": cp_from_p_T()");

  Real enthalpy = 0.0;
  if (temperature < 755.0)
    for (std::size_t i = 0; i < _a0.size(); ++i)
      enthalpy += _a0[i] * MathUtils::pow(temperature, i + 1) / (i + 1.0);
  else
  {
    for (std::size_t i = 0; i < _a1.size(); ++i)
      enthalpy += _a1[i] * MathUtils::pow(temperature, i + 1) / (i + 1.0);

    // As enthalpy is obtained from cp by integration in this formulation, a zero
    // shift is required in this regime, see Irvine and Liley (1984)
    enthalpy -= 1.4837e3;
  }

  // convert to J/kg by multiplying by 1000
  return enthalpy * 1000.0;
}

void
MethaneFluidProperties::h_from_p_T(
    Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const
{
  h = this->h_from_p_T(pressure, temperature);
  // Enthalpy doesn't depend on pressure
  dh_dp = 0.0;

  Real dhdt = 0.0;
  if (temperature < 755.0)
    for (std::size_t i = 0; i < _a0.size(); ++i)
      dhdt += _a0[i] * MathUtils::pow(temperature, i);
  else
    for (std::size_t i = 0; i < _a1.size(); ++i)
      dhdt += _a1[i] * MathUtils::pow(temperature, i);

  // convert to J/kg/K by multiplying by 1000
  dh_dT = dhdt * 1000.0;
}

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
