//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NaClFluidProperties.h"

registerMooseObject("FluidPropertiesApp", NaClFluidProperties);

InputParameters
NaClFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
  params.addClassDescription("Fluid properties for NaCl");
  return params;
}

NaClFluidProperties::NaClFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters),
    _Mnacl(58.443e-3),
    _p_critical(1.82e7),
    _T_critical(3841.15),
    _rho_critical(108.43),
    _p_triple(50.0),
    _T_triple(1073.85)
{
}

NaClFluidProperties::~NaClFluidProperties() {}

std::string
NaClFluidProperties::fluidName() const
{
  return "nacl";
}

Real
NaClFluidProperties::molarMass() const
{
  return _Mnacl;
}

Real
NaClFluidProperties::criticalPressure() const
{
  return _p_critical;
}

Real
NaClFluidProperties::criticalTemperature() const
{
  return _T_critical;
}

Real
NaClFluidProperties::criticalDensity() const
{
  return _rho_critical;
}

Real
NaClFluidProperties::triplePointPressure() const
{
  return _p_triple;
}

Real
NaClFluidProperties::triplePointTemperature() const
{
  return _T_triple;
}

Real
NaClFluidProperties::rho_from_p_T(Real pressure, Real temperature) const
{
  // Correlation needs pressure in bar
  Real pbar = pressure * 1.0e-5;
  // Correlation requires temperature in Celcius
  Real Tc = temperature - _T_c2k;

  // Halite density at 0 Pa
  Real density_P0 = 2.17043e3 - 2.4599e-1 * Tc - 9.5797e-5 * Tc * Tc;

  // Halite density as a function of pressure
  Real l = 5.727e-3 + 2.715e-3 * std::exp(Tc / 733.4);

  return density_P0 + l * pbar;
}

void
NaClFluidProperties::rho_from_p_T(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = this->rho_from_p_T(pressure, temperature);

  // Correlation needs pressure in bar
  Real pbar = pressure * 1.0e-5;
  // Correlation requires temperature in Celcius
  Real Tc = temperature - _T_c2k;

  // Halite density at 0 Pa
  Real ddensity_P0_dT = -2.4599e-1 - 1.91594e-4 * Tc;

  Real l = 5.727e-3 + 2.715e-3 * std::exp(Tc / 733.4);
  Real dl_dT = 2.715e-3 * std::exp(Tc / 733.4) / 733.4;

  drho_dp = l * 1.0e-5;
  drho_dT = ddensity_P0_dT + dl_dT * pbar;
}

Real
NaClFluidProperties::e_from_p_T(Real pressure, Real temperature) const
{
  return h_from_p_T(pressure, temperature) - pressure / rho_from_p_T(pressure, temperature);
}

void
NaClFluidProperties::e_from_p_T(
    Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const
{
  Real h, dh_dp, dh_dT;
  h_from_p_T(pressure, temperature, h, dh_dp, dh_dT);
  Real rho, drho_dp, drho_dT;
  rho_from_p_T(pressure, temperature, rho, drho_dp, drho_dT);

  e = h - pressure / rho;
  de_dp = dh_dp + pressure * drho_dp / rho / rho - 1.0 / rho;
  de_dT = dh_dT + pressure * drho_dT / rho / rho;
}

Real
NaClFluidProperties::cp_from_p_T(Real pressure, Real temperature) const
{
  // Correlation needs pressure in bar
  Real pbar = pressure * 10.0e-5;
  // Correlation requires temperature in Celcius
  Real Tc = temperature - _T_c2k;
  // Triple point temperature of NaCl (in C)
  Real Tt = _T_triple - _T_c2k;
  // Coefficients used in the correlation
  Real r3 = -1.7099e-3 - 3.82734e-6 * Tc - 8.65455e-9 * Tc * Tc;
  Real r4 = 5.29063e-8 - 9.63084e-11 * Tc + 6.50745e-13 * Tc * Tc;

  // Halite isobaric heat capapcity
  return 1148.81 + 0.551548 * (Tc - Tt) + 2.64309e-4 * (Tc - Tt) * (Tc - Tt) + r3 * pbar +
         r4 * pbar * pbar;
}

Real
NaClFluidProperties::cv_from_p_T(Real pressure, Real temperature) const
{
  return e_from_p_T(pressure, temperature) / temperature;
}

Real
NaClFluidProperties::k_from_p_T(Real /*pressure*/, Real temperature) const
{
  // Correlation requires temperature in Celcius
  Real Tc = temperature - _T_c2k;

  return 6.82793 - 3.16584e-2 * Tc + 1.03451e-4 * Tc * Tc - 1.48207e-7 * Tc * Tc * Tc;
}

void
NaClFluidProperties::k_from_p_T(
    Real /*pressure*/, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const
{
  // Correlation requires temperature in Celcius
  Real Tc = temperature - _T_c2k;

  k = 6.82793 - 3.16584e-2 * Tc + 1.03451e-4 * Tc * Tc - 1.48207e-7 * Tc * Tc * Tc;
  dk_dp = 0.0;
  dk_dT = -3.16584e-2 + 2.06902e-4 * Tc - 4.44621e-7 * Tc * Tc;
}

Real
NaClFluidProperties::h_from_p_T(Real pressure, Real temperature) const
{
  // Correlation needs pressure in bar
  Real pbar = pressure * 1.0e-5;
  // Correlation requires temperature in Celcius
  Real Tc = temperature - _T_c2k;
  // Triple point temperature of water (in C)
  Real Tt = 273.16 - _T_c2k;
  // Triple point presure of water (in bar)
  Real pt = 611.657 * 1.0e-5;

  // Note: the enthalpy of halite is 0 at the triple point of water
  return 8.7664e2 * (Tc - Tt) + 6.4139e-2 * (Tc * Tc - Tt * Tt) +
         8.8101e-5 * (Tc * Tc * Tc - Tt * Tt * Tt) + 44.14 * (pbar - pt);
}

void
NaClFluidProperties::h_from_p_T(
    Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const
{
  // Correlation needs pressure in bar
  Real pbar = pressure * 1.0e-5;
  // Correlation requires temperature in Celcius
  Real Tc = temperature - _T_c2k;
  // Triple point temperature of water (in C)
  Real Tt = 273.16 - _T_c2k;
  // Triple point presure of water (in bar)
  Real pt = 611.657 * 1.0e-5;

  // Note: the enthalpy of halite is 0 at the triple point of water
  h = 8.7664e2 * (Tc - Tt) + 6.4139e-2 * (Tc * Tc - Tt * Tt) +
      8.8101e-5 * (Tc * Tc * Tc - Tt * Tt * Tt) + 44.14 * (pbar - pt);

  dh_dp = 44.14 * 1.0e-5;
  dh_dT = 8.7664e2 + 2.0 * 6.4139e-2 * Tc + 3.0 * 8.8101e-5 * Tc * Tc;
}
