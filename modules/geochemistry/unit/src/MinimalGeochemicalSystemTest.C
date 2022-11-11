//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "PertinentGeochemicalSystem.h"

const Real eps = 1E-12; // accounts for precision loss when substituting reactions

/**
 * Test that:
 * - all elements in the basis_species list are basis aqueous species or redox species in the
 * database file
 * - no element appears more than once in the basis_species list
 * - H2O appears first in the basis_species list
 */
TEST(PertinentGeochemicalSystemTest, basisExceptions)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  try
  {
    PertinentGeochemicalSystem model(
        database, {"H2O", "Ca++", "(O-phth)--", "H2O", "Na+"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("H2O exists more than once in the basis species list") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    PertinentGeochemicalSystem model(database,
                                     {"Ca++", "H2O", "H+", "(O-phth)--", "Am++++"},
                                     {},
                                     {},
                                     {},
                                     {},
                                     {},
                                     "O2(aq)",
                                     "e-");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("First member of basis species list must be H2O") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    PertinentGeochemicalSystem model(
        database,
        {"H2O", "Ca++", "H+", "(O-phth)--", "Am++++", "does_not_exist"},
        {},
        {},
        {},
        {},
        {},
        "O2(aq)",
        "e-");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("does_not_exist does not exist in the basis species or redox species in "
                         "database/moose_testdb.json") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/**
 * Test that:
 * - all elements in the minerals list are minerals in the database file
 * - no element appears more than once in this list
 * - the equilibrium reaction of each of these contains only basis species in the basis_species
 * list, or secondary species (including redox couples) whose reactions contain only basis_species
 * - if a mineral in this list is also a "sorbing mineral" in the database file, its sorbing
 * sites must be present in the basis_species list
 */
TEST(PertinentGeochemicalSystemTest, mineralExceptions)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  try
  {
    PertinentGeochemicalSystem model(database,
                                     {"H2O", "Ca++", "H+", "(O-phth)--", "HCO3-"},
                                     {"Calcite", "Calcite_asdf", "Calcite"},
                                     {},
                                     {},
                                     {},
                                     {},
                                     "O2(aq)",
                                     "e-");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Calcite exists more than once in the minerals list") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    PertinentGeochemicalSystem model(database,
                                     {"H2O", "Ca++", "H+", "Am++++", "HCO3-"},
                                     {"Calcite", "Calcite_asdf", "does_not_exist"},
                                     {},
                                     {},
                                     {},
                                     {},
                                     "O2(aq)",
                                     "e-");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("does_not_exist does not exist in database database/moose_testdb.json") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    PertinentGeochemicalSystem model(database,
                                     {"H2O", "H+", "(O-phth)--", "HCO3-"},
                                     {"Calcite"},
                                     {},
                                     {},
                                     {},
                                     {},
                                     "O2(aq)",
                                     "e-");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("The reaction for Calcite depends on Ca++ which is not reducable to a set "
                         "of basis species") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    PertinentGeochemicalSystem model(
        database, {"H2O", "H+", "O2(aq)"}, {"Fe(OH)3(ppd)"}, {}, {}, {}, {}, "O2(aq)", "e-");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("The reaction for Fe(OH)3(ppd) depends on Fe+++ which is not reducable to "
                         "a set of basis species") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    PertinentGeochemicalSystem model(database,
                                     {"H2O", "H+", "O2(aq)", "Fe++", ">(s)FeOH"},
                                     {"Fe(OH)3(ppd)"},
                                     {},
                                     {},
                                     {},
                                     {},
                                     "O2(aq)",
                                     "e-");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("The sorbing sites for Fe(OH)3(ppd) include >(w)FeOH which is not in the "
                         "basis_species list") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  // This should pass since the basis species can make Fe+++, which then makes Fe(OH)3(ppd), and the
  // surface sites for Fe(OH)3(ppd) are included
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", "O2(aq)", "Fe++", ">(s)FeOH", ">(w)FeOH"},
                                   {"Fe(OH)3(ppd)"},
                                   {},
                                   {},
                                   {},
                                   {},
                                   "O2(aq)",
                                   "e-");
}

/**
 * Test that:
 * - all elements in the gases list are gases in the database file
 * - no element appears more than once in this list
 * - the equilibrium reaction of each of these contains only species that are reducable to basis
 * species
 */
TEST(PertinentGeochemicalSystemTest, gasExceptions)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  try
  {
    PertinentGeochemicalSystem model(database,
                                     {"H2O", "Ca++", "H+", "HCO3-", "CH4(aq)"},
                                     {"Calcite"},
                                     {"CH4(g)", "CH4(g)"},
                                     {},
                                     {},
                                     {},
                                     "O2(aq)",
                                     "e-");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("CH4(g) exists more than once in the gases list") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    PertinentGeochemicalSystem model(database,
                                     {"H2O", "Ca++", "H+", "HCO3-", "CH4(aq)"},
                                     {"Calcite"},
                                     {"CH4(g)", "does_not_exist"},
                                     {},
                                     {},
                                     {},
                                     "O2(aq)",
                                     "e-");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("does_not_exist does not exist in database database/moose_testdb.json") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    PertinentGeochemicalSystem model(database,
                                     {"H2O", "Ca++", "H+", "HCO3-"},
                                     {"Calcite"},
                                     {"CH4(g)"},
                                     {},
                                     {},
                                     {},
                                     "O2(aq)",
                                     "e-");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("The reaction for CH4(g) depends on CH4(aq) which is not reducable to a "
                         "set of basis species") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  // this should pass: CH4(g) depends on the redox couple CH4(aq) which in turn depends on the basis
  // species provided
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "Ca++", "H+", "HCO3-", "O2(aq)"},
                                   {"Calcite"},
                                   {"CH4(g)"},
                                   {},
                                   {},
                                   {},
                                   "O2(aq)",
                                   "e-");
}

/**
 * Test that:
 * - all elements in the kinetic_minerals list are minerals in the database file
 * - no element appears more than once in this list
 * - no member of the kinetic_minerals list can also appear in the minerals list
 * - the equilibrium reaction of each of these contains only species that are reducable to a set of
 * basis species
 * - if a mineral in this list is also a "sorbing mineral" in the database file, its sorbing sites
 * must be present in the basis_species list
 */
TEST(PertinentGeochemicalSystemTest, kineticMineralExceptions)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  try
  {
    PertinentGeochemicalSystem model(
        database, {"H2O"}, {}, {}, {"Calcite", "Calcite_asdf", "Calcite"}, {}, {}, "O2(aq)", "e-");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Calcite exists more than once in the kinetic_minerals list") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    PertinentGeochemicalSystem model(database,
                                     {"H2O", "Ca++", "HCO3-", "H+"},
                                     {"Calcite_asdf"},
                                     {},
                                     {"Calcite", "Calcite_asdf"},
                                     {},
                                     {},
                                     "O2(aq)",
                                     "e-");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Calcite_asdf exists in both the minerals and kinetic_minerals lists") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    PertinentGeochemicalSystem model(database,
                                     {"H2O", "Ca++", "HCO3-", "H+"},
                                     {"Calcite_asdf"},
                                     {},
                                     {"Calcite", "does_not_exist"},
                                     {},
                                     {},
                                     "O2(aq)",
                                     "e-");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("does_not_exist does not exist in database database/moose_testdb.json") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    PertinentGeochemicalSystem model(
        database, {"H2O", "O2(aq)", "H+"}, {}, {}, {"Fe(OH)3(ppd)"}, {}, {}, "O2(aq)", "e-");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("The reaction for Fe(OH)3(ppd) depends on Fe+++ which is not reducable to "
                         "a set of basis species") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    PertinentGeochemicalSystem model(database,
                                     {"H2O", "H+", "O2(aq)", "Fe++", ">(s)FeOH"},
                                     {},
                                     {},
                                     {"Fe(OH)3(ppd)"},
                                     {},
                                     {},
                                     "O2(aq)",
                                     "e-");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("The sorbing sites for Fe(OH)3(ppd) include >(w)FeOH which is not in the "
                         "basis_species list") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  // This should pass since the basis species can make Fe+++, which then makes Fe(OH)3(ppd), and the
  // surface sites for Fe(OH)3(ppd) are included
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", "O2(aq)", "Fe++", ">(s)FeOH", ">(w)FeOH"},
                                   {},
                                   {},
                                   {"Fe(OH)3(ppd)"},
                                   {},
                                   {},
                                   "O2(aq)",
                                   "e-");
}

/**
 * Test that:
 * - all elements in the kinetic_redox list are "redox couples" in the database file
 * - no element appears more than once in this list
 * - no member of the kinetic_redox list can also appear in the basis_species list
 * - the equilibrium reaction of each of these contains only species that are reducable to basis
 * species
 */
TEST(PertinentGeochemicalSystemTest, kineticRedoxExceptions)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  try
  {
    PertinentGeochemicalSystem model(
        database, {"H2O"}, {}, {}, {}, {"(O-phth)--", "Am++++", "(O-phth)--"}, {}, "O2(aq)", "e-");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("(O-phth)-- exists more than once in the kinetic_redox list") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    PertinentGeochemicalSystem model(
        database, {"H2O", "(O-phth)--"}, {}, {}, {}, {"Am++++", "(O-phth)--"}, {}, "O2(aq)", "e-");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("(O-phth)-- exists in both the basis_species and kinetic_redox lists") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    PertinentGeochemicalSystem model(
        database, {"H2O"}, {}, {}, {}, {"(O-phth)--", "does_not_exist"}, {}, "O2(aq)", "e-");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("does_not_exist does not exist in database database/moose_testdb.json") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    PertinentGeochemicalSystem model(
        database, {"H2O", "HCO3-", "H+"}, {}, {}, {}, {"(O-phth)--"}, {}, "O2(aq)", "e-");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("The reaction for (O-phth)-- depends on O2(aq) which is not reducable to "
                         "a set of basis species") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  // this should pass
  PertinentGeochemicalSystem model(
      database, {"H2O", "HCO3-", "H+", "O2(aq)"}, {}, {}, {}, {"(O-phth)--"}, {}, "O2(aq)", "e-");
}

/**
 * Test that:
 * - all elements in the kinetic_surface_species list are "surface species" in the database file
 * - no element appears more than once in this list
 * - the equilibrium reaction of each of these contains only species that are reducable to basis
 * species
 */
TEST(PertinentGeochemicalSystemTest, kineticSurfaceExceptions)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  try
  {
    PertinentGeochemicalSystem model(
        database, {"H2O"}, {}, {}, {}, {}, {">(s)FeO-", ">(s)FeOCa+", ">(s)FeO-"}, "O2(aq)", "e-");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find(">(s)FeO- exists more than once in the kinetic_surface_species list") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    PertinentGeochemicalSystem model(database,
                                     {"H2O"},
                                     {},
                                     {},
                                     {},
                                     {},
                                     {">(s)FeO-", ">(s)FeOCa+", "does_not_exist"},
                                     "O2(aq)",
                                     "e-");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("does_not_exist does not exist in database database/moose_testdb.json") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    PertinentGeochemicalSystem model(
        database, {"H2O", "H+"}, {}, {}, {}, {}, {">(s)FeO-"}, "O2(aq)", "e-");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("The reaction for >(s)FeO- depends on >(s)FeOH which is not reducable to "
                         "a set of basis species") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  // this should pass
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", ">(s)FeOH"}, {}, {}, {}, {}, {">(s)FeO-"}, "O2(aq)", "e-");
}

/// Test that PertinentGeochemicalSystem correctly extracts temperatures from the GeochemicalDatabaseReader
TEST(PertinentGeochemicalSystemTest, temperatures)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  PertinentGeochemicalSystem model(database, {"H2O"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  ASSERT_EQ(mgd.original_database->getTemperatures(), database.getTemperatures());
}

/**
 * Test that the names of basis and equilibrium species are correctly recorded by
 * PertinentGeochemicalSystem This mostly tests that given a set of basis species, the set of
 * equilibrium and kinetic species is correct
 */
TEST(PertinentGeochemicalSystemTest, names1)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, CaCO3, CaOH+, OH-, (O-phth)--,
  // >(s)FeO-
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++", ">(s)FeOH"},
                                   {"Calcite"},
                                   {"O2(g)"},
                                   {"Calcite_asdf"},
                                   {"CH4(aq)"},
                                   {">(s)FeOCa+"},
                                   "O2(aq)",
                                   "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  ASSERT_EQ(mgd.basis_species_index.size(), (std::size_t)6);
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_name[sp.second], sp.first);

  ASSERT_EQ(mgd.eqm_species_index.size(), (std::size_t)9);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_name[sp.second], sp.first);

  ASSERT_EQ(mgd.kin_species_index.size(), (std::size_t)3);
  for (const auto & sp : mgd.kin_species_index)
    ASSERT_EQ(mgd.kin_species_name[sp.second], sp.first);
}

/**
 * Another test that the names of basis and equilibrium species are correctly recorded by
 * PertinentGeochemicalSystem This mostly tests that given a set of basis species, the set of
 * equilibrium and kinetic species is correct
 */
TEST(PertinentGeochemicalSystemTest, names2)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, CaCO3, CaOH+, OH-, (O-phth)--
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++", ">(s)FeOH"},
                                   {},
                                   {"O2(g)"},
                                   {"Calcite_asdf", "Calcite"},
                                   {"CH4(aq)"},
                                   {">(s)FeOCa+", ">(s)FeO-"},
                                   "O2(aq)",
                                   "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  ASSERT_EQ(mgd.basis_species_index.size(), (std::size_t)6);
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_name[sp.second], sp.first);

  ASSERT_EQ(mgd.eqm_species_index.size(), (std::size_t)7);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_name[sp.second], sp.first);

  ASSERT_EQ(mgd.kin_species_index.size(), (std::size_t)5);
  for (const auto & sp : mgd.kin_species_index)
    ASSERT_EQ(mgd.kin_species_name[sp.second], sp.first);
}

/// Test that the charge of species is correctly recorded
TEST(PertinentGeochemicalSystemTest, charge)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, CaCO3, CaOH+, OH-, (O-phth)--,
  // >(s)FeO-, e-
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++", ">(s)FeOH"},
                                   {"Calcite"},
                                   {},
                                   {"Calcite_asdf"},
                                   {"CH4(aq)"},
                                   {">(s)FeOCa+"},
                                   "O2(aq)",
                                   "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::map<std::string, Real> charge_gold;
  charge_gold["H2O"] = 0.0;
  charge_gold["H+"] = 1.0;
  charge_gold["HCO3-"] = -1.0;
  charge_gold["O2(aq)"] = 0.0;
  charge_gold["Ca++"] = 2.0;
  charge_gold[">(s)FeOH"] = 0.0;
  charge_gold["CO2(aq)"] = 0.0;
  charge_gold["CO3--"] = -2.0;
  charge_gold["CaCO3"] = 0.0;
  charge_gold["CaOH+"] = 1.0;
  charge_gold["OH-"] = -1.0;
  charge_gold["(O-phth)--"] = -2.0;
  charge_gold[">(s)FeO-"] = -1.0;
  charge_gold[">(s)FeOCa+"] = -1.0;
  charge_gold["Calcite"] = 0.0;
  charge_gold["e-"] = -1.0;
  charge_gold["Calcite_asdf"] = 0.0;
  charge_gold["CH4(aq)"] = 0.0;
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_charge[sp.second], charge_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_charge[sp.second], charge_gold[sp.first]);
  for (const auto & sp : mgd.kin_species_index)
    ASSERT_EQ(mgd.kin_species_charge[sp.second], charge_gold[sp.first]);
}

/// Test that the radius of each species is correctly recorded
TEST(PertinentGeochemicalSystemTest, radius)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, CaCO3, CaOH+, OH-, (O-phth)--,
  // >(s)FeO-, >(s)FeOCa+, e-
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++", ">(s)FeOH"},
                                   {"Calcite"},
                                   {},
                                   {},
                                   {"CH4(aq)"},
                                   {},
                                   "O2(aq)",
                                   "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::map<std::string, Real> radius_gold;
  radius_gold["H2O"] = 0.0;
  radius_gold["H+"] = 9.0;
  radius_gold["HCO3-"] = 4.5;
  radius_gold["O2(aq)"] = -0.5;
  radius_gold["Ca++"] = 6.0;
  radius_gold[">(s)FeOH"] = 0.0;
  radius_gold["CO2(aq)"] = 4.0;
  radius_gold["CO3--"] = 4.5;
  radius_gold["CaCO3"] = 4.0;
  radius_gold["CaOH+"] = 4.0;
  radius_gold["OH-"] = 3.5;
  radius_gold["(O-phth)--"] = 4.0;
  radius_gold[">(s)FeO-"] = -1.5;
  radius_gold[">(s)FeOCa+"] = -1.5;
  radius_gold["Calcite"] = 0.0;
  radius_gold["e-"] = 0.0;
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_radius[sp.second], radius_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_radius[sp.second], radius_gold[sp.first]);
}

/// Test that the molecular weight of each species is correctly recorded
TEST(PertinentGeochemicalSystemTest, molecular_weight)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, CaCO3, CaOH+, OH-, (O-phth)--,
  // >(s)FeO-, >(s)FeOCa+, e-
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++", ">(s)FeOH"},
                                   {"Calcite"},
                                   {},
                                   {"Calcite_asdf"},
                                   {"CH4(aq)"},
                                   {">(s)FeOCa+"},
                                   "O2(aq)",
                                   "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::map<std::string, Real> molecular_weight_gold;
  molecular_weight_gold["H2O"] = 18.0152;
  molecular_weight_gold["H+"] = 1.0079;
  molecular_weight_gold["HCO3-"] = 61.0171;
  molecular_weight_gold["O2(aq)"] = 31.9988;
  molecular_weight_gold["Ca++"] = 40.0800;
  molecular_weight_gold[">(s)FeOH"] = 72.8543;
  molecular_weight_gold["CO2(aq)"] = 44.0098;
  molecular_weight_gold["CO3--"] = 60.0092;
  molecular_weight_gold["CaCO3"] = 100.0892;
  molecular_weight_gold["CaOH+"] = 57.0873;
  molecular_weight_gold["OH-"] = 17.0073;
  molecular_weight_gold["(O-phth)--"] = 164.1172;
  molecular_weight_gold[">(s)FeO-"] = 71.8464;
  molecular_weight_gold[">(s)FeOCa+"] = 279.7144;
  molecular_weight_gold["Calcite"] = 100.0892;
  molecular_weight_gold["e-"] = 0.0;
  molecular_weight_gold["Calcite_asdf"] = 111.0892;
  molecular_weight_gold["CH4(aq)"] = 16.0426;
  molecular_weight_gold[">(s)FeOCa+"] = 279.7144;
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_molecular_weight[sp.second], molecular_weight_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_molecular_weight[sp.second], molecular_weight_gold[sp.first]);
  for (const auto & sp : mgd.kin_species_index)
    ASSERT_EQ(mgd.kin_species_molecular_weight[sp.second], molecular_weight_gold[sp.first]);
}

/// Test that the molecular volume of each species is correctly recorded
TEST(PertinentGeochemicalSystemTest, molecular_volume)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, CaCO3, CaOH+, OH-, (O-phth)--,
  // >(s)FeO-, >(s)FeOCa+, e-
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++", ">(s)FeOH"},
                                   {"Calcite"},
                                   {},
                                   {"Calcite_asdf"},
                                   {"CH4(aq)"},
                                   {">(s)FeOCa+"},
                                   "O2(aq)",
                                   "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::map<std::string, Real> molecular_volume_gold;
  molecular_volume_gold["H2O"] = 0.0;
  molecular_volume_gold["H+"] = 0.0;
  molecular_volume_gold["HCO3-"] = 0.0;
  molecular_volume_gold["O2(aq)"] = 0.0;
  molecular_volume_gold["Ca++"] = 0.0;
  molecular_volume_gold[">(s)FeOH"] = 0.0;
  molecular_volume_gold["CO2(aq)"] = 0.0;
  molecular_volume_gold["CO3--"] = 0.0;
  molecular_volume_gold["CaCO3"] = 0.0;
  molecular_volume_gold["CaOH+"] = 0.0;
  molecular_volume_gold["OH-"] = 0.0;
  molecular_volume_gold["(O-phth)--"] = 0.0;
  molecular_volume_gold[">(s)FeO-"] = 0.0;
  molecular_volume_gold[">(s)FeOCa+"] = 0.0;
  molecular_volume_gold["Calcite"] = 36.9340;
  molecular_volume_gold["e-"] = 0.0;
  molecular_volume_gold["Calcite_asdf"] = 136.9340;
  molecular_volume_gold["CH4(aq)"] = 0.0;
  molecular_volume_gold[">(s)FeOCa+"] = 0.0;
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_molecular_volume[sp.second], molecular_volume_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_molecular_volume[sp.second], molecular_volume_gold[sp.first]);
  for (const auto & sp : mgd.kin_species_index)
    ASSERT_EQ(mgd.kin_species_molecular_volume[sp.second], molecular_volume_gold[sp.first]);
}

/// Test that the surface complexation information is correctly recorded
TEST(PertinentGeochemicalSystemTest, surfaceComplexationInfo)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, CaCO3, CaOH+, OH-, (O-phth)--,
  // >(s)FeO-, >(s)FeOCa+, e-
  PertinentGeochemicalSystem model1(database,
                                    {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++", ">(s)FeOH"},
                                    {"Calcite"},
                                    {},
                                    {},
                                    {"CH4(aq)"},
                                    {},
                                    "O2(aq)",
                                    "e-");
  ModelGeochemicalDatabase mgd1 = model1.modelGeochemicalDatabase();

  ASSERT_EQ(mgd1.surface_complexation_info.size(), (std::size_t)0);

  // The following system has secondary species: CO2(aq), CO3--, CaCO3, CaOH+, OH-, (O-phth)--,
  // >(s)FeO-, >(s)FeOCa+, e-
  PertinentGeochemicalSystem model2(database,
                                    {"H2O", "H+", "O2(aq)", "Fe++", ">(s)FeOH", ">(w)FeOH"},
                                    {"Fe(OH)3(ppd)"},
                                    {},
                                    {},
                                    {},
                                    {},
                                    "O2(aq)",
                                    "e-");
  ModelGeochemicalDatabase mgd2 = model2.modelGeochemicalDatabase();

  ASSERT_EQ(mgd2.surface_complexation_info.count("Fe(OH)3(ppd)"), (std::size_t)1);
  ASSERT_EQ(mgd2.surface_complexation_info["Fe(OH)3(ppd)"].surface_area, 600.0);
  ASSERT_EQ(mgd2.surface_complexation_info["Fe(OH)3(ppd)"].sorption_sites[">(s)FeOH"], 0.005);
  ASSERT_EQ(mgd2.surface_complexation_info["Fe(OH)3(ppd)"].sorption_sites[">(w)FeOH"], 0.2);

  // The following system has secondary species: CO2(aq), CO3--, CaCO3, CaOH+, OH-, (O-phth)--,
  // >(s)FeO-, >(s)FeOCa+, e-
  PertinentGeochemicalSystem model3(database,
                                    {"H2O", "H+", "O2(aq)", "Fe++", ">(s)FeOH", ">(w)FeOH"},
                                    {},
                                    {},
                                    {"Goethite"},
                                    {},
                                    {},
                                    "O2(aq)",
                                    "e-");
  ModelGeochemicalDatabase mgd3 = model3.modelGeochemicalDatabase();

  ASSERT_EQ(mgd3.surface_complexation_info.count("Goethite"), (std::size_t)1);
  ASSERT_EQ(mgd3.surface_complexation_info["Goethite"].surface_area, 60.0);
  ASSERT_EQ(mgd3.surface_complexation_info["Goethite"].sorption_sites[">(s)FeOH"], 0.05);
  ASSERT_EQ(mgd3.surface_complexation_info["Goethite"].sorption_sites[">(w)FeOH"], 0.222);
}

/// Test exception when a sorption site is involved in more than one mineral
TEST(PertinentGeochemicalSystemTest, surfaceComplexationRepeatedException)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  try
  {
    PertinentGeochemicalSystem model3(database,
                                      {"H2O", "H+", "O2(aq)", "Fe++", ">(s)FeOH", ">(w)FeOH"},
                                      {"Goethite", "Fe(OH)3(ppd)"},
                                      {},
                                      {},
                                      {},
                                      {},
                                      "O2(aq)",
                                      "e-");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("The sorbing site >(s)FeOH appears in more than one sorbing mineral") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Test exception when an equilibrium species has a reaction involving more than one sorbing site
TEST(PertinentGeochemicalSystemTest, excessSorbingSitesException)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  try
  {
    PertinentGeochemicalSystem model2(database,
                                      {"H2O", "H+", "Fe+++", "sorbsite1", "sorbsite2"},
                                      {"problematic_sorber"},
                                      {},
                                      {},
                                      {},
                                      {},
                                      "O2(aq)",
                                      "e-");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("It is an error for any equilibrium species (such as problem_eqm) to have "
                         "a reaction involving more than one sorbing site") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Test information related to surface sorption is correctly built
TEST(PertinentGeochemicalSystemTest, surfaceSorptionBuilding)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  PertinentGeochemicalSystem modelC(
      database, {"H2O", "H+", "Ca++", "HCO3-"}, {"Calcite"}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgdC = modelC.modelGeochemicalDatabase();

  EXPECT_EQ(mgdC.surface_sorption_name.size(), (std::size_t)0);
  EXPECT_EQ(mgdC.surface_sorption_area.size(), (std::size_t)0);
  for (unsigned j = 0; j < mgdC.eqm_species_name.size(); ++j)
    EXPECT_EQ(mgdC.surface_sorption_related[j], false);

  PertinentGeochemicalSystem model2(database,
                                    {"H2O", "H+", "Fe+++", ">(s)FeOH", ">(w)FeOH"},
                                    {"Fe(OH)3(ppd)fake", "Goethite"},
                                    {},
                                    {},
                                    {},
                                    {},
                                    "O2(aq)",
                                    "e-");
  ModelGeochemicalDatabase mgd2 = model2.modelGeochemicalDatabase();
  EXPECT_EQ(mgd2.surface_complexation_info.count("Goethite"), (std::size_t)1);
  EXPECT_EQ(mgd2.surface_sorption_name.size(), (std::size_t)1);
  EXPECT_EQ(mgd2.surface_sorption_name[0], "Goethite");
  EXPECT_EQ(mgd2.surface_sorption_area.size(), (std::size_t)1);
  EXPECT_EQ(mgd2.surface_sorption_area[0], 60.0);
  EXPECT_EQ(mgd2.surface_sorption_related.size(), mgd2.eqm_species_name.size());
  EXPECT_EQ(mgd2.surface_sorption_number.size(), mgd2.eqm_species_name.size());
  // Eqm species are: OH- >(s)FeO- Fe(OH)3(ppd)fake Goethite
  EXPECT_EQ(mgd2.eqm_species_index.count(">(s)FeO-"), (std::size_t)1);
  const unsigned posn = mgd2.eqm_species_index.at(">(s)FeO-");
  for (unsigned j = 0; j < mgd2.eqm_species_name.size(); ++j)
    if (j == posn)
      EXPECT_TRUE(mgd2.surface_sorption_related[j]);
    else
      EXPECT_FALSE(mgd2.surface_sorption_related[j]);
  EXPECT_EQ(mgd2.surface_sorption_number[posn], (unsigned int)0);
}

/// Test that the fugacity coefficients are correctly recorded
TEST(PertinentGeochemicalSystemTest, GasChi)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, CaCO3, CaOH+, OH-, (O-phth)--,
  // >(s)FeO-, >(s)FeOCa+, e-
  PertinentGeochemicalSystem model1(database,
                                    {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++", ">(s)FeOH"},
                                    {"Calcite"},
                                    {},
                                    {},
                                    {"CH4(aq)"},
                                    {},
                                    "O2(aq)",
                                    "e-");
  ModelGeochemicalDatabase mgd1 = model1.modelGeochemicalDatabase();

  ASSERT_EQ(mgd1.gas_chi.size(), (std::size_t)0);

  PertinentGeochemicalSystem model2(database,
                                    {"H2O", "Ca++", "H+", "HCO3-", "O2(aq)"},
                                    {"Calcite"},
                                    {"CH4(g)"},
                                    {},
                                    {},
                                    {},
                                    "O2(aq)",
                                    "e-");
  ModelGeochemicalDatabase mgd2 = model2.modelGeochemicalDatabase();

  ASSERT_EQ(mgd2.gas_chi.count("CH4(g)"), (std::size_t)1);
  ASSERT_EQ(mgd2.gas_chi["CH4(g)"][0], -537.779);
  ASSERT_EQ(mgd2.gas_chi["CH4(g)"][1], 1.54946);
  ASSERT_EQ(mgd2.gas_chi["CH4(g)"][2], -.000927827);
  ASSERT_EQ(mgd2.gas_chi["CH4(g)"][3], 1.20861);
  ASSERT_EQ(mgd2.gas_chi["CH4(g)"][4], -.00370814);
  ASSERT_EQ(mgd2.gas_chi["CH4(g)"][5], 3.33804e-6);
}

/**
 * Test that the stoichiometric coefficients are correctly computed for all species in equilibrium
 */
TEST(PertinentGeochemicalSystemTest, stoichiometry1)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, CaCO3, CaOH+, OH-, (O-phth)--
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++", ">(s)FeOH"},
                                   {"Calcite"},
                                   {},
                                   {"Calcite_asdf"},
                                   {"CH4(aq)"},
                                   {">(s)FeO-", ">(s)FeOCa+"},
                                   "O2(aq)",
                                   "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  ASSERT_EQ(mgd.eqm_species_index.size(), (std::size_t)7);
  for (const auto & sp : {"CO2(aq)", "CO3--", "CaCO3", "CaOH+", "OH-", "(O-phth)--", "Calcite"})
    ASSERT_EQ(mgd.eqm_species_index.count(sp), (std::size_t)1);

  std::map<std::string, DenseMatrix<Real>> stoi_gold;
  for (const auto & sp : {"CO2(aq)",
                          "CO3--",
                          "CaCO3",
                          "CaOH+",
                          "OH-",
                          "(O-phth)--",
                          ">(s)FeO-",
                          ">(s)FeOCa+",
                          "Calcite",
                          "Calcite_asdf",
                          "CH4(aq)"})
    stoi_gold[sp] = DenseMatrix<Real>(1, 6);
  // remember the order of primaries: {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++", ">(s)FeOH"},
  stoi_gold["CO2(aq)"](0, 0) = -1;
  stoi_gold["CO2(aq)"](0, 1) = 1;
  stoi_gold["CO2(aq)"](0, 2) = 1;
  stoi_gold["CO3--"](0, 1) = -1;
  stoi_gold["CO3--"](0, 2) = 1;
  stoi_gold["CaCO3"](0, 4) = 1;
  stoi_gold["CaCO3"](0, 2) = 1;
  stoi_gold["CaCO3"](0, 1) = -1;
  stoi_gold["CaOH+"](0, 4) = 1;
  stoi_gold["CaOH+"](0, 0) = 1;
  stoi_gold["CaOH+"](0, 1) = -1;
  stoi_gold["OH-"](0, 0) = 1;
  stoi_gold["OH-"](0, 1) = -1;
  stoi_gold["(O-phth)--"](0, 0) = -5;
  stoi_gold["(O-phth)--"](0, 2) = 8;
  stoi_gold["(O-phth)--"](0, 1) = 6;
  stoi_gold["(O-phth)--"](0, 3) = -7.5;
  stoi_gold[">(s)FeO-"](0, 5) = 1;
  stoi_gold[">(s)FeO-"](0, 1) = -1;
  stoi_gold[">(s)FeOCa+"](0, 5) = 1;
  stoi_gold[">(s)FeOCa+"](0, 4) = 1;
  stoi_gold[">(s)FeOCa+"](0, 1) = -1;
  stoi_gold["Calcite"](0, 4) = 1;
  stoi_gold["Calcite"](0, 2) = 1;
  stoi_gold["Calcite"](0, 1) = -1;
  stoi_gold["Calcite_asdf"](0, 4) = 2;
  stoi_gold["Calcite_asdf"](0, 2) = 1;
  stoi_gold["Calcite_asdf"](0, 1) = -1;
  // remember the order of primaries: {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++", ">(s)FeOH"},
  stoi_gold["CH4(aq)"](0, 0) = 1;
  stoi_gold["CH4(aq)"](0, 1) = 1;
  stoi_gold["CH4(aq)"](0, 2) = 1;
  stoi_gold["CH4(aq)"](0, 3) = -2;

  for (const auto & sp : {"CO2(aq)", "CO3--", "CaCO3", "CaOH+", "OH-", "(O-phth)--", "Calcite"})
  {
    const unsigned row = mgd.eqm_species_index[sp];
    ASSERT_EQ(mgd.eqm_stoichiometry.sub_matrix(row, 1, 0, 6), stoi_gold[sp]);
  }
  for (const auto & sp : {"Calcite_asdf", "CH4(aq)", ">(s)FeO-", ">(s)FeOCa+"})
  {
    const unsigned row = mgd.kin_species_index[sp];
    ASSERT_EQ(mgd.kin_stoichiometry.sub_matrix(row, 1, 0, 6), stoi_gold[sp]);
  }
}

/// Test that the equilibrium constants are correctly computed and recorded
TEST(PertinentGeochemicalSystemTest, log10K1)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, CaCO3, CaOH+, OH-, (O-phth)--,
  // >(s)FeO-, >(s)FeOCa+
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++", ">(s)FeOH"},
                                   {"Calcite"},
                                   {},
                                   {},
                                   {"CH4(aq)"},
                                   {},
                                   "O2(aq)",
                                   "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  ASSERT_EQ(mgd.eqm_log10K(mgd.eqm_species_index["CO2(aq)"], 0), -6.5570);
  ASSERT_EQ(mgd.eqm_log10K(mgd.eqm_species_index["CO2(aq)"], 1), -6.3660);
  ASSERT_EQ(mgd.eqm_log10K(mgd.eqm_species_index["CO2(aq)"], 2), -6.3325);
  ASSERT_EQ(mgd.eqm_log10K(mgd.eqm_species_index["CO2(aq)"], 3), -6.4330);
  ASSERT_EQ(mgd.eqm_log10K(mgd.eqm_species_index["CO2(aq)"], 4), -6.7420);
  ASSERT_EQ(mgd.eqm_log10K(mgd.eqm_species_index["CO2(aq)"], 5), -7.1880);
  ASSERT_EQ(mgd.eqm_log10K(mgd.eqm_species_index["CO2(aq)"], 6), -7.7630);
  ASSERT_EQ(mgd.eqm_log10K(mgd.eqm_species_index["CO2(aq)"], 7), -8.4650);
  ASSERT_EQ(mgd.eqm_log10K(mgd.eqm_species_index["CO3--"], 0), 10.6169);
  ASSERT_EQ(mgd.eqm_log10K(mgd.eqm_species_index["CaCO3"], 0), 7.5520);
  ASSERT_EQ(mgd.eqm_log10K(mgd.eqm_species_index["CaOH+"], 0), 13.7095);
  ASSERT_EQ(mgd.eqm_log10K(mgd.eqm_species_index["OH-"], 0), 14.9325);
  ASSERT_EQ(mgd.eqm_log10K(mgd.eqm_species_index["(O-phth)--"], 0), 594.3211);
  ASSERT_EQ(mgd.eqm_log10K(mgd.eqm_species_index[">(s)FeO-"], 0), 8.9300);
  ASSERT_EQ(mgd.eqm_log10K(mgd.eqm_species_index[">(s)FeOCa+"], 0), 1.7200);
  ASSERT_EQ(mgd.eqm_log10K(mgd.eqm_species_index["Calcite"], 0), 2.0683);
}

/**
 * Test that the stoichiometric coefficients are correctly computed and recorded, including the case
 * where secondary speices or minerals depend on the basis species only through redox or other
 * secondary species
 */
TEST(PertinentGeochemicalSystemTest, stoichiometry2)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, OH-, (O-phth)--, CH4(aq), Fe+++,
  // >(s)FeO-
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe++", "HCO3-", "O2(aq)"},
                                   {"Fe(OH)3(ppd)fake"},
                                   {"CH4(g)fake"},
                                   {"Fe(OH)3(ppd)"},
                                   {},
                                   {},
                                   "O2(aq)",
                                   "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  ASSERT_EQ(mgd.eqm_species_index.size(), (std::size_t)9);
  for (const auto & sp : {"CO2(aq)",
                          "CO3--",
                          "OH-",
                          "(O-phth)--",
                          "CH4(aq)",
                          "Fe+++",
                          ">(s)FeO-",
                          "Fe(OH)3(ppd)fake",
                          "CH4(g)fake"})
    ASSERT_EQ(mgd.eqm_species_index.count(sp), (std::size_t)1);

  std::map<std::string, DenseMatrix<Real>> stoi_gold;
  for (const auto & sp : {"CO2(aq)",
                          "CO3--",
                          "OH-",
                          "(O-phth)--",
                          "CH4(aq)",
                          "Fe+++",
                          ">(s)FeO-",
                          "Fe(OH)3(ppd)fake",
                          "Fe(OH)3(ppd)",
                          "CH4(g)fake"})
    stoi_gold[sp] = DenseMatrix<Real>(1, 7);
  // remember the order of primaries:
  // {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe++", "HCO3-", "O2(aq)"}
  stoi_gold["CO2(aq)"](0, 0) = -1;
  stoi_gold["CO2(aq)"](0, 1) = 1;
  stoi_gold["CO2(aq)"](0, 5) = 1;
  stoi_gold["CO3--"](0, 5) = 1;
  stoi_gold["CO3--"](0, 1) = -1;
  stoi_gold["OH-"](0, 0) = 1;
  stoi_gold["OH-"](0, 1) = -1;
  stoi_gold["(O-phth)--"](0, 0) = -5;
  stoi_gold["(O-phth)--"](0, 5) = 8;
  stoi_gold["(O-phth)--"](0, 1) = 6;
  stoi_gold["(O-phth)--"](0, 6) = -7.5;
  stoi_gold["CH4(aq)"](0, 0) = 1;
  stoi_gold["CH4(aq)"](0, 1) = 1;
  stoi_gold["CH4(aq)"](0, 5) = 1;
  stoi_gold["CH4(aq)"](0, 6) = -2;
  stoi_gold["Fe+++"](0, 0) = -0.5;
  stoi_gold["Fe+++"](0, 4) = 1;
  stoi_gold["Fe+++"](0, 1) = 1;
  stoi_gold["Fe+++"](0, 6) = 0.25;
  stoi_gold[">(s)FeO-"](0, 2) = 1;
  stoi_gold[">(s)FeO-"](0, 1) = -1;
  stoi_gold["Fe(OH)3(ppd)fake"](0, 1) = -1;
  stoi_gold["Fe(OH)3(ppd)fake"](0, 0) = 2;
  stoi_gold["Fe(OH)3(ppd)fake"](0, 4) = 2;
  stoi_gold["Fe(OH)3(ppd)fake"](0, 6) = 0.5;
  stoi_gold["CH4(g)fake"](0, 0) = 3;
  stoi_gold["CH4(g)fake"](0, 4) = -2;
  stoi_gold["CH4(g)fake"](0, 5) = 3.5;
  stoi_gold["CH4(g)fake"](0, 6) = -4.5;
  stoi_gold["Fe(OH)3(ppd)"](0, 1) = -2;
  stoi_gold["Fe(OH)3(ppd)"](0, 4) = 1;
  stoi_gold["Fe(OH)3(ppd)"](0, 0) = 2.5;
  stoi_gold["Fe(OH)3(ppd)"](0, 6) = 0.25;

  for (const auto & sp : {"CO2(aq)",
                          "CO3--",
                          "OH-",
                          "(O-phth)--",
                          "CH4(aq)",
                          "Fe+++",
                          ">(s)FeO-",
                          "Fe(OH)3(ppd)fake",
                          "CH4(g)fake"})
  {
    const unsigned row = mgd.eqm_species_index[sp];
    ASSERT_EQ(mgd.eqm_stoichiometry.sub_matrix(row, 1, 0, 7), stoi_gold[sp]);
  }
  ASSERT_EQ(mgd.kin_stoichiometry.sub_matrix(0, 1, 0, 7), stoi_gold["Fe(OH)3(ppd)"]);
}

/**
 * Test that the stoichiometric coefficients are correctly computed and recorded, including the case
 * where secondary speices or minerals depend on the basis species only through redox or other
 * secondary species.  Concentrating on kinetic species
 */
TEST(PertinentGeochemicalSystemTest, stoichiometry3)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, OH-, CH4(aq), Fe+++
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe++", "HCO3-", "O2(aq)"},
                                   {},
                                   {"CH4(g)fake"},
                                   {"Fe(OH)3(ppd)", "Fe(OH)3(ppd)fake"},
                                   {"(O-phth)--"},
                                   {">(s)FeO-"},
                                   "O2(aq)",
                                   "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  ASSERT_EQ(mgd.eqm_species_index.size(), (std::size_t)6);
  for (const auto & sp : {"CO2(aq)", "CO3--", "OH-", "CH4(aq)", "Fe+++", "CH4(g)fake"})
    ASSERT_EQ(mgd.eqm_species_index.count(sp), (std::size_t)1);

  std::map<std::string, DenseMatrix<Real>> stoi_gold;
  for (const auto & sp : {"CO2(aq)",
                          "CO3--",
                          "OH-",
                          "(O-phth)--",
                          "CH4(aq)",
                          "Fe+++",
                          ">(s)FeO-",
                          "Fe(OH)3(ppd)fake",
                          "Fe(OH)3(ppd)",
                          "CH4(g)fake"})
    stoi_gold[sp] = DenseMatrix<Real>(1, 7);
  // remember the order of primaries:
  // {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe++", "HCO3-", "O2(aq)"}
  stoi_gold["CO2(aq)"](0, 0) = -1;
  stoi_gold["CO2(aq)"](0, 1) = 1;
  stoi_gold["CO2(aq)"](0, 5) = 1;
  stoi_gold["CO3--"](0, 5) = 1;
  stoi_gold["CO3--"](0, 1) = -1;
  stoi_gold["OH-"](0, 0) = 1;
  stoi_gold["OH-"](0, 1) = -1;
  stoi_gold["(O-phth)--"](0, 0) = -5;
  stoi_gold["(O-phth)--"](0, 5) = 8;
  stoi_gold["(O-phth)--"](0, 1) = 6;
  stoi_gold["(O-phth)--"](0, 6) = -7.5;
  stoi_gold["CH4(aq)"](0, 0) = 1;
  stoi_gold["CH4(aq)"](0, 1) = 1;
  stoi_gold["CH4(aq)"](0, 5) = 1;
  stoi_gold["CH4(aq)"](0, 6) = -2;
  stoi_gold["Fe+++"](0, 0) = -0.5;
  stoi_gold["Fe+++"](0, 4) = 1;
  stoi_gold["Fe+++"](0, 1) = 1;
  stoi_gold["Fe+++"](0, 6) = 0.25;
  stoi_gold[">(s)FeO-"](0, 2) = 1;
  stoi_gold[">(s)FeO-"](0, 1) = -1;
  stoi_gold["Fe(OH)3(ppd)fake"](0, 1) = -1;
  stoi_gold["Fe(OH)3(ppd)fake"](0, 0) = 2;
  stoi_gold["Fe(OH)3(ppd)fake"](0, 4) = 2;
  stoi_gold["Fe(OH)3(ppd)fake"](0, 6) = 0.5;
  stoi_gold["CH4(g)fake"](0, 0) = 3;
  stoi_gold["CH4(g)fake"](0, 4) = -2;
  stoi_gold["CH4(g)fake"](0, 5) = 3.5;
  stoi_gold["CH4(g)fake"](0, 6) = -4.5;
  stoi_gold["Fe(OH)3(ppd)"](0, 1) = -2;
  stoi_gold["Fe(OH)3(ppd)"](0, 4) = 1;
  stoi_gold["Fe(OH)3(ppd)"](0, 0) = 2.5;
  stoi_gold["Fe(OH)3(ppd)"](0, 6) = 0.25;

  for (const auto & sp : {"CO2(aq)", "CO3--", "OH-", "CH4(aq)", "Fe+++", "CH4(g)fake"})
  {
    const unsigned row = mgd.eqm_species_index[sp];
    ASSERT_EQ(mgd.eqm_stoichiometry.sub_matrix(row, 1, 0, 7), stoi_gold[sp]);
  }

  for (const auto & sp : {"Fe(OH)3(ppd)", "Fe(OH)3(ppd)fake", "(O-phth)--", ">(s)FeO-"})
  {
    const unsigned row = mgd.kin_species_index[sp];
    ASSERT_EQ(mgd.kin_stoichiometry.sub_matrix(row, 1, 0, 7), stoi_gold[sp]);
  }
}

/**
 * Test that the equilibrium constants are correctly computed and recorded, including the case
 * where secondary speices or minerals depend on the basis species only through redox or other
 * secondary species
 */
TEST(PertinentGeochemicalSystemTest, log10K2)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, OH-, (O-phth)--, CH4(aq), Fe+++,
  // >(s)FeO-
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe++", "HCO3-", "O2(aq)"},
                                   {"Fe(OH)3(ppd)fake"},
                                   {"CH4(g)fake"},
                                   {},
                                   {},
                                   {},
                                   "O2(aq)",
                                   "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO2(aq)"], 0), -6.5570, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO3--"], 0), 10.6169, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["OH-"], 0), 14.9325, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["(O-phth)--"], 0), 594.3211, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CH4(aq)"], 0), 157.8920, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["Fe+++"], 0), -10.0553, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index[">(s)FeO-"], 0), 8.93, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index[">(s)FeO-"], 1), 8.93 - 0.3 * (25 - 0), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index[">(s)FeO-"], 2), 8.93 - 0.3 * (60 - 0), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index[">(s)FeO-"], 3), 8.93 - 0.3 * (100 - 0), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index[">(s)FeO-"], 4), 8.93 - 0.3 * (150 - 0), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index[">(s)FeO-"], 5), 8.93 - 0.3 * (200 - 0), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index[">(s)FeO-"], 6), 8.93 - 0.3 * (250 - 0), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index[">(s)FeO-"], 7), 8.93 - 0.3 * (300 - 0), eps);
  ASSERT_NEAR(
      mgd.eqm_log10K(mgd.eqm_species_index["Fe(OH)3(ppd)fake"], 0), 6.1946 + 2 * (-10.0553), eps);
  ASSERT_NEAR(
      mgd.eqm_log10K(mgd.eqm_species_index["Fe(OH)3(ppd)fake"], 1), 4.8890 + 2 * (-8.4878), eps);
  ASSERT_NEAR(
      mgd.eqm_log10K(mgd.eqm_species_index["Fe(OH)3(ppd)fake"], 2), 3.4608 + 2 * (-6.6954), eps);
  ASSERT_NEAR(
      mgd.eqm_log10K(mgd.eqm_species_index["Fe(OH)3(ppd)fake"], 3), 2.2392 + 2 * (-5.0568), eps);
  ASSERT_NEAR(
      mgd.eqm_log10K(mgd.eqm_species_index["Fe(OH)3(ppd)fake"], 4), 1.1150 + 2 * (-3.4154), eps);
  ASSERT_NEAR(
      mgd.eqm_log10K(mgd.eqm_species_index["Fe(OH)3(ppd)fake"], 5), 0.2446 + 2 * (-2.0747), eps);
  ASSERT_NEAR(
      mgd.eqm_log10K(mgd.eqm_species_index["Fe(OH)3(ppd)fake"], 6), -0.5504 + 2 * (-0.8908), eps);
  ASSERT_NEAR(
      mgd.eqm_log10K(mgd.eqm_species_index["Fe(OH)3(ppd)fake"], 7), -1.5398 + 2 * (0.2679), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CH4(g)fake"], 0),
              -2.6487 + 2 * 157.8920 - 2 * (-10.0553),
              eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CH4(g)fake"], 1),
              -2.8202 + 2 * 144.1080 - 2 * (-8.4878),
              eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CH4(g)fake"], 2),
              -2.9329 + 2 * 127.9360 - 2 * (-6.6954),
              eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CH4(g)fake"], 3),
              -2.9446 + 2 * 112.8800 - 2 * (-5.0568),
              eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CH4(g)fake"], 4),
              -2.9163 + 2 * 97.7060 - 2 * (-3.4154),
              eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CH4(g)fake"], 5),
              -2.7253 + 2 * 85.2880 - 2 * (-2.0747),
              eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CH4(g)fake"], 6),
              -2.4643 + 2 * 74.7500 - 2 * (-0.8908),
              eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CH4(g)fake"], 7),
              -2.1569 + 2 * 65.6500 - 2 * (0.2679),
              eps);
}

/**
 * Test that equilibrium species ar correctly identified, including the case where there is
 * dependence through redox or other secondary species
 */
TEST(PertinentGeochemicalSystemTest, secondarySpecies2)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, CaCO3, CaOH+, OH-, >(s)FeO-,
  // >(s)FeOCa+
  PertinentGeochemicalSystem model(
      database,
      {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++", ">(s)FeOH", "(O-phth)--"},
      {"Calcite"},
      {},
      {},
      {"CH4(aq)"},
      {},
      "O2(aq)",
      "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  ASSERT_EQ(mgd.eqm_species_index.size(), (std::size_t)8);
  for (const auto & sp :
       {"CO2(aq)", "CO3--", "CaCO3", "CaOH+", "OH-", ">(s)FeO-", ">(s)FeOCa+", "Calcite"})
    ASSERT_EQ(mgd.eqm_species_index.count(sp), (std::size_t)1);
}

/// Test that PertinentGeochemicalSystem correctly identifies minerals
TEST(PertinentGeochemicalSystemTest, isMineral)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, OH-, (O-phth)--, CH4(aq), Fe+++,
  // >(s)FeO-, e-
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe++", "HCO3-", "O2(aq)"},
                                   {"Fe(OH)3(ppd)fake"},
                                   {"CH4(g)fake"},
                                   {},
                                   {},
                                   {},
                                   "O2(aq)",
                                   "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  for (const auto & species : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_mineral[species.second], false);
  for (const auto & species : mgd.eqm_species_index)
    if (species.first == "Fe(OH)3(ppd)fake")
      ASSERT_EQ(mgd.eqm_species_mineral[species.second], true);
    else
      ASSERT_EQ(mgd.eqm_species_mineral[species.second], false);

  PertinentGeochemicalSystem model2(
      database,
      {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe++", "HCO3-", "O2(aq)"},
      {},
      {"CH4(g)fake"},
      {"Fe(OH)3(ppd)fake"},
      {"(O-phth)--"},
      {">(s)FeO-"},
      "O2(aq)",
      "e-");
  ModelGeochemicalDatabase mgd2 = model2.modelGeochemicalDatabase();

  for (const auto & species : mgd2.basis_species_index)
    ASSERT_EQ(mgd2.basis_species_mineral[species.second], false);
  for (const auto & species : mgd2.eqm_species_index)
    ASSERT_EQ(mgd2.eqm_species_mineral[species.second], false);
  for (const auto & species : mgd2.kin_species_index)
    if (species.first == "Fe(OH)3(ppd)fake")
      ASSERT_EQ(mgd2.kin_species_mineral[species.second], true);
    else
      ASSERT_EQ(mgd2.kin_species_mineral[species.second], false);
}

/// Test that PertinentGeochemicalSystem correctly identifies gases
TEST(PertinentGeochemicalSystemTest, isGas)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, OH-, (O-phth)--, CH4(aq), Fe+++,
  // >(s)FeO-, e-
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe++", "HCO3-", "O2(aq)"},
                                   {"Fe(OH)3(ppd)fake"},
                                   {"CH4(g)fake", "O2(g)"},
                                   {},
                                   {},
                                   {},
                                   "O2(aq)",
                                   "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  for (const auto & species : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_gas[species.second], false);
  for (const auto & species : mgd.eqm_species_index)
    if (species.first == "CH4(g)fake" || species.first == "O2(g)")
      ASSERT_EQ(mgd.eqm_species_gas[species.second], true);
    else
      ASSERT_EQ(mgd.eqm_species_gas[species.second], false);
}

/// Test that PertinentGeochemicalSystem correctly identifies transported species
TEST(PertinentGeochemicalSystemTest, isTransported)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, OH-, (O-phth)--, CH4(aq), Fe+++,
  // >(s)FeO-, e-
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe++", "HCO3-", "O2(aq)"},
                                   {"Fe(OH)3(ppd)"},
                                   {"CH4(g)fake"},
                                   {},
                                   {},
                                   {},
                                   "O2(aq)",
                                   "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  for (const auto & species : mgd.basis_species_index)
    if (species.first == ">(s)FeOH" || species.first == ">(w)FeOH")
      EXPECT_EQ(mgd.basis_species_transported[species.second], false);
    else
      EXPECT_EQ(mgd.basis_species_transported[species.second], true);
  for (const auto & species : mgd.eqm_species_index)
    if (species.first == "Fe(OH)3(ppd)" || species.first == ">(s)FeO-")
      EXPECT_EQ(mgd.eqm_species_transported[species.second], false);
    else
      EXPECT_EQ(mgd.eqm_species_transported[species.second], true);

  PertinentGeochemicalSystem model2(
      database,
      {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe++", "HCO3-", "O2(aq)"},
      {},
      {"CH4(g)fake"},
      {"Fe(OH)3(ppd)"},
      {"(O-phth)--"},
      {">(s)FeO-"},
      "O2(aq)",
      "e-");
  ModelGeochemicalDatabase mgd2 = model2.modelGeochemicalDatabase();

  for (const auto & species : mgd2.basis_species_index)
    if (species.first == ">(s)FeOH" || species.first == ">(w)FeOH")
      ASSERT_EQ(mgd2.basis_species_transported[species.second], false);
    else
      ASSERT_EQ(mgd2.basis_species_transported[species.second], true);
  for (const auto & species : mgd2.eqm_species_index)
    ASSERT_EQ(mgd2.eqm_species_transported[species.second], true);
  for (const auto & species : mgd2.kin_species_index)
    if (species.first == "Fe(OH)3(ppd)" || species.first == ">(s)FeO-")
      ASSERT_EQ(mgd2.kin_species_transported[species.second], false);
    else
      ASSERT_EQ(mgd2.kin_species_transported[species.second], true);
}

/// Tests the redox information is correctly captured
TEST(PertinentGeochemicalSystemTest, redoxCapture)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  PertinentGeochemicalSystem model_no_redox(
      database,
      {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe++", "HCO3-", "O2(aq)"},
      {"Fe(OH)3(ppd)fake"},
      {"CH4(g)fake", "O2(g)"},
      {},
      {},
      {},
      "O2(aq)",
      "e-");
  ModelGeochemicalDatabase mgd_no_redox = model_no_redox.modelGeochemicalDatabase();

  EXPECT_EQ(mgd_no_redox.redox_stoichiometry.m(), (unsigned)1);
  EXPECT_EQ(mgd_no_redox.redox_log10K.m(), (unsigned)1);

  PertinentGeochemicalSystem model_redox(
      database,
      {"H2O", "H+", "HCO3-", "O2(aq)", "(O-phth)--", "CH4(aq)", "StoiCheckRedox", "Fe+++"},
      {"Fe(OH)3(ppd)fake"},
      {"CH4(g)fake", "O2(g)"},
      {},
      {},
      {},
      "O2(aq)",
      "e-");
  ModelGeochemicalDatabase mgd_redox = model_redox.modelGeochemicalDatabase();

  EXPECT_EQ(mgd_redox.redox_lhs, "e-");

  // StoiCheckRedox is not expressed in terms of O2(aq), and there is no Fe++ so Fe+++ does not
  // have a pair
  EXPECT_EQ(mgd_redox.redox_stoichiometry.m(), (unsigned)3);
  EXPECT_EQ(mgd_redox.redox_log10K.m(), (unsigned)3);

  // not sure which order the redox has been ordered in.  The reactions are:
  // e- = (1/4/7.5)(O-phth)-- + (1/2 + 5/4/7.5)H2O + (-1 - 6/4/7.5)H+ - 8/4/7.5HCO3-
  // e- = (1/8)CH4(aq) + (1/2 - 1/8)H2O - (1+1/8)H+ - (1/8)HCO3-
  const bool ophth_is_slot_one = (mgd_redox.redox_stoichiometry(1, 4) > 1.0E-6);
  const unsigned ophth_slot = (ophth_is_slot_one ? 1 : 2);
  const unsigned ch4_slot = (ophth_is_slot_one ? 2 : 1);

  // e- = (1/4/7.5)(O-phth)-- + (1/2 + 5/4/7.5)H2O + (-1 - 6/4/7.5)H+ - 8/4/7.5HCO3-
  Real boa = 1.0 / 4.0 / 7.5;
  EXPECT_EQ(mgd_redox.redox_stoichiometry(ophth_slot, 0), 0.5 + 5.0 * boa);
  EXPECT_EQ(mgd_redox.redox_stoichiometry(ophth_slot, 1), -1 - 6.0 * boa);
  EXPECT_EQ(mgd_redox.redox_stoichiometry(ophth_slot, 2), -8.0 * boa);
  EXPECT_EQ(mgd_redox.redox_stoichiometry(ophth_slot, 3), 0.0);
  EXPECT_EQ(mgd_redox.redox_stoichiometry(ophth_slot, 4), boa);
  EXPECT_EQ(mgd_redox.redox_stoichiometry(ophth_slot, 5), 0.0);
  EXPECT_EQ(mgd_redox.redox_stoichiometry(ophth_slot, 6), 0.0);
  EXPECT_EQ(mgd_redox.redox_stoichiometry(ophth_slot, 7), 0.0);
  EXPECT_NEAR(
      mgd_redox.redox_log10K(ophth_slot, 0), -boa * 594.3211 + 22.76135 - 0.25 * (-2.6610), 1E-8);
  EXPECT_NEAR(
      mgd_redox.redox_log10K(ophth_slot, 1), -boa * 542.8292 + 20.7757 - 0.25 * (-2.8990), 1E-8);
  EXPECT_NEAR(
      mgd_redox.redox_log10K(ophth_slot, 2), -boa * 482.3612 + 18.513025 - 0.25 * (-3.0580), 1E-8);
  EXPECT_NEAR(
      mgd_redox.redox_log10K(ophth_slot, 3), -boa * 425.9738 + 16.4658 - 0.25 * (-3.1250), 1E-8);
  EXPECT_NEAR(
      mgd_redox.redox_log10K(ophth_slot, 4), -boa * 368.7004 + 14.473225 - 0.25 * (-3.0630), 1E-8);
  EXPECT_NEAR(
      mgd_redox.redox_log10K(ophth_slot, 5), -boa * 321.8658 + 12.92125 - 0.25 * (-2.9140), 1E-8);
  EXPECT_NEAR(
      mgd_redox.redox_log10K(ophth_slot, 6), -boa * 281.8216 + 11.68165 - 0.25 * (-2.6600), 1E-8);
  EXPECT_NEAR(
      mgd_redox.redox_log10K(ophth_slot, 7), -boa * 246.4849 + 10.67105 - 0.25 * (-2.4100), 1E-8);

  // e- = (1/8)CH4(aq) + (1/2 - 1/8)H2O - (1+1/8)H+ - (1/8)HCO3-
  boa = 1.0 / 8.0;
  EXPECT_EQ(mgd_redox.redox_stoichiometry(ch4_slot, 0), 0.5 - boa);
  EXPECT_EQ(mgd_redox.redox_stoichiometry(ch4_slot, 1), -1 - boa);
  EXPECT_EQ(mgd_redox.redox_stoichiometry(ch4_slot, 2), -boa);
  EXPECT_EQ(mgd_redox.redox_stoichiometry(ch4_slot, 3), 0.0);
  EXPECT_EQ(mgd_redox.redox_stoichiometry(ch4_slot, 4), 0.0);
  EXPECT_EQ(mgd_redox.redox_stoichiometry(ch4_slot, 5), boa);
  EXPECT_EQ(mgd_redox.redox_stoichiometry(ch4_slot, 6), 0.0);
  EXPECT_EQ(mgd_redox.redox_stoichiometry(ch4_slot, 7), 0.0);
  EXPECT_NEAR(
      mgd_redox.redox_log10K(ch4_slot, 0), -boa * 157.8920 + 22.76135 - 0.25 * (-2.6610), 1E-8);
  EXPECT_NEAR(
      mgd_redox.redox_log10K(ch4_slot, 1), -boa * 144.1080 + 20.7757 - 0.25 * (-2.8990), 1E-8);
  EXPECT_NEAR(
      mgd_redox.redox_log10K(ch4_slot, 2), -boa * 127.9360 + 18.513025 - 0.25 * (-3.0580), 1E-8);
  EXPECT_NEAR(
      mgd_redox.redox_log10K(ch4_slot, 3), -boa * 112.8800 + 16.4658 - 0.25 * (-3.1250), 1E-8);
  EXPECT_NEAR(
      mgd_redox.redox_log10K(ch4_slot, 4), -boa * 97.7060 + 14.473225 - 0.25 * (-3.0630), 1E-8);
  EXPECT_NEAR(
      mgd_redox.redox_log10K(ch4_slot, 5), -boa * 85.2880 + 12.92125 - 0.25 * (-2.9140), 1E-8);
  EXPECT_NEAR(
      mgd_redox.redox_log10K(ch4_slot, 6), -boa * 74.7500 + 11.68165 - 0.25 * (-2.6600), 1E-8);
  EXPECT_NEAR(
      mgd_redox.redox_log10K(ch4_slot, 7), -boa * 65.6500 + 10.67105 - 0.25 * (-2.4100), 1E-8);
}

/// Previous test has shown that redox information is correctly captured for usual cases.  This test concentrates on the strange case that the database has e- expressed in terms of secondary species.  Also, with a redox couple that depends on secondary species, so will not be recorded in redox_stoichiometry
TEST(PertinentGeochemicalSystemTest, redoxCapture_db_strange)
{
  GeochemicalDatabaseReader database("database/moose_testdb_e_strange.json");

  PertinentGeochemicalSystem model_redox(
      database, {"H2O", "H+", "HCO3-", "O2(aq)", "(O-phth)--"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd_redox = model_redox.modelGeochemicalDatabase();

  EXPECT_EQ(mgd_redox.redox_lhs, "e-");

  EXPECT_EQ(mgd_redox.redox_stoichiometry.m(), (unsigned)1);
  EXPECT_EQ(mgd_redox.redox_log10K.m(), (unsigned)1);

  // Check the reaction:
  // e- = H2O + 3O2(aq) + OH- = 2H2O + 3O2(aq) - H+
  EXPECT_EQ(mgd_redox.redox_stoichiometry(0, 0), 2.0);
  EXPECT_EQ(mgd_redox.redox_stoichiometry(0, 1), -1.0);
  EXPECT_EQ(mgd_redox.redox_stoichiometry(0, 2), 0.0);
  EXPECT_EQ(mgd_redox.redox_stoichiometry(0, 3), 3.0);
  EXPECT_EQ(mgd_redox.redox_stoichiometry(0, 4), 0.0);
  EXPECT_NEAR(mgd_redox.redox_log10K(0, 0), 22.76135 + 14.9325, 1E-8);
  EXPECT_NEAR(mgd_redox.redox_log10K(0, 1), 20.7757 + 13.9868, 1E-8);
  EXPECT_NEAR(mgd_redox.redox_log10K(0, 2), 18.513025 + 13.0199, 1E-8);
  EXPECT_NEAR(mgd_redox.redox_log10K(0, 3), 16.4658 + 12.2403, 1E-8);
  EXPECT_NEAR(mgd_redox.redox_log10K(0, 4), 14.473225 + 11.5940, 1E-8);
  EXPECT_NEAR(mgd_redox.redox_log10K(0, 5), 12.92125 + 11.2191, 1E-8);
  EXPECT_NEAR(mgd_redox.redox_log10K(0, 6), 11.68165 + 11.0880, 1E-8);
  EXPECT_NEAR(mgd_redox.redox_log10K(0, 7), 10.67105 + 1001.2844, 1E-8);
}

/// Test when a redox couple does not depend on the redox oxide, so the redox couple should not be put into redox_stoichiometry
TEST(PertinentGeochemicalSystemTest, redoxCapture_redox_o_strange)
{
  GeochemicalDatabaseReader database("database/moose_testdb_e_strange.json");

  PertinentGeochemicalSystem model_redox(
      database, {"H2O", "H+", "HCO3-", "O2(aq)", "(O-phth)_0"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd_redox = model_redox.modelGeochemicalDatabase();

  EXPECT_EQ(mgd_redox.redox_lhs, "e-");

  EXPECT_EQ(mgd_redox.redox_stoichiometry.m(), (unsigned)1);
  EXPECT_EQ(mgd_redox.redox_log10K.m(), (unsigned)1);

  // Check the reaction:
  // e- = H2O + 3O2(aq) + OH- = 2H2O + 3O2(aq) - H+
  EXPECT_EQ(mgd_redox.redox_stoichiometry(0, 0), 2.0);
  EXPECT_EQ(mgd_redox.redox_stoichiometry(0, 1), -1.0);
  EXPECT_EQ(mgd_redox.redox_stoichiometry(0, 2), 0.0);
  EXPECT_EQ(mgd_redox.redox_stoichiometry(0, 3), 3.0);
  EXPECT_EQ(mgd_redox.redox_stoichiometry(0, 4), 0.0);
  EXPECT_NEAR(mgd_redox.redox_log10K(0, 0), 22.76135 + 14.9325, 1E-8);
  EXPECT_NEAR(mgd_redox.redox_log10K(0, 1), 20.7757 + 13.9868, 1E-8);
  EXPECT_NEAR(mgd_redox.redox_log10K(0, 2), 18.513025 + 13.0199, 1E-8);
  EXPECT_NEAR(mgd_redox.redox_log10K(0, 3), 16.4658 + 12.2403, 1E-8);
  EXPECT_NEAR(mgd_redox.redox_log10K(0, 4), 14.473225 + 11.5940, 1E-8);
  EXPECT_NEAR(mgd_redox.redox_log10K(0, 5), 12.92125 + 11.2191, 1E-8);
  EXPECT_NEAR(mgd_redox.redox_log10K(0, 6), 11.68165 + 11.0880, 1E-8);
  EXPECT_NEAR(mgd_redox.redox_log10K(0, 7), 10.67105 + 1001.2844, 1E-8);
}

/// Test addKineticRate exceptions
TEST(PertinentGeochemicalSystemTest, addKineticRateExceptions)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++"},
                                   {},
                                   {},
                                   {"Calcite"},
                                   {"CH4(aq)"},
                                   {},
                                   "O2(aq)",
                                   "e-");

  try
  {
    KineticRateUserDescription rate("Ca++",
                                    1.0,
                                    2.0,
                                    true,
                                    0.0,
                                    0.0,
                                    0.0,
                                    {"H2O"},
                                    {3.0},
                                    {0.0},
                                    {0.0},
                                    4.0,
                                    5.0,
                                    6.0,
                                    7.0,
                                    DirectionChoiceEnum::BOTH,
                                    "H2O",
                                    0.0,
                                    -1.0,
                                    0.0);
    model.addKineticRate(rate);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find(
            "Cannot prescribe a kinetic rate to species Ca++ since it is not a kinetic species") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    KineticRateUserDescription rate("CH4(aq)",
                                    1.0,
                                    2.0,
                                    true,
                                    0.0,
                                    0.0,
                                    0.0,
                                    {"H2O", "H++"},
                                    {3.0, 1.0},
                                    {0.0, 0.0},
                                    {0.0, 0.0},
                                    4.0,
                                    5.0,
                                    6.0,
                                    7.0,
                                    DirectionChoiceEnum::BOTH,
                                    "H2O",
                                    0.0,
                                    -1.0,
                                    0.0);
    model.addKineticRate(rate);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Promoting species H++ must be a basis or a secondary species") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    KineticRateUserDescription rate("CH4(aq)",
                                    1.0,
                                    2.0,
                                    true,
                                    0.0,
                                    0.0,
                                    0.0,
                                    {},
                                    {},
                                    {},
                                    {},
                                    4.0,
                                    5.0,
                                    6.0,
                                    7.0,
                                    DirectionChoiceEnum::BOTH,
                                    "CH4(aq)",
                                    0.0,
                                    -1.0,
                                    0.0);
    model.addKineticRate(rate);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Progeny CH4(aq) must be a basis or a secondary species") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Test addKineticRate
TEST(PertinentGeochemicalSystemTest, addKineticRate)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++"},
                                   {},
                                   {},
                                   {"Calcite"},
                                   {"CH4(aq)"},
                                   {},
                                   "O2(aq)",
                                   "e-");
  KineticRateUserDescription rate("CH4(aq)",
                                  1.0,
                                  2.0,
                                  true,
                                  0.0,
                                  0.0,
                                  0.0,
                                  {"H2O", "OH-", "O2(aq)", "CO2(aq)", "CaCO3"},
                                  {3.0, 3.1, 3.2, 3.3, 3.4},
                                  {1.0, 2.0, 3.0, 4.0, 5.0},
                                  {1.25, 1.5, 1.75, 2.25, 3.25},
                                  4.0,
                                  5.0,
                                  6.0,
                                  7.0,
                                  DirectionChoiceEnum::BOTH,
                                  "H+",
                                  2.0,
                                  -1.0,
                                  0.0);
  model.addKineticRate(rate);

  const ModelGeochemicalDatabase & mgd = model.modelGeochemicalDatabase();

  EXPECT_EQ(mgd.kin_rate.size(), (std::size_t)1);
  EXPECT_EQ(mgd.kin_rate[0].kinetic_species_index, mgd.kin_species_index.at("CH4(aq)"));
  EXPECT_EQ(mgd.kin_rate[0].description.intrinsic_rate_constant, 1.0);
  EXPECT_EQ(mgd.kin_rate[0].description.area_quantity, 2.0);
  EXPECT_EQ(mgd.kin_rate[0].description.multiply_by_mass, true);
  std::vector<Real> pi_gold(mgd.basis_species_index.size() + mgd.eqm_species_index.size(), 0.0);
  std::vector<Real> pmi_gold(mgd.basis_species_index.size() + mgd.eqm_species_index.size(), 0.0);
  std::vector<Real> pmk_gold(mgd.basis_species_index.size() + mgd.eqm_species_index.size(), 0.0);
  EXPECT_EQ(mgd.kin_rate[0].promoting_indices.size(), pi_gold.size());
  EXPECT_EQ(mgd.kin_rate[0].promoting_monod_indices.size(), pmi_gold.size());
  EXPECT_EQ(mgd.kin_rate[0].promoting_half_saturation.size(), pmk_gold.size());
  pi_gold[0] = 3.0; // H2O
  pmi_gold[0] = 1.0;
  pmk_gold[0] = 1.25;
  pi_gold[5 + mgd.eqm_species_index.at("OH-")] = 3.1;
  pmi_gold[5 + mgd.eqm_species_index.at("OH-")] = 2.0;
  pmk_gold[5 + mgd.eqm_species_index.at("OH-")] = 1.5;
  pi_gold[3] = 3.2; // O2(aq)
  pmi_gold[3] = 3.0;
  pmk_gold[3] = 1.75;
  pi_gold[5 + mgd.eqm_species_index.at("CO2(aq)")] = 3.3;
  pmi_gold[5 + mgd.eqm_species_index.at("CO2(aq)")] = 4.0;
  pmk_gold[5 + mgd.eqm_species_index.at("CO2(aq)")] = 2.25;
  pi_gold[5 + mgd.eqm_species_index.at("CaCO3")] = 3.4;
  pmi_gold[5 + mgd.eqm_species_index.at("CaCO3")] = 5.0;
  pmk_gold[5 + mgd.eqm_species_index.at("CaCO3")] = 3.25;
  for (unsigned i = 0; i < pi_gold.size(); ++i)
  {
    EXPECT_EQ(mgd.kin_rate[0].promoting_indices[i], pi_gold[i]);
    EXPECT_EQ(mgd.kin_rate[0].promoting_monod_indices[i], pmi_gold[i]);
    EXPECT_EQ(mgd.kin_rate[0].promoting_half_saturation[i], pmk_gold[i]);
  }
  EXPECT_EQ(mgd.kin_rate[0].description.theta, 4.0);
  EXPECT_EQ(mgd.kin_rate[0].description.eta, 5.0);
  EXPECT_EQ(mgd.kin_rate[0].description.activation_energy, 6.0);
  EXPECT_EQ(mgd.kin_rate[0].description.one_over_T0, 7.0);
  EXPECT_EQ(mgd.kin_rate[0].progeny_index, mgd.basis_species_index.at("H+"));
  EXPECT_EQ(mgd.kin_rate[0].description.progeny, "H+");
  EXPECT_EQ(mgd.kin_rate[0].description.progeny_efficiency, 2.0);

  KineticRateUserDescription ratec("Calcite",
                                   7.0,
                                   6.0,
                                   false,
                                   0.0,
                                   0.0,
                                   0.0,
                                   {"H+"},
                                   {-3.0},
                                   {-1.0},
                                   {-2.0},
                                   5.0,
                                   4.0,
                                   3.0,
                                   2.0,
                                   DirectionChoiceEnum::BOTH,
                                   "OH-",
                                   1.125,
                                   -1.0,
                                   0.0);
  model.addKineticRate(ratec);

  EXPECT_EQ(mgd.kin_rate.size(), (std::size_t)2);
  EXPECT_EQ(mgd.kin_rate[1].kinetic_species_index, mgd.kin_species_index.at("Calcite"));
  EXPECT_EQ(mgd.kin_rate[1].description.intrinsic_rate_constant, 7.0);
  EXPECT_EQ(mgd.kin_rate[1].description.area_quantity, 6.0);
  EXPECT_EQ(mgd.kin_rate[1].description.multiply_by_mass, false);
  EXPECT_EQ(mgd.kin_rate[1].promoting_indices.size(), pi_gold.size());
  std::fill(pi_gold.begin(), pi_gold.end(), 0.0);
  std::fill(pmi_gold.begin(), pmi_gold.end(), 0.0);
  std::fill(pmk_gold.begin(), pmk_gold.end(), 0.0);
  pi_gold[1] = -3.0; // H++
  pmi_gold[1] = -1.0;
  pmk_gold[1] = -2.0;
  for (unsigned i = 0; i < pi_gold.size(); ++i)
  {
    EXPECT_EQ(mgd.kin_rate[1].promoting_indices[i], pi_gold[i]);
    EXPECT_EQ(mgd.kin_rate[1].promoting_monod_indices[i], pmi_gold[i]);
    EXPECT_EQ(mgd.kin_rate[1].promoting_half_saturation[i], pmk_gold[i]);
  }
  EXPECT_EQ(mgd.kin_rate[1].description.theta, 5.0);
  EXPECT_EQ(mgd.kin_rate[1].description.eta, 4.0);
  EXPECT_EQ(mgd.kin_rate[1].description.activation_energy, 3.0);
  EXPECT_EQ(mgd.kin_rate[1].description.one_over_T0, 2.0);
  EXPECT_EQ(mgd.kin_rate[1].progeny_index, 5 + mgd.eqm_species_index.at("OH-"));
  EXPECT_EQ(mgd.kin_rate[1].description.progeny, "OH-");
  EXPECT_EQ(mgd.kin_rate[1].description.progeny_efficiency, 1.125);
}

/**
 * Test that the kinetic equilibrium constants are correctly computed and recorded, including the
 * case where the kinetic species depend on the basis species only through redox or other
 * secondary species
 */
TEST(PertinentGeochemicalSystemTest, kin_log10K1)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe++", "HCO3-", "O2(aq)"},
                                   {},
                                   {"CH4(g)fake"},
                                   {"Fe(OH)3(ppd)fake"},
                                   {"(O-phth)--"},
                                   {">(s)FeO-"},
                                   "O2(aq)",
                                   "e-");
  const ModelGeochemicalDatabase & mgd = model.modelGeochemicalDatabase();

  ASSERT_NEAR(mgd.kin_log10K(mgd.kin_species_index.at("Fe(OH)3(ppd)fake"), 0),
              6.1946 + 2 * (-10.0553),
              eps);
  ASSERT_NEAR(
      mgd.kin_log10K(mgd.kin_species_index.at("Fe(OH)3(ppd)fake"), 1), 4.8890 + 2 * (-8.4878), eps);
  ASSERT_NEAR(
      mgd.kin_log10K(mgd.kin_species_index.at("Fe(OH)3(ppd)fake"), 2), 3.4608 + 2 * (-6.6954), eps);
  ASSERT_NEAR(
      mgd.kin_log10K(mgd.kin_species_index.at("Fe(OH)3(ppd)fake"), 3), 2.2392 + 2 * (-5.0568), eps);
  ASSERT_NEAR(
      mgd.kin_log10K(mgd.kin_species_index.at("Fe(OH)3(ppd)fake"), 4), 1.1150 + 2 * (-3.4154), eps);
  ASSERT_NEAR(
      mgd.kin_log10K(mgd.kin_species_index.at("Fe(OH)3(ppd)fake"), 5), 0.2446 + 2 * (-2.0747), eps);
  ASSERT_NEAR(mgd.kin_log10K(mgd.kin_species_index.at("Fe(OH)3(ppd)fake"), 6),
              -0.5504 + 2 * (-0.8908),
              eps);
  ASSERT_NEAR(
      mgd.kin_log10K(mgd.kin_species_index.at("Fe(OH)3(ppd)fake"), 7), -1.5398 + 2 * (0.2679), eps);
  EXPECT_EQ(mgd.kin_log10K(mgd.kin_species_index.at("(O-phth)--"), 0), 594.3211);
  EXPECT_EQ(mgd.kin_log10K(mgd.kin_species_index.at("(O-phth)--"), 1), 542.8292);
  EXPECT_EQ(mgd.kin_log10K(mgd.kin_species_index.at("(O-phth)--"), 2), 482.3612);
  EXPECT_EQ(mgd.kin_log10K(mgd.kin_species_index.at("(O-phth)--"), 3), 425.9738);
  EXPECT_EQ(mgd.kin_log10K(mgd.kin_species_index.at("(O-phth)--"), 4), 368.7004);
  EXPECT_EQ(mgd.kin_log10K(mgd.kin_species_index.at("(O-phth)--"), 5), 321.8658);
  EXPECT_EQ(mgd.kin_log10K(mgd.kin_species_index.at("(O-phth)--"), 6), 281.8216);
  EXPECT_EQ(mgd.kin_log10K(mgd.kin_species_index.at("(O-phth)--"), 7), 246.4849);
  ASSERT_NEAR(mgd.kin_log10K(mgd.kin_species_index.at(">(s)FeO-"), 0), 8.93, eps);
  ASSERT_NEAR(mgd.kin_log10K(mgd.kin_species_index.at(">(s)FeO-"), 1), 8.93 - 0.3 * (25 - 0), eps);
  ASSERT_NEAR(mgd.kin_log10K(mgd.kin_species_index.at(">(s)FeO-"), 2), 8.93 - 0.3 * (60 - 0), eps);
  ASSERT_NEAR(mgd.kin_log10K(mgd.kin_species_index.at(">(s)FeO-"), 3), 8.93 - 0.3 * (100 - 0), eps);
  ASSERT_NEAR(mgd.kin_log10K(mgd.kin_species_index.at(">(s)FeO-"), 4), 8.93 - 0.3 * (150 - 0), eps);
  ASSERT_NEAR(mgd.kin_log10K(mgd.kin_species_index.at(">(s)FeO-"), 5), 8.93 - 0.3 * (200 - 0), eps);
  ASSERT_NEAR(mgd.kin_log10K(mgd.kin_species_index.at(">(s)FeO-"), 6), 8.93 - 0.3 * (250 - 0), eps);
  ASSERT_NEAR(mgd.kin_log10K(mgd.kin_species_index.at(">(s)FeO-"), 7), 8.93 - 0.3 * (300 - 0), eps);
}

/// Test that PertinentGeochemicalSystem correctly initializes swap_to_original_basis
TEST(PertinentGeochemicalSystemTest, initSwapToOrigBasis)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");
  PertinentGeochemicalSystem model(database, {"H2O", "H+"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();
  ASSERT_EQ(mgd.swap_to_original_basis.n(), 0.0);
}

/// Test getIndexOfOriginalBasisSpecies
TEST(PertinentGeochemicalSystemTest, getIndexOfOriginalBasisSpecies)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");
  PertinentGeochemicalSystem model(database, {"H2O", "H+"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  try
  {
    model.getIndexOfOriginalBasisSpecies("OH-");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("species OH- is not in the original basis") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  };
  EXPECT_EQ(model.getIndexOfOriginalBasisSpecies("H2O"), (unsigned)0);
  EXPECT_EQ(model.getIndexOfOriginalBasisSpecies("H+"), (unsigned)1);
}

/// Test originalBasisNames
TEST(PertinentGeochemicalSystemTest, originalBasisNames)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");
  PertinentGeochemicalSystem model(database, {"H2O", "H+"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  EXPECT_EQ(model.originalBasisNames()[0], "H2O");
  EXPECT_EQ(model.originalBasisNames()[1], "H+");
}

/// Test that PertinentGeochemicalSystem correctly builds mineral list when {"*"} is used
TEST(PertinentGeochemicalSystemTest, allMinerals)
{
  GeochemicalDatabaseReader database("database/moose_testdb_all_minerals.json");
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe++", "HCO3-", "O2(aq)"},
                                   {"*"},
                                   {},
                                   {"Fe(OH)3(ppd)fake"},
                                   {"(O-phth)--"},
                                   {">(s)FeO-"},
                                   "O2(aq)",
                                   "e-");
  ModelGeochemicalDatabase mgd2 = model.modelGeochemicalDatabase();

  for (const auto & species : mgd2.basis_species_index)
    ASSERT_EQ(mgd2.basis_species_mineral[species.second], false);
  EXPECT_EQ(mgd2.eqm_species_index.count("Calcite"), (std::size_t)0);
  EXPECT_EQ(mgd2.eqm_species_index.count("Calcite_asdf"), (std::size_t)0);
  EXPECT_EQ(mgd2.eqm_species_index.count("Fe(OH)3(ppd)fake"), (std::size_t)0);
  EXPECT_EQ(mgd2.eqm_species_index.count("Fe(OH)3(ppd)"), (std::size_t)1);
  EXPECT_EQ(mgd2.eqm_species_index.count("Goethite"), (std::size_t)1);
  EXPECT_EQ(mgd2.eqm_species_index.count("Something"), (std::size_t)1);
  for (const auto & species : mgd2.kin_species_index)
    if (species.first == "Fe(OH)3(ppd)fake")
      ASSERT_EQ(mgd2.kin_species_mineral[species.second], true);
    else
      ASSERT_EQ(mgd2.kin_species_mineral[species.second], false);
}
