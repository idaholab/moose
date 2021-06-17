//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "GeochemistryIonicStrength.h"
#include "GeochemistrySpeciesSwapper.h"

const GeochemicalDatabaseReader database("database/moose_testdb.json");
// The following system has secondary species: CO2(aq), CO3--, CaCO3, CaOH+, OH-, (O-phth)--,
// >(s)FeO-
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
const GeochemistryIonicStrength is_calc(1E9, 1E9, false, false);
const GeochemistryIonicStrength is_basis_only(1E9, 1E9, true, false);
const GeochemistryIonicStrength is_cl_only(1E9, 1E9, false, true);

TEST(GeochemistryIonicStrengthTest, sizeExceptions)
{
  const std::vector<Real> basis_m_good(6);
  const std::vector<Real> eqm_m_good(8);
  const std::vector<Real> kin_m_good(3);

  try
  {
    is_calc.ionicStrength(mgd, {}, eqm_m_good, kin_m_good);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find(
            "Ionic strength calculation: Number of basis species in mgd not equal to the size of "
            "basis_species_molality") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    is_calc.ionicStrength(mgd, basis_m_good, {}, kin_m_good);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Ionic strength calculation: Number of equilibrium species in mgd not "
                         "equal to the size of "
                         "eqm_species_molality") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    is_calc.ionicStrength(mgd, basis_m_good, eqm_m_good, {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Ionic strength calculation: Number of kinetic species in mgd not "
                         "equal to the size of "
                         "kin_species_molality") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    is_calc.stoichiometricIonicStrength(mgd, {}, eqm_m_good, kin_m_good);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Stoichiometric ionic strength calculation: Number of basis species in "
                         "mgd not equal to the size of "
                         "basis_species_molality") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    is_calc.stoichiometricIonicStrength(mgd, basis_m_good, {}, kin_m_good);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find(
            "Stoichiometric ionic strength calculation: Number of equilibrium species in mgd not "
            "equal to the size of "
            "eqm_species_molality") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    is_calc.stoichiometricIonicStrength(mgd, basis_m_good, eqm_m_good, {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("Stoichiometric ionic strength calculation: Number of kinetic species in mgd not "
                 "equal to the size of "
                 "kin_species_molality") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Test getting and setting maximum ionic strengths
TEST(GeochemistryIonicStrengthTest, getsetMax)
{
  GeochemistryIonicStrength is(1.0, 2.0, false, false);
  EXPECT_EQ(is.getMaxIonicStrength(), 1.0);
  EXPECT_EQ(is.getMaxStoichiometricIonicStrength(), 2.0);

  is.setMaxIonicStrength(3.25);
  is.setMaxStoichiometricIonicStrength(4.5);

  EXPECT_EQ(is.getMaxIonicStrength(), 3.25);
  EXPECT_EQ(is.getMaxStoichiometricIonicStrength(), 4.5);
}

/// Test using basis or Cl only
TEST(GeochemistryIonicStrengthTest, getsetOnly)
{
  EXPECT_FALSE(is_calc.getUseOnlyBasisMolality());
  EXPECT_TRUE(is_basis_only.getUseOnlyBasisMolality());
  EXPECT_FALSE(is_cl_only.getUseOnlyBasisMolality());
  EXPECT_FALSE(is_calc.getUseOnlyClMolality());
  EXPECT_FALSE(is_basis_only.getUseOnlyClMolality());
  EXPECT_TRUE(is_cl_only.getUseOnlyClMolality());

  GeochemistryIonicStrength is_tmp(0.125, 1E9, false, false);
  EXPECT_FALSE(is_tmp.getUseOnlyBasisMolality());
  EXPECT_FALSE(is_tmp.getUseOnlyClMolality());
  is_tmp.setUseOnlyBasisMolality(true);
  EXPECT_TRUE(is_tmp.getUseOnlyBasisMolality());
  EXPECT_FALSE(is_tmp.getUseOnlyClMolality());
  is_tmp.setUseOnlyClMolality(true);
  EXPECT_TRUE(is_tmp.getUseOnlyBasisMolality());
  EXPECT_TRUE(is_tmp.getUseOnlyClMolality());
  is_tmp.setUseOnlyBasisMolality(false);
  EXPECT_FALSE(is_tmp.getUseOnlyBasisMolality());
  EXPECT_TRUE(is_tmp.getUseOnlyClMolality());
}

/// Test computing ionic strength
TEST(GeochemistryIonicStrengthTest, ionicStrength)
{
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

  const Real gold_basis_only = 0.5 * gold_ionic_str;

  std::vector<Real> eqm_m(8);
  eqm_m[mgd.eqm_species_index.at("CO2(aq)")] = 1.1;
  eqm_m[mgd.eqm_species_index.at("CO3--")] = 2.2;
  gold_ionic_str += 4 * 2.2;
  eqm_m[mgd.eqm_species_index.at("CaCO3")] = 3.3;
  eqm_m[mgd.eqm_species_index.at("CaOH+")] = 4.4;
  gold_ionic_str += 4.4;
  eqm_m[mgd.eqm_species_index.at("OH-")] = 5.5;
  gold_ionic_str += 5.5;
  eqm_m[mgd.eqm_species_index.at("(O-phth)--")] = 6.6;
  gold_ionic_str += 4 * 6.6;
  eqm_m[mgd.eqm_species_index.at(">(s)FeO-")] = 7.7;
  gold_ionic_str += 7.7;
  eqm_m[mgd.eqm_species_index.at("Calcite")] = 9.9;

  std::vector<Real> kin_m(3);
  kin_m[mgd.kin_species_index.at("Calcite_asdf")] = -1.1;
  kin_m[mgd.kin_species_index.at("CH4(aq)")] = -2.2;
  kin_m[mgd.kin_species_index.at(">(s)FeOCa+")] = -3.3;
  gold_ionic_str += -3.3;

  gold_ionic_str *= 0.5;

  const Real ionic_str = is_calc.ionicStrength(mgd, basis_m, eqm_m, kin_m);
  const Real ionic_str_basis_only = is_basis_only.ionicStrength(mgd, basis_m, eqm_m, kin_m);

  EXPECT_NEAR(ionic_str, gold_ionic_str, 1E-9);
  EXPECT_NEAR(ionic_str_basis_only, gold_basis_only, 1E-9);

  GeochemistryIonicStrength is_max(0.125, 1E9, false, false);
  EXPECT_EQ(is_max.ionicStrength(mgd, basis_m, eqm_m, kin_m), 0.125);
}

/// Test computing stoichiometric ionic strength
TEST(GeochemistryIonicStrengthTest, stoichiometricIonicStrength)
{
  Real gold_ionic_str = 0.0;

  std::vector<Real> basis_m(6);
  basis_m[mgd.basis_species_index.at("H2O")] = 1.0;
  basis_m[mgd.basis_species_index.at("H+")] = 2.0;
  gold_ionic_str += 2.0;
  basis_m[mgd.basis_species_index.at("HCO3-")] = 3.0;
  gold_ionic_str += 3.0;
  basis_m[mgd.basis_species_index.at("O2(aq)")] = 4.0;
  basis_m[mgd.basis_species_index.at("Ca++")] = 5.0;
  gold_ionic_str += 5.0 * 4;
  basis_m[mgd.basis_species_index.at(">(s)FeOH")] = 6.0;

  const Real gold_basis_only = 0.5 * gold_ionic_str;

  std::vector<Real> eqm_m(8);
  eqm_m[mgd.eqm_species_index.at("CO2(aq)")] = 1.1;
  gold_ionic_str += 1.1 * (1 + 1);
  eqm_m[mgd.eqm_species_index.at("CO3--")] = 2.2;
  gold_ionic_str += 2.2 * 4;
  eqm_m[mgd.eqm_species_index.at("CaCO3")] = 3.3;
  gold_ionic_str += 3.3 * (4 + 1 - 1);
  eqm_m[mgd.eqm_species_index.at("CaOH+")] = 4.4;
  gold_ionic_str += 4.4 * (1);
  eqm_m[mgd.eqm_species_index.at("OH-")] = 5.5;
  gold_ionic_str += 5.5 * (1);
  eqm_m[mgd.eqm_species_index.at("(O-phth)--")] = 6.6;
  gold_ionic_str += 6.6 * (4);
  eqm_m[mgd.eqm_species_index.at(">(s)FeO-")] = 7.7;
  gold_ionic_str += 7.7 * (1);
  eqm_m[mgd.eqm_species_index.at("Calcite")] = 9.9;
  gold_ionic_str += 9.9 * (4 + 1 - 1);

  std::vector<Real> kin_m(3);
  kin_m[mgd.kin_species_index.at("Calcite_asdf")] = -1.1;
  gold_ionic_str += 0.0;
  kin_m[mgd.kin_species_index.at("CH4(aq)")] = -2.2;
  gold_ionic_str += 0.0;
  kin_m[mgd.kin_species_index.at(">(s)FeOCa+")] = -3.3;
  gold_ionic_str += -3.3 * (1);

  gold_ionic_str *= 0.5;

  const Real ionic_str = is_calc.stoichiometricIonicStrength(mgd, basis_m, eqm_m, kin_m);
  const Real ionic_str_basis_only =
      is_basis_only.stoichiometricIonicStrength(mgd, basis_m, eqm_m, kin_m);

  EXPECT_NEAR(ionic_str, gold_ionic_str, 1E-9);
  EXPECT_NEAR(ionic_str_basis_only, gold_basis_only, 1E-9);

  GeochemistryIonicStrength is_max(1E9, 0.125, false, false);
  EXPECT_EQ(is_max.stoichiometricIonicStrength(mgd, basis_m, eqm_m, kin_m), 0.125);
}

/// Test exception stoichiometric ionic strength when using Cl only
TEST(GeochemistryIonicStrengthTest, stoichiometricIonicStrength_except)
{
  std::vector<Real> basis_m(6);
  std::vector<Real> eqm_m(8);
  std::vector<Real> kin_m(3);
  try
  {
    is_cl_only.stoichiometricIonicStrength(mgd, basis_m, eqm_m, kin_m);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find(
            "GeochemistryIonicStrength: attempting to compute stoichiometric ionic strength "
            "using only the Cl- molality, but Cl- does not appear in the geochemical system") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Test stoichiometric ionic strength when using Cl only
TEST(GeochemistryIonicStrengthTest, stoichiometricIonicStrengthOnlyCl)
{
  const GeochemicalDatabaseReader db_cl("../database/moose_geochemdb.json", true, true, false);

  // Cl is a basis species
  const PertinentGeochemicalSystem model_cl(
      db_cl, {"H2O", "H+", "Cl-"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  const ModelGeochemicalDatabase mgd_cl = model_cl.modelGeochemicalDatabase();
  const GeochemistryIonicStrength is_cl(1E9, 1E9, false, true);
  const std::vector<Real> basis_m{0.75, 2.5, 3.5};
  const std::vector<Real> eqm_m{1.5, 0.25};
  EXPECT_EQ(is_cl.stoichiometricIonicStrength(mgd_cl, basis_m, eqm_m, {}), 3.5);

  // Cl is an equilibrium species
  const PertinentGeochemicalSystem model_cl_eqm(
      db_cl, {"H2O", "H+", "Cl-"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd_cl_eqm = model_cl_eqm.modelGeochemicalDatabase();
  GeochemistrySpeciesSwapper swapper(3, 1E-6);
  swapper.performSwap(mgd_cl_eqm, "Cl-", "HCl");
  const GeochemistryIonicStrength is_cl_eqm(1E9, 1E9, false, true);
  const std::vector<Real> basis_m_eqm{0.75, 2.5, 3.5};
  const std::vector<Real> eqm_m_eqm{1.5, 0.25};
  const unsigned pos = mgd_cl_eqm.eqm_species_name[0] == "Cl-" ? 0 : 1;
  EXPECT_EQ(is_cl_eqm.stoichiometricIonicStrength(mgd_cl_eqm, basis_m_eqm, eqm_m_eqm, {}),
            eqm_m_eqm[pos]);

  // Cl is a kinetic species
  const GeochemicalDatabaseReader db_cl_kin(
      "database/moose_testdb_cl_kinetic.json", true, true, false);
  const PertinentGeochemicalSystem model_cl_kin(
      db_cl_kin, {"H2O"}, {}, {}, {}, {"Cl-"}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd_cl_kin = model_cl_kin.modelGeochemicalDatabase();
  const GeochemistryIonicStrength is_cl_kin(1E9, 1E9, false, true);
  const std::vector<Real> basis_m_kin{0.75};
  const std::vector<Real> kin_m_kin{7.75};
  EXPECT_EQ(is_cl_kin.stoichiometricIonicStrength(mgd_cl_kin, basis_m_kin, {}, kin_m_kin), 7.75);
}
