//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GasTransportUtilsTest.h"
#include "GasTransportUtils.h"
#include "NASA7Polynomials.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"

namespace
{
/// Molar mass of a built-in species [kg/mol]
Real
molarMass(const std::string & species)
{
  return NASA7::speciesData(species).molar_mass;
}
}

TEST_F(GasTransportUtilsTest, canteraViscosity)
{
  // Viscosities at 1e5 Pa computed with Cantera 3.2.0 from gri30.yaml for the species without a
  // dipole moment. Cantera evaluates the collision integrals and fits the viscosity differently,
  // and the two agree within 0.5% from 300 K to 3000 K.
  const Real tol = 6e-3;
  const std::vector<std::pair<Real, std::vector<std::pair<std::string, Real>>>> references = {
      {300.0,
       {{"H2", 9.000177310613688e-06},
        {"O2", 2.065378948167078e-05},
        {"CH4", 1.1453629801108389e-05},
        {"CO", 1.777126940240487e-05},
        {"CO2", 1.5048181672334403e-05},
        {"N2", 1.80854698821696e-05}}},
      {1000.0,
       {{"H2", 1.9676368365578972e-05},
        {"O2", 4.7911694307285326e-05},
        {"CH4", 2.7657970234929e-05},
        {"CO", 4.0801958261007516e-05},
        {"CO2", 4.09913251119492e-05},
        {"N2", 4.149814387750264e-05}}},
      {2500.0,
       {{"H2", 3.5525956370270646e-05},
        {"O2", 8.681334277254339e-05},
        {"CH4", 5.031808575692049e-05},
        {"CO", 7.387175858990157e-05},
        {"CO2", 7.612436248107707e-05},
        {"N2", 7.512886600196423e-05}}}};

  for (const auto & [T, values] : references)
    for (const auto & [species, mu] : values)
      REL_TEST(GasTransportUtils::viscosity(
                   T, molarMass(species), GasTransportUtils::lennardJonesParameters(species)),
               mu,
               tol);
}

TEST_F(GasTransportUtilsTest, canteraDiffusion)
{
  // Binary diffusion coefficients at 1e5 Pa computed with Cantera 3.2.0 from gri30.yaml. For pairs
  // without a dipole moment the two agree within 0.1% from 300 K to 3000 K. Cantera adds a dipole
  // correction for pairs with H2O, which changes their coefficients by up to 4%.
  const Real p = 1e5;
  const Real tol = 2e-3;
  const Real tol_H2O = 5e-2;
  struct Reference
  {
    Real T;
    std::string species_i;
    std::string species_j;
    Real D;
  };
  const std::vector<Reference> references = {{300.0, "N2", "O2", 2.114004397958847e-05},
                                             {300.0, "H2", "N2", 7.892785088901448e-05},
                                             {300.0, "CH4", "CO2", 1.7394895936669404e-05},
                                             {300.0, "H2", "CO", 7.818547081111362e-05},
                                             {300.0, "CO", "CO2", 1.58357230065214e-05},
                                             {300.0, "H2O", "N2", 2.2961566123205863e-05},
                                             {300.0, "H2O", "H2", 8.662561051685164e-05},
                                             {1000.0, "N2", "O2", 0.00016508453019480936},
                                             {1000.0, "H2", "N2", 0.0005928256767875003},
                                             {1000.0, "CH4", "CO2", 0.00014842228814550512},
                                             {1000.0, "H2", "CO", 0.0005873361123312881},
                                             {1000.0, "CO", "CO2", 0.00013074964295438555},
                                             {1000.0, "H2O", "N2", 0.0002111137941842763},
                                             {1000.0, "H2O", "H2", 0.000727354808885168},
                                             {2500.0, "N2", "O2", 0.0007553835721572624},
                                             {2500.0, "H2", "N2", 0.0027020824913847425},
                                             {2500.0, "CH4", "CO2", 0.0006888837584287778},
                                             {2500.0, "H2", "CO", 0.002677091367171154},
                                             {2500.0, "CO", "CO2", 0.0006031661492209093}};

  for (const auto & ref : references)
  {
    const Real D = GasTransportUtils::binaryDiffusionCoefficient(
        p,
        ref.T,
        molarMass(ref.species_i),
        molarMass(ref.species_j),
        GasTransportUtils::lennardJonesParameters(ref.species_i),
        GasTransportUtils::lennardJonesParameters(ref.species_j));
    REL_TEST(D, ref.D, ref.species_i == "H2O" ? tol_H2O : tol);
  }
}

TEST_F(GasTransportUtilsTest, polingExample)
{
  // Example 11-1 of Poling et al. (2001): N2 and CO2 at 590 K and 1 bar, with the Lennard-Jones
  // parameters and molar masses of that example. It reports 0.52 cm^2/s after rounding.
  const GasTransportUtils::LennardJonesParameters n2{3.798e-10, 71.4};
  const GasTransportUtils::LennardJonesParameters co2{3.941e-10, 195.2};
  const Real p = 1e5;
  const Real T = 590;
  ABS_TEST(GasTransportUtils::binaryDiffusionCoefficient(p, T, 28.014e-3, 44.010e-3, n2, co2),
           0.52e-4,
           0.005e-4);
}

TEST_F(GasTransportUtilsTest, steamZeroDensity)
{
  // Water97FluidProperties evaluates the same low-density viscosity times a factor that tends to
  // one with the density. At 1e-10 kg/m^3 that factor differs from one by about 4e-12.
  const Real rho = 1e-10;
  for (const Real T : {300.0, 600.0, 900.0, 1200.0})
    REL_TEST(GasTransportUtils::steamViscosityZeroDensity(T), _water->mu_from_rho_T(rho, T), 1e-10);

  // Verification values of the thermal conductivity at zero density from the IAPWS 2011 release,
  // given to nine significant digits in mW/(m K)
  REL_TEST(GasTransportUtils::steamConductivityZeroDensity(298.15), 18.4341883e-3, 1e-8);
  REL_TEST(GasTransportUtils::steamConductivityZeroDensity(873.15), 79.1034659e-3, 1e-8);
}

TEST_F(GasTransportUtilsTest, derivatives)
{
  // Derivatives from automatic differentiation with respect to temperature (index 0) and pressure
  // (index 1) agree with central differences
  const Real T = 800;
  const Real p = 2e5;
  DNDerivativeType dT_dT, dp_dp;
  Moose::derivInsert(dT_dT, 0, 1);
  Moose::derivInsert(dp_dp, 1, 1);
  const ADReal T_ad(T, dT_dT);
  const ADReal p_ad(p, dp_dp);
  const Real h = REL_PERTURBATION * T;

  const auto lj_H2 = GasTransportUtils::lennardJonesParameters("H2");
  const auto lj_N2 = GasTransportUtils::lennardJonesParameters("N2");
  const auto mu = [&lj_N2](const auto & T)
  { return GasTransportUtils::viscosity(T, molarMass("N2"), lj_N2); };
  const auto D = [&lj_H2, &lj_N2](const auto & p, const auto & T)
  {
    return GasTransportUtils::binaryDiffusionCoefficient(
        p, T, molarMass("H2"), molarMass("N2"), lj_H2, lj_N2);
  };

  REL_TEST(mu(T_ad).derivatives()[0], (mu(T + h) - mu(T - h)) / (2 * h), REL_TOL_DERIVATIVE);
  REL_TEST(
      D(p_ad, T_ad).derivatives()[0], (D(p, T + h) - D(p, T - h)) / (2 * h), REL_TOL_DERIVATIVE);
  // The diffusion coefficient is inversely proportional to pressure
  REL_TEST(D(p_ad, T_ad).derivatives()[1], -D(p, T) / p, REL_TOL_CONSISTENCY);
  REL_TEST(GasTransportUtils::steamViscosityZeroDensity(T_ad).derivatives()[0],
           (GasTransportUtils::steamViscosityZeroDensity(T + h) -
            GasTransportUtils::steamViscosityZeroDensity(T - h)) /
               (2 * h),
           REL_TOL_DERIVATIVE);
  REL_TEST(GasTransportUtils::steamConductivityZeroDensity(T_ad).derivatives()[0],
           (GasTransportUtils::steamConductivityZeroDensity(T + h) -
            GasTransportUtils::steamConductivityZeroDensity(T - h)) /
               (2 * h),
           REL_TOL_DERIVATIVE);
}
