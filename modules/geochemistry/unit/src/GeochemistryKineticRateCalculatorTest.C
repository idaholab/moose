//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "GeochemicalSystem.h"
#include "GeochemistryKineticRateCalculator.h"

const GeochemicalDatabaseReader db_kin("database/moose_testdb.json");
PertinentGeochemicalSystem model_kin(db_kin,
                                     {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++"},
                                     {},
                                     {},
                                     {"Calcite"},
                                     {"CH4(aq)"},
                                     {},
                                     "O2(aq)",
                                     "e-");
KineticRateUserDescription rate_ch4("CH4(aq)",
                                    1.5,
                                    2.0,
                                    true,
                                    {"H2O", "OH-", "O2(aq)", "CO2(aq)", "CaCO3"},
                                    {3.0, 3.1, 3.2, 3.3, 3.4},
                                    0.8,
                                    2.5,
                                    66.0,
                                    0.003);
KineticRateUserDescription
    rate_cal("Calcite", 7.0, 6.0, false, {"H+"}, {-3.0}, 2.5, 0.8, 55.0, 0.00315);
const ModelGeochemicalDatabase & mgd_kin = model_kin.modelGeochemicalDatabase();

/// Test exceptions in KineticRateUserDescription
TEST(KineticRateUserDescriptionTest, exceptions)
{

  try
  {
    KineticRateUserDescription rate(
        "CH4(aq)", 1.0, 2.0, true, {"H2O", "H+"}, {3.0}, 4.0, 5.0, 6.0, 7.0);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("The promoting_species and promoting_indices vectors must be the same size") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    KineticRateUserDescription rate(
        "CH4(aq)", 1.0, 2.0, true, {"H2O", "OH-", "H2O"}, {3.0, 1.0, -1.0}, 4.0, 5.0, 6.0, 7.0);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Promoting species H2O has already been provided with an exponent") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Test exceptions
TEST(GeochemistryKineticRateCalculatorTest, exceptions)
{
  model_kin.addKineticRate(rate_ch4);
  const std::vector<Real> promoting_indices(5 + 6, 0.0);
  const std::vector<std::string> basis_name = {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++"};
  const std::vector<bool> & basis_species_gas = mgd_kin.basis_species_gas;
  const std::vector<Real> basis_molality = {1.0, 2.0, 3.0, 4.0, 5.0};
  const std::vector<Real> basis_activity = {0.1, 0.2, 0.3, 0.4, 0.5};
  const std::vector<bool> basis_activity_known(5, false);
  const std::vector<std::string> & eqm_name = mgd_kin.eqm_species_name;
  const std::vector<bool> & eqm_species_gas = mgd_kin.eqm_species_gas;
  const std::vector<Real> eqm_molality = {1.5, 2.5, 3.5, 4.5, 5.5, 7.5};
  const std::vector<Real> eqm_activity = {1.1, 1.2, 1.3, 1.4, 1.5, 1.7};
  const DenseMatrix<Real> & eqm_stoichiometry = mgd_kin.eqm_stoichiometry;
  const DenseMatrix<Real> & kin_stoichiometry = mgd_kin.kin_stoichiometry;

  Real rate;
  Real drate_dkin;
  std::vector<Real> drate_dmol(5);

  try
  {
    GeochemistryKineticRateCalculator::calculateRate({},
                                                     rate_ch4,
                                                     basis_name,
                                                     basis_species_gas,
                                                     basis_molality,
                                                     basis_activity,
                                                     basis_activity_known,
                                                     eqm_name,
                                                     eqm_species_gas,
                                                     eqm_molality,
                                                     eqm_activity,
                                                     eqm_stoichiometry,
                                                     1.0,
                                                     2.0,
                                                     3.0,
                                                     4.0,
                                                     kin_stoichiometry,
                                                     0,
                                                     25.0,
                                                     rate,
                                                     drate_dkin,
                                                     drate_dmol);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("kinetic_rate: promoting_indices incorrectly sized 0") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    GeochemistryKineticRateCalculator::calculateRate(promoting_indices,
                                                     rate_ch4,
                                                     basis_name,
                                                     {},
                                                     basis_molality,
                                                     basis_activity,
                                                     basis_activity_known,
                                                     eqm_name,
                                                     eqm_species_gas,
                                                     eqm_molality,
                                                     eqm_activity,
                                                     eqm_stoichiometry,
                                                     1.0,
                                                     2.0,
                                                     3.0,
                                                     4.0,
                                                     kin_stoichiometry,
                                                     0,
                                                     25.0,
                                                     rate,
                                                     drate_dkin,
                                                     drate_dmol);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("kinetic_rate: incorrectly sized basis-species vectors 5 0 5 5 5 5") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    GeochemistryKineticRateCalculator::calculateRate(promoting_indices,
                                                     rate_ch4,
                                                     basis_name,
                                                     basis_species_gas,
                                                     {},
                                                     basis_activity,
                                                     basis_activity_known,
                                                     eqm_name,
                                                     eqm_species_gas,
                                                     eqm_molality,
                                                     eqm_activity,
                                                     eqm_stoichiometry,
                                                     1.0,
                                                     2.0,
                                                     3.0,
                                                     4.0,
                                                     kin_stoichiometry,
                                                     0,
                                                     25.0,
                                                     rate,
                                                     drate_dkin,
                                                     drate_dmol);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("kinetic_rate: incorrectly sized basis-species vectors 5 5 0 5 5 5") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    GeochemistryKineticRateCalculator::calculateRate(promoting_indices,
                                                     rate_ch4,
                                                     basis_name,
                                                     basis_species_gas,
                                                     basis_molality,
                                                     {},
                                                     basis_activity_known,
                                                     eqm_name,
                                                     eqm_species_gas,
                                                     eqm_molality,
                                                     eqm_activity,
                                                     eqm_stoichiometry,
                                                     1.0,
                                                     2.0,
                                                     3.0,
                                                     4.0,
                                                     kin_stoichiometry,
                                                     0,
                                                     25.0,
                                                     rate,
                                                     drate_dkin,
                                                     drate_dmol);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("kinetic_rate: incorrectly sized basis-species vectors 5 5 5 0 5 5") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    GeochemistryKineticRateCalculator::calculateRate(promoting_indices,
                                                     rate_ch4,
                                                     basis_name,
                                                     basis_species_gas,
                                                     basis_molality,
                                                     basis_activity,
                                                     {},
                                                     eqm_name,
                                                     eqm_species_gas,
                                                     eqm_molality,
                                                     eqm_activity,
                                                     eqm_stoichiometry,
                                                     1.0,
                                                     2.0,
                                                     3.0,
                                                     4.0,
                                                     kin_stoichiometry,
                                                     0,
                                                     25.0,
                                                     rate,
                                                     drate_dkin,
                                                     drate_dmol);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("kinetic_rate: incorrectly sized basis-species vectors 5 5 5 5 0 5") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    std::vector<Real> drdm(4);
    GeochemistryKineticRateCalculator::calculateRate(promoting_indices,
                                                     rate_ch4,
                                                     basis_name,
                                                     basis_species_gas,
                                                     basis_molality,
                                                     basis_activity,
                                                     basis_activity_known,
                                                     eqm_name,
                                                     eqm_species_gas,
                                                     eqm_molality,
                                                     eqm_activity,
                                                     eqm_stoichiometry,
                                                     1.0,
                                                     2.0,
                                                     3.0,
                                                     4.0,
                                                     kin_stoichiometry,
                                                     0,
                                                     25.0,
                                                     rate,
                                                     drate_dkin,
                                                     drdm);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("kinetic_rate: incorrectly sized basis-species vectors 5 5 5 5 5 4") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    GeochemistryKineticRateCalculator::calculateRate(promoting_indices,
                                                     rate_ch4,
                                                     basis_name,
                                                     basis_species_gas,
                                                     basis_molality,
                                                     basis_activity,
                                                     basis_activity_known,
                                                     eqm_name,
                                                     {},
                                                     eqm_molality,
                                                     eqm_activity,
                                                     eqm_stoichiometry,
                                                     1.0,
                                                     2.0,
                                                     3.0,
                                                     4.0,
                                                     kin_stoichiometry,
                                                     0,
                                                     25.0,
                                                     rate,
                                                     drate_dkin,
                                                     drate_dmol);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("kinetic_rate: incorrectly sized equilibrium-species vectors 6 0 6 6") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    GeochemistryKineticRateCalculator::calculateRate(promoting_indices,
                                                     rate_ch4,
                                                     basis_name,
                                                     basis_species_gas,
                                                     basis_molality,
                                                     basis_activity,
                                                     basis_activity_known,
                                                     eqm_name,
                                                     eqm_species_gas,
                                                     {},
                                                     eqm_activity,
                                                     eqm_stoichiometry,
                                                     1.0,
                                                     2.0,
                                                     3.0,
                                                     4.0,
                                                     kin_stoichiometry,
                                                     0,
                                                     25.0,
                                                     rate,
                                                     drate_dkin,
                                                     drate_dmol);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("kinetic_rate: incorrectly sized equilibrium-species vectors 6 6 0 6") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    GeochemistryKineticRateCalculator::calculateRate(promoting_indices,
                                                     rate_ch4,
                                                     basis_name,
                                                     basis_species_gas,
                                                     basis_molality,
                                                     basis_activity,
                                                     basis_activity_known,
                                                     eqm_name,
                                                     eqm_species_gas,
                                                     eqm_molality,
                                                     {},
                                                     eqm_stoichiometry,
                                                     1.0,
                                                     2.0,
                                                     3.0,
                                                     4.0,
                                                     kin_stoichiometry,
                                                     0,
                                                     25.0,
                                                     rate,
                                                     drate_dkin,
                                                     drate_dmol);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("kinetic_rate: incorrectly sized equilibrium-species vectors 6 6 6 0") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    DenseMatrix<Real> stoi(7, 4);
    GeochemistryKineticRateCalculator::calculateRate(promoting_indices,
                                                     rate_ch4,
                                                     basis_name,
                                                     basis_species_gas,
                                                     basis_molality,
                                                     basis_activity,
                                                     basis_activity_known,
                                                     eqm_name,
                                                     eqm_species_gas,
                                                     eqm_molality,
                                                     eqm_activity,
                                                     stoi,
                                                     1.0,
                                                     2.0,
                                                     3.0,
                                                     4.0,
                                                     kin_stoichiometry,
                                                     0,
                                                     25.0,
                                                     rate,
                                                     drate_dkin,
                                                     drate_dmol);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("kinetic_rate: incorrectly sized eqm stoichiometry matrix 7 4") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    DenseMatrix<Real> stoi(4, 5);
    GeochemistryKineticRateCalculator::calculateRate(promoting_indices,
                                                     rate_ch4,
                                                     basis_name,
                                                     basis_species_gas,
                                                     basis_molality,
                                                     basis_activity,
                                                     basis_activity_known,
                                                     eqm_name,
                                                     eqm_species_gas,
                                                     eqm_molality,
                                                     eqm_activity,
                                                     stoi,
                                                     1.0,
                                                     2.0,
                                                     3.0,
                                                     4.0,
                                                     kin_stoichiometry,
                                                     0,
                                                     25.0,
                                                     rate,
                                                     drate_dkin,
                                                     drate_dmol);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("kinetic_rate: incorrectly sized eqm stoichiometry matrix 4 5") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    DenseMatrix<Real> stoi(1, 5);
    GeochemistryKineticRateCalculator::calculateRate(promoting_indices,
                                                     rate_ch4,
                                                     basis_name,
                                                     basis_species_gas,
                                                     basis_molality,
                                                     basis_activity,
                                                     basis_activity_known,
                                                     eqm_name,
                                                     eqm_species_gas,
                                                     eqm_molality,
                                                     eqm_activity,
                                                     eqm_stoichiometry,
                                                     1.0,
                                                     2.0,
                                                     3.0,
                                                     4.0,
                                                     stoi,
                                                     1,
                                                     25.0,
                                                     rate,
                                                     drate_dkin,
                                                     drate_dmol);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("kinetic_rate: incorrectly sized kinetic stoichiometry matrix 1 5") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    DenseMatrix<Real> stoi(1, 4);
    GeochemistryKineticRateCalculator::calculateRate(promoting_indices,
                                                     rate_ch4,
                                                     basis_name,
                                                     basis_species_gas,
                                                     basis_molality,
                                                     basis_activity,
                                                     basis_activity_known,
                                                     eqm_name,
                                                     eqm_species_gas,
                                                     eqm_molality,
                                                     eqm_activity,
                                                     eqm_stoichiometry,
                                                     1.0,
                                                     2.0,
                                                     3.0,
                                                     4.0,
                                                     stoi,
                                                     0,
                                                     25.0,
                                                     rate,
                                                     drate_dkin,
                                                     drate_dmol);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("kinetic_rate: incorrectly sized kinetic stoichiometry matrix 1 4") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Test rate calculations in various scenarios
TEST(GeochemistryKineticRateCalculatorTest, rate1)
{
  model_kin.addKineticRate(rate_ch4);
  model_kin.addKineticRate(rate_cal);
  std::vector<Real> promoting_indices(5 + 7, 0.0);
  const std::vector<std::string> basis_name = {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++"};
  const std::vector<bool> basis_species_gas = {false, false, false, true, false};
  std::vector<Real> basis_molality = {1.5, 2.0, 3.0, 4.0, 5.0};
  const std::vector<Real> basis_activity = {0.1, 0.2, 0.3, 0.4, 0.5};
  const std::vector<bool> basis_activity_known = {false, false, false, true, true};
  const std::vector<std::string> eqm_name = {"OH-", "n2", "n3", "n4", "n5", "n6", "n7"};
  const std::vector<bool> eqm_species_gas = {false, false, false, true, false, false, false};
  const std::vector<Real> eqm_molality = {1.5, 2.5, 2.3, 2.1, 1.9, 1.7, 1.3};
  const std::vector<Real> eqm_activity = {1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7};
  DenseMatrix<Real> eqm_stoichiometry(7, 5);
  DenseMatrix<Real> kin_stoichiometry(2, 5);

  Real rate;
  Real drate_dkin;
  std::vector<Real> drate_dmol(5, 1.0);

  GeochemistryKineticRateCalculator::calculateRate(promoting_indices,
                                                   rate_ch4,
                                                   basis_name,
                                                   basis_species_gas,
                                                   basis_molality,
                                                   basis_activity,
                                                   basis_activity_known,
                                                   eqm_name,
                                                   eqm_species_gas,
                                                   eqm_molality,
                                                   eqm_activity,
                                                   eqm_stoichiometry,
                                                   1.25,
                                                   2.25,
                                                   3.5,
                                                   4.0,
                                                   kin_stoichiometry,
                                                   0,
                                                   50.0,
                                                   rate,
                                                   drate_dkin,
                                                   drate_dmol);
  EXPECT_NEAR(rate,
              -1.5 * 2.0 * 1.25 * 2.25 *
                  std::pow(std::abs(1 - std::pow(std::pow(10.0, 4.0 - 3.5), 0.8)), 2.5) *
                  std::exp(66.0 / GeochemistryConstants::GAS_CONSTANT *
                           (0.003 - 1.0 / (50.0 + GeochemistryConstants::CELSIUS_TO_KELVIN))),
              1.0E-6);
  EXPECT_NEAR(drate_dkin,
              -1.5 * 2.0 * 2.25 *
                  std::pow(std::abs(1 - std::pow(std::pow(10.0, 4.0 - 3.5), 0.8)), 2.5) *
                  std::exp(66.0 / GeochemistryConstants::GAS_CONSTANT *
                           (0.003 - 1.0 / (50.0 + GeochemistryConstants::CELSIUS_TO_KELVIN))),
              1.0E-6);
  for (unsigned i = 0; i < 5; ++i)
    ASSERT_EQ(drate_dmol[i], 0.0);

  GeochemistryKineticRateCalculator::calculateRate(promoting_indices,
                                                   rate_ch4,
                                                   basis_name,
                                                   basis_species_gas,
                                                   basis_molality,
                                                   basis_activity,
                                                   basis_activity_known,
                                                   eqm_name,
                                                   eqm_species_gas,
                                                   eqm_molality,
                                                   eqm_activity,
                                                   eqm_stoichiometry,
                                                   1.25,
                                                   2.25,
                                                   4.5,
                                                   4.0,
                                                   kin_stoichiometry,
                                                   0,
                                                   50.0,
                                                   rate,
                                                   drate_dkin,
                                                   drate_dmol);
  EXPECT_NEAR(rate,
              1.5 * 2.0 * 1.25 * 2.25 *
                  std::pow(std::abs(1 - std::pow(std::pow(10.0, 4.0 - 4.5), 0.8)), 2.5) *
                  std::exp(66.0 / GeochemistryConstants::GAS_CONSTANT *
                           (0.003 - 1.0 / (50.0 + GeochemistryConstants::CELSIUS_TO_KELVIN))),
              1.0E-6);
  EXPECT_NEAR(drate_dkin,
              1.5 * 2.0 * 2.25 *
                  std::pow(std::abs(1 - std::pow(std::pow(10.0, 4.0 - 4.5), 0.8)), 2.5) *
                  std::exp(66.0 / GeochemistryConstants::GAS_CONSTANT *
                           (0.003 - 1.0 / (50.0 + GeochemistryConstants::CELSIUS_TO_KELVIN))),
              1.0E-6);
  for (unsigned i = 0; i < 5; ++i)
    ASSERT_EQ(drate_dmol[i], 0.0);

  for (unsigned i = 0; i < 5; ++i)
    promoting_indices[i] = 0.5 * (i + 1);
  GeochemistryKineticRateCalculator::calculateRate(promoting_indices,
                                                   rate_cal,
                                                   basis_name,
                                                   basis_species_gas,
                                                   basis_molality,
                                                   basis_activity,
                                                   basis_activity_known,
                                                   eqm_name,
                                                   eqm_species_gas,
                                                   eqm_molality,
                                                   eqm_activity,
                                                   eqm_stoichiometry,
                                                   1.25,
                                                   2.25,
                                                   3.5,
                                                   4.0,
                                                   kin_stoichiometry,
                                                   0,
                                                   40.0,
                                                   rate,
                                                   drate_dkin,
                                                   drate_dmol);
  EXPECT_NEAR(rate,
              -7.0 * 6.0 * std::pow(1.5, 0.5) * std::pow(0.2, 1.0) * std::pow(3.0, 1.5) *
                  std::pow(0.4, 2.0) * std::pow(5.0, 2.5) *
                  std::pow(std::abs(1 - std::pow(std::pow(10.0, 4.0 - 3.5), 2.5)), 0.8) *
                  std::exp(55.0 / GeochemistryConstants::GAS_CONSTANT *
                           (0.00315 - 1.0 / (40.0 + GeochemistryConstants::CELSIUS_TO_KELVIN))),
              1.0E-6);
  EXPECT_EQ(drate_dkin, 0.0);
  EXPECT_NEAR(drate_dmol[0], 0.5 * rate / 1.5, 1.0E-6);
  EXPECT_NEAR(drate_dmol[1], 1.0 * rate / 2.0, 1.0E-6);
  EXPECT_NEAR(drate_dmol[2], 1.5 * rate / 3.0, 1.0E-6);
  EXPECT_EQ(drate_dmol[3], 0.0);
  EXPECT_NEAR(drate_dmol[4], 2.5 * rate / 5.0, 1.0E-6);

  for (unsigned i = 5; i < 12; ++i)
    promoting_indices[i] = -0.1 * (i + 1);
  GeochemistryKineticRateCalculator::calculateRate(promoting_indices,
                                                   rate_cal,
                                                   basis_name,
                                                   basis_species_gas,
                                                   basis_molality,
                                                   basis_activity,
                                                   basis_activity_known,
                                                   eqm_name,
                                                   eqm_species_gas,
                                                   eqm_molality,
                                                   eqm_activity,
                                                   eqm_stoichiometry,
                                                   1.25,
                                                   2.25,
                                                   3.5,
                                                   4.0,
                                                   kin_stoichiometry,
                                                   0,
                                                   40.0,
                                                   rate,
                                                   drate_dkin,
                                                   drate_dmol);
  EXPECT_NEAR(rate,
              -7.0 * 6.0 * std::pow(1.5, 0.5) * std::pow(0.2, 1.0) * std::pow(3.0, 1.5) *
                  std::pow(0.4, 2.0) * std::pow(5.0, 2.5) * std::pow(1.1, -0.6) *
                  std::pow(2.5, -0.7) * std::pow(2.3, -0.8) * std::pow(1.4, -0.9) *
                  std::pow(1.9, -1.0) * std::pow(1.7, -1.1) * std::pow(1.3, -1.2) *
                  std::pow(std::abs(1 - std::pow(std::pow(10.0, 4.0 - 3.5), 2.5)), 0.8) *
                  std::exp(55.0 / GeochemistryConstants::GAS_CONSTANT *
                           (0.00315 - 1.0 / (40.0 + GeochemistryConstants::CELSIUS_TO_KELVIN))),
              1.0E-6);
  EXPECT_EQ(drate_dkin, 0.0);
  EXPECT_NEAR(drate_dmol[0], 0.5 * rate / 1.5, 1.0E-6);
  EXPECT_NEAR(drate_dmol[1], 1.0 * rate / 2.0, 1.0E-6);
  EXPECT_NEAR(drate_dmol[2], 1.5 * rate / 3.0, 1.0E-6);
  EXPECT_EQ(drate_dmol[3], 0.0);
  EXPECT_NEAR(drate_dmol[4], 2.5 * rate / 5.0, 1.0E-6);

  // derivatives of activity coefficients are zero
  eqm_stoichiometry(0, 0) = 1.0;   // OH- depends on H2O
  eqm_stoichiometry(0, 1) = -1.25; // OH- depends on H+
  eqm_stoichiometry(0, 2) = 1.5;   // OH- depends on another thing
  eqm_stoichiometry(0, 3) = 3.5;   // OH- depends on the gas
  eqm_stoichiometry(3, 0) = 2.0;   // gas depends on H2O
  eqm_stoichiometry(3, 1) = -2.25; // gas depends on H+
  eqm_stoichiometry(3, 2) = -2.5;  // gas depends on another thing
  eqm_stoichiometry(3, 3) = -33.5; // gas depends on the gas
  GeochemistryKineticRateCalculator::calculateRate(promoting_indices,
                                                   rate_cal,
                                                   basis_name,
                                                   basis_species_gas,
                                                   basis_molality,
                                                   basis_activity,
                                                   basis_activity_known,
                                                   eqm_name,
                                                   eqm_species_gas,
                                                   eqm_molality,
                                                   eqm_activity,
                                                   eqm_stoichiometry,
                                                   1.25,
                                                   2.25,
                                                   3.5,
                                                   4.0,
                                                   kin_stoichiometry,
                                                   0,
                                                   40.0,
                                                   rate,
                                                   drate_dkin,
                                                   drate_dmol);
  EXPECT_NEAR(rate,
              -7.0 * 6.0 * std::pow(1.5, 0.5) * std::pow(0.2, 1.0) * std::pow(3.0, 1.5) *
                  std::pow(0.4, 2.0) * std::pow(5.0, 2.5) * std::pow(1.1, -0.6) *
                  std::pow(2.5, -0.7) * std::pow(2.3, -0.8) * std::pow(1.4, -0.9) *
                  std::pow(1.9, -1.0) * std::pow(1.7, -1.1) * std::pow(1.3, -1.2) *
                  std::pow(std::abs(1 - std::pow(std::pow(10.0, 4.0 - 3.5), 2.5)), 0.8) *
                  std::exp(55.0 / GeochemistryConstants::GAS_CONSTANT *
                           (0.00315 - 1.0 / (40.0 + GeochemistryConstants::CELSIUS_TO_KELVIN))),
              1.0E-6);
  EXPECT_EQ(drate_dkin, 0.0);
  EXPECT_NEAR(drate_dmol[0], 0.5 * rate / 1.5, 1.0E-6);
  EXPECT_NEAR(drate_dmol[1],
              1.0 * rate / 2.0 - 0.6 * (-1.25) * rate / 2.0 - 0.9 * (-2.25) * rate / 2.0,
              1.0E-6);
  EXPECT_NEAR(drate_dmol[2],
              1.5 * rate / 3.0 - 0.6 * (1.5) * rate / 3.0 - 0.9 * (-2.5) * rate / 3.0,
              1.0E-6);
  EXPECT_EQ(drate_dmol[3], 0.0);
  EXPECT_NEAR(drate_dmol[4], 2.5 * rate / 5.0, 1.0E-6);

  // nonzero contributions from kinetic stoichiometry via the activity product
  eqm_stoichiometry.zero();
  kin_stoichiometry(0, 0) = -1.0;
  kin_stoichiometry(0, 1) = 1.25;
  kin_stoichiometry(0, 2) = -1.5;
  kin_stoichiometry(0, 3) = 1.75;
  kin_stoichiometry(1, 1) = -11.0;
  GeochemistryKineticRateCalculator::calculateRate(promoting_indices,
                                                   rate_cal,
                                                   basis_name,
                                                   basis_species_gas,
                                                   basis_molality,
                                                   basis_activity,
                                                   basis_activity_known,
                                                   eqm_name,
                                                   eqm_species_gas,
                                                   eqm_molality,
                                                   eqm_activity,
                                                   eqm_stoichiometry,
                                                   1.25,
                                                   2.25,
                                                   3.5,
                                                   4.0,
                                                   kin_stoichiometry,
                                                   0,
                                                   40.0,
                                                   rate,
                                                   drate_dkin,
                                                   drate_dmol);
  const Real theta_term = std::pow(std::pow(10.0, 4.0 - 3.5), 2.5);
  EXPECT_NEAR(rate,
              -7.0 * 6.0 * std::pow(1.5, 0.5) * std::pow(0.2, 1.0) * std::pow(3.0, 1.5) *
                  std::pow(0.4, 2.0) * std::pow(5.0, 2.5) * std::pow(1.1, -0.6) *
                  std::pow(2.5, -0.7) * std::pow(2.3, -0.8) * std::pow(1.4, -0.9) *
                  std::pow(1.9, -1.0) * std::pow(1.7, -1.1) * std::pow(1.3, -1.2) *
                  std::pow(std::abs(1 - theta_term), 0.8) *
                  std::exp(55.0 / GeochemistryConstants::GAS_CONSTANT *
                           (0.00315 - 1.0 / (40.0 + GeochemistryConstants::CELSIUS_TO_KELVIN))),
              1.0E-6);
  EXPECT_EQ(drate_dkin, 0.0);
  EXPECT_NEAR(drate_dmol[0], 0.5 * rate / 1.5, 1.0E-6);
  EXPECT_NEAR(drate_dmol[1],
              1.0 * rate / 2.0 +
                  0.8 * rate / std::abs(1 - theta_term) * (-1) * (-2.5) * theta_term * 1.25 / 2.0,
              1.0E-6);
  EXPECT_NEAR(drate_dmol[2],
              1.5 * rate / 3.0 +
                  0.8 * rate / std::abs(1 - theta_term) * (-1) * (-2.5) * theta_term * (-1.5 / 3.0),
              1.0E-6);
  EXPECT_EQ(drate_dmol[3], 0.0);
  EXPECT_NEAR(drate_dmol[4], 2.5 * rate / 5.0, 1.0E-6);

  // do a finite-difference of the previous test.
  // Assume activity coefficients = 1
  Real ap = std::pow(0.1, -1.0) * std::pow(basis_activity[1], 1.25) *
            std::pow(basis_molality[2], -1.5) * std::pow(0.4, 1.75);
  GeochemistryKineticRateCalculator::calculateRate(promoting_indices,
                                                   rate_cal,
                                                   basis_name,
                                                   basis_species_gas,
                                                   basis_molality,
                                                   basis_activity,
                                                   basis_activity_known,
                                                   eqm_name,
                                                   eqm_species_gas,
                                                   eqm_molality,
                                                   eqm_activity,
                                                   eqm_stoichiometry,
                                                   1.25,
                                                   2.25,
                                                   -1.5,
                                                   std::log10(ap),
                                                   kin_stoichiometry,
                                                   0,
                                                   40.0,
                                                   rate,
                                                   drate_dkin,
                                                   drate_dmol);
  const Real eps = 1.0E-8;
  basis_molality[2] += eps;
  ap = std::pow(0.1, -1.0) * std::pow(basis_activity[1], 1.25) * std::pow(basis_molality[2], -1.5) *
       std::pow(0.4, 1.75);
  Real rate_new;
  GeochemistryKineticRateCalculator::calculateRate(promoting_indices,
                                                   rate_cal,
                                                   basis_name,
                                                   basis_species_gas,
                                                   basis_molality,
                                                   basis_activity,
                                                   basis_activity_known,
                                                   eqm_name,
                                                   eqm_species_gas,
                                                   eqm_molality,
                                                   eqm_activity,
                                                   eqm_stoichiometry,
                                                   1.25,
                                                   2.25,
                                                   -1.5,
                                                   std::log10(ap),
                                                   kin_stoichiometry,
                                                   0,
                                                   40.0,
                                                   rate_new,
                                                   drate_dkin,
                                                   drate_dmol);
  EXPECT_NEAR(drate_dmol[2], (rate_new - rate) / eps, 1E-5);
}
