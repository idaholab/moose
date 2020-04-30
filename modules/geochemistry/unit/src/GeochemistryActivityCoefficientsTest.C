//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "GeochemistryActivityCoefficients.h"
#include "GeochemistryActivityCalculators.h"

const GeochemicalDatabaseReader database("database/moose_testdb.json");
// The following system has secondary species: (O-phth)--, CO2(aq), CO3--, CaCO3, CaOH+, OH-, e-,
// >(s)FeO-, Calcite
const PertinentGeochemicalSystem model(database,
                                       {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++", ">(s)FeOH"},
                                       {"Calcite"},
                                       {},
                                       {"Calcite_asdf"},
                                       {"CH4(aq)"},
                                       {">(s)FeOCa+"},
                                       "O2(aq)",
                                       "e-");
const ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

/// Test interface with ionic strength object
TEST(GeochemistryActivityCoefficientsTest, ionicStrengthInterface)
{
  GeochemistryActivityCoefficients ac(
      GeochemistryActivityCoefficients::ActivityCoefficientMethodEnum::DEBYE_HUCKEL,
      1.0E9,
      2.0E9,
      false);

  EXPECT_EQ(ac.getMaxIonicStrength(), 1.0E9);
  ac.setMaxIonicStrength(1.0E8);
  EXPECT_EQ(ac.getMaxIonicStrength(), 1.0E8);
  EXPECT_EQ(ac.getMaxStoichiometricIonicStrength(), 2.0E9);
  ac.setMaxStoichiometricIonicStrength(2.0E-8);
  EXPECT_EQ(ac.getMaxStoichiometricIonicStrength(), 2.0E-8);
  EXPECT_TRUE(!ac.getUseOnlyBasisMolality());
  ac.setUseOnlyBasisMolality(true);
  EXPECT_TRUE(ac.getUseOnlyBasisMolality());

  Real gold_ionic_str = 0.0;
  std::vector<Real> basis_m(6);
  basis_m[mgd.basis_species_index.at("H2O")] = 1.0;
  basis_m[mgd.basis_species_index.at("H+")] = 2.0;
  gold_ionic_str += 2.0;
  basis_m[mgd.basis_species_index.at("HCO3-")] = 3.0;
  gold_ionic_str += 3.0;
  basis_m[mgd.basis_species_index.at("O2(aq)")] = 4.0;
  basis_m[mgd.basis_species_index.at("Ca++")] = 5.0;
  gold_ionic_str += 4 * 5.0;
  basis_m[mgd.basis_species_index.at(">(s)FeOH")] = 6.0;

  ac.setInternalParameters(25.0, mgd, basis_m, std::vector<Real>(9), std::vector<Real>(3));

  EXPECT_NEAR(ac.getIonicStrength(), 0.5 * gold_ionic_str, 1E-8);
  EXPECT_EQ(ac.getStoichiometricIonicStrength(), 2.0E-8);
}

/// Test getDebyeHuckel
TEST(GeochemistryActivityCoefficientsTest, getDebyeHuckel)
{
  GeochemistryActivityCoefficients ac(
      GeochemistryActivityCoefficients::ActivityCoefficientMethodEnum::DEBYE_HUCKEL,
      1.0E9,
      2.0E9,
      false);
  ac.setInternalParameters(
      25.0, mgd, std::vector<Real>(6, 1.0), std::vector<Real>(9), std::vector<Real>(3));
  std::vector<Real> dh = ac.getDebyeHuckel();

  const std::vector<Real> dh_gold = {0.5092,
                                     0.3283,
                                     0.035,
                                     1.45397,
                                     0.022357,
                                     0.0093804,
                                     -0.0005262,
                                     0.1127,
                                     -0.01049,
                                     0.001545,
                                     0.0};

  EXPECT_EQ(dh.size(), dh_gold.size());
  for (unsigned i = 0; i < dh_gold.size(); ++i)
    EXPECT_EQ(dh[i], dh_gold[i]);
}

/// Test calculate activity coefficients for method=DebyeHuckel
TEST(GeochemistryActivityCoefficientsTest, buildActivityCoefficients)
{
  GeochemistryActivityCoefficients ac(
      GeochemistryActivityCoefficients::ActivityCoefficientMethodEnum::DEBYE_HUCKEL,
      1.0E9,
      2.0E9,
      false);
  ac.setInternalParameters(
      25.0, mgd, std::vector<Real>(6, 1.0), std::vector<Real>(9), std::vector<Real>(3));

  const Real ionic_str = ac.getIonicStrength();

  std::vector<Real> basis_ac;
  std::vector<Real> eqm_ac;
  ac.buildActivityCoefficients(mgd, basis_ac, eqm_ac);

  EXPECT_EQ(basis_ac.size(), 6);
  EXPECT_EQ(eqm_ac.size(), 9);

  std::vector<Real> dh = ac.getDebyeHuckel();

  // Note that GeochemistryActivity has been tested elsewhere: here we're just checking the slots
  // get filled correctly
  for (unsigned i = 1; i < 6; ++i) // don't loop over water
  {
    Real gold = 0.0;
    if (i == 3) // O2(aq)
      gold = std::pow(10.0,
                      GeochemistryActivityCalculators::log10ActCoeffDHBdotNeutral(
                          ionic_str, dh[7], dh[8], dh[9], dh[10]));
    else if (i == 5) // >(s)FeOH
      gold = 1.0;
    else
      gold =
          std::pow(10.0,
                   GeochemistryActivityCalculators::log10ActCoeffDHBdot(mgd.basis_species_charge[i],
                                                                        mgd.basis_species_radius[i],
                                                                        std::sqrt(ionic_str),
                                                                        dh[0],
                                                                        dh[1],
                                                                        dh[2]));
    ASSERT_NEAR(basis_ac[i], gold, 1.0E-8);
  }

  for (unsigned j = 0; j < 8; ++j) // don't loop over the mineral calcite
  {
    Real gold = 0.0;
    if (j == 1 || j == 3) // CO2(aq) and CaCO3
      gold = 1.0;
    else
      gold =
          std::pow(10.0,
                   GeochemistryActivityCalculators::log10ActCoeffDHBdot(mgd.eqm_species_charge[j],
                                                                        mgd.eqm_species_radius[j],
                                                                        std::sqrt(ionic_str),
                                                                        dh[0],
                                                                        dh[1],
                                                                        dh[2]));
    ASSERT_NEAR(eqm_ac[j], gold, 1.0E-8);
  }
}

/// Test water activity for method=DebyeHuckel
TEST(GeochemistryActivityCoefficientsTest, waterActivity)
{
  GeochemistryActivityCoefficients ac(
      GeochemistryActivityCoefficients::ActivityCoefficientMethodEnum::DEBYE_HUCKEL,
      1.0E9,
      2.0E9,
      false);
  ac.setInternalParameters(
      25.0, mgd, std::vector<Real>(6, 1.0), std::vector<Real>(9), std::vector<Real>(3));

  std::vector<Real> dh = ac.getDebyeHuckel();
  const Real stoi_ionic_str = ac.getStoichiometricIonicStrength();

  // note that GeochemistryActivityCalculators::lnActivityDHBdotWater has been tested elsewhere:
  // this is just testing the information gets passed through to GeochemistryActivityCoefficients
  EXPECT_NEAR(ac.waterActivity(),
              std::exp(GeochemistryActivityCalculators::lnActivityDHBdotWater(
                  stoi_ionic_str, dh[0], dh[3], dh[4], dh[5], dh[6])),
              1.0E-8);
}
