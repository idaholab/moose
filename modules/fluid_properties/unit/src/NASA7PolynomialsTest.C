//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "NASA7Polynomials.h"
#include "ADReal.h"

namespace
{
/// Dimensionless standard-state properties of a species at one temperature
struct Reference
{
  std::string species;
  Real cp_R;
  Real h_RT;
  Real s_R;
};

/**
 * The reference values were computed with Cantera 3.2.0 from gri30.yaml as cp/R, h/(RT), and s/R.
 * Cantera evaluates the same coefficients, so only floating-point roundoff, from a different order
 * of operations, separates the results.
 */
const Real tol = 1e-12;

void
expectCanteraValues(const Real T, const std::array<Reference, 7> & references)
{
  for (const auto & ref : references)
  {
    const auto & data = NASA7::speciesData(ref.species);
    EXPECT_NEAR(NASA7::cpOverR(data, T), ref.cp_R, tol * std::abs(ref.cp_R)) << ref.species;
    EXPECT_NEAR(NASA7::hOverRT(data, T), ref.h_RT, tol * std::abs(ref.h_RT)) << ref.species;
    EXPECT_NEAR(NASA7::sOverR(data, T), ref.s_R, tol * std::abs(ref.s_R)) << ref.species;
  }
}
}

TEST(NASA7PolynomialsTest, canteraBelowMidpoint)
{
  // 923.15 K (650 C) is a typical operating temperature of solid oxide cells
  expectCanteraValues(923.15,
                      {{{"H2", 3.613168165140949, 2.393557882983218, 19.70388492877914},
                        {"O2", 4.147314656442323, 2.611086604285082, 28.963169712215056},
                        {"H2O", 4.844876850286245, -28.52667487834571, 27.59941233403597},
                        {"CH4", 8.411873653444164, -5.402055033541204, 29.171138189568893},
                        {"CO", 3.9393320432028016, -11.904521875475112, 27.892178607608198},
                        {"CO2", 6.410478331025108, -47.455763652874914, 31.870172746438772},
                        {"N2", 3.883883836068483, 2.4714182404274005, 27.11985064871594}}});
}

TEST(NASA7PolynomialsTest, canteraAtMidpoint)
{
  // Cantera uses the low-temperature polynomial at the midpoint temperature itself
  expectCanteraValues(1000.0,
                      {{{"H2", 3.6277926600000017, 2.4880181500000003, 19.99356155536211},
                        {"O2", 4.195457489999999, 2.7310016248333326, 29.296709192476225},
                        {"H2O", 4.9666161199999985, -25.957432841000003, 27.991587242388515},
                        {"CH4", 8.854050229999995, -4.323604100000004, 29.861079446444933},
                        {"CO", 3.9885755050000005, -10.6849246888, 28.209233158602412},
                        {"CO2", 6.533298272000001, -43.31136104626667, 32.38768755547865},
                        {"N2", 3.9403564, 2.5822312499999995, 27.43274636040696}}});
}

TEST(NASA7PolynomialsTest, canteraAboveMidpoint)
{
  expectCanteraValues(1500.0,
                      {{{"H2", 3.8822959451999997, 2.9101473592241667, 21.51235276622741},
                        {"O2", 4.3989938846625, 3.2555381173887485, 31.039298778830236},
                        {"H2O", 5.687841430574999, -15.524086927924582, 30.14793701207187},
                        {"CH4", 10.874274296875003, 0.43494356952083546, 33.86860929626944},
                        {"CO", 4.234951016024999, -5.748131851494999, 29.878586100354962},
                        {"CO2", 7.023470866475001, -26.605086887146665, 35.14116319971301},
                        {"N2", 4.1861203930625, 3.079423123570834, 29.081165481314585}}});
}

TEST(NASA7PolynomialsTest, molarMass)
{
  // Molecular weights reported by Cantera 3.2.0 for gri30.yaml [g/mol]
  const std::array<std::pair<std::string, Real>, 7> molecular_weights = {{{"H2", 2.016},
                                                                          {"O2", 31.998},
                                                                          {"H2O", 18.015},
                                                                          {"CH4", 16.043},
                                                                          {"CO", 28.010},
                                                                          {"CO2", 44.009},
                                                                          {"N2", 28.014}}};

  // The tolerance [kg/mol] only absorbs roundoff in the conversion from g/mol; it is far below the
  // 1e-6 kg/mol resolution of the reported values
  for (const auto & [species, molecular_weight] : molecular_weights)
    EXPECT_NEAR(NASA7::speciesData(species).molar_mass, 1e-3 * molecular_weight, 1e-15) << species;
}

TEST(NASA7PolynomialsTest, consistentDerivatives)
{
  // The enthalpy and entropy polynomials are integrals of the heat capacity polynomial:
  // d(h/R)/dT = cp/R and d(s/R)/dT = cp/(R T). Check this with automatic differentiation on both
  // sides of the midpoint temperature.
  DNDerivativeType dTdT;
  Moose::derivInsert(dTdT, 0, 1);

  for (const std::string species : {"H2", "O2", "H2O", "CH4", "CO", "CO2", "N2"})
    for (const Real T_value : {600.0, 2000.0})
    {
      const auto & data = NASA7::speciesData(species);
      const ADReal T(T_value, dTdT);

      const Real cp_R = NASA7::cpOverR(data, T_value);
      const ADReal h_R = T * NASA7::hOverRT(data, T);
      const ADReal s_R = NASA7::sOverR(data, T);
      EXPECT_NEAR(h_R.derivatives()[0], cp_R, tol * cp_R) << species << " at " << T_value;
      EXPECT_NEAR(s_R.derivatives()[0], cp_R / T_value, tol * cp_R / T_value)
          << species << " at " << T_value;
    }
}
