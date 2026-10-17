//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "metaphysicl/raw_type.h"

#include <array>
#include <cmath>
#include <string>

/**
 * Standard-state thermodynamic properties of ideal-gas species from NASA 7-coefficient
 * polynomials. Each species has two polynomials, one for temperatures up to a midpoint
 * temperature and one above it. The evaluators return the dimensionless forms cp/R, h/(RT), and
 * s/R, so that they are independent of the value of the universal gas constant.
 */
namespace NASA7
{
/// Coefficients a_0, ..., a_6 of the polynomial for one temperature interval
using Coefficients = std::array<Real, 7>;

/// NASA 7-coefficient data of one species
struct SpeciesData
{
  /// Species name, e.g. "H2O"
  std::string name;
  /// Molar mass [kg/mol]
  Real molar_mass;
  /// Lower end of the temperature range of the fit [K]
  Real T_min;
  /// Temperature where the low- and high-temperature polynomials meet [K]
  Real T_mid;
  /// Upper end of the temperature range of the fit [K]
  Real T_max;
  /// Coefficients for T <= T_mid
  Coefficients low;
  /// Coefficients for T > T_mid
  Coefficients high;
};

/**
 * Data of a built-in species
 *
 * @param[in] name   species name: H2, O2, H2O, CH4, CO, CO2, or N2
 */
const SpeciesData & speciesData(const std::string & name);

/**
 * Coefficients of the polynomial that applies at a temperature. The low-temperature polynomial
 * applies at the midpoint temperature itself, as in Cantera. The temperature range of the fit is
 * not checked.
 */
template <typename CppType>
const Coefficients &
coefficients(const SpeciesData & data, const CppType & T)
{
  return MetaPhysicL::raw_value(T) <= data.T_mid ? data.low : data.high;
}

/// Standard-state molar isobaric heat capacity divided by the gas constant, cp/R
template <typename CppType>
CppType
cpOverR(const SpeciesData & data, const CppType & T)
{
  const auto & a = coefficients(data, T);
  return a[0] + T * (a[1] + T * (a[2] + T * (a[3] + T * a[4])));
}

/// Standard-state molar enthalpy divided by the gas constant and temperature, h/(RT)
template <typename CppType>
CppType
hOverRT(const SpeciesData & data, const CppType & T)
{
  const auto & a = coefficients(data, T);
  return a[0] + T * (a[1] / 2 + T * (a[2] / 3 + T * (a[3] / 4 + T * a[4] / 5))) + a[5] / T;
}

/**
 * Standard-state molar entropy divided by the gas constant, s/R, at the reference pressure of the
 * data (101325 Pa for the built-in species)
 */
template <typename CppType>
CppType
sOverR(const SpeciesData & data, const CppType & T)
{
  using std::log;
  const auto & a = coefficients(data, T);
  return a[0] * log(T) + T * (a[1] + T * (a[2] / 2 + T * (a[3] / 3 + T * a[4] / 4))) + a[6];
}
}
