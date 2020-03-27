//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MinimalGeochemicalSystem.h"

const Real eps = 1E-12; // accounts for precision loss when substituting reactions

/**
 * Test that:
 * - all elements in the basis_species list are basis aqueous species or redox species in the
 * database file
 * - no element appears more than once in the basis_species list
 * - H2O appears first in the basis_species list
 */
TEST(MinimalGeochemicalSystemTest, basisExceptions)
{
  GeochemicalDatabaseReader database("data/moose_testdb.json");

  try
  {
    MinimalGeochemicalSystem model(
        database, {"H2O", "Ca++", "(O-phth)--", "H2O", "Na+"}, {}, {}, {}, {}, {});
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
    MinimalGeochemicalSystem model(
        database, {"Ca++", "H2O", "H+", "(O-phth)--", "Am++++"}, {}, {}, {}, {}, {});
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
    MinimalGeochemicalSystem model(database,
                                   {"H2O", "Ca++", "H+", "(O-phth)--", "Am++++", "does_not_exist"},
                                   {},
                                   {},
                                   {},
                                   {},
                                   {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("does_not_exist does not exist in the basis species or redox species in "
                         "data/moose_testdb.json") != std::string::npos)
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
TEST(MinimalGeochemicalSystemTest, mineralExceptions)
{
  GeochemicalDatabaseReader database("data/moose_testdb.json");

  try
  {
    MinimalGeochemicalSystem model(database,
                                   {"H2O", "Ca++", "H+", "(O-phth)--", "HCO3-"},
                                   {"Calcite", "Calcite_asdf", "Calcite"},
                                   {},
                                   {},
                                   {},
                                   {});
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
    MinimalGeochemicalSystem model(database,
                                   {"H2O", "Ca++", "H+", "Am++++", "HCO3-"},
                                   {"Calcite", "Calcite_asdf", "does_not_exist"},
                                   {},
                                   {},
                                   {},
                                   {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("does_not_exist does not exist in database data/moose_testdb.json") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    MinimalGeochemicalSystem model(
        database, {"H2O", "H+", "(O-phth)--", "HCO3-"}, {"Calcite"}, {}, {}, {}, {});
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
    MinimalGeochemicalSystem model(
        database, {"H2O", "H+", "O2(aq)"}, {"Fe(OH)3(ppd)"}, {}, {}, {}, {});
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
    MinimalGeochemicalSystem model(
        database, {"H2O", "H+", "O2(aq)", "Fe++", ">(s)FeOH"}, {"Fe(OH)3(ppd)"}, {}, {}, {}, {});
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
  MinimalGeochemicalSystem model(database,
                                 {"H2O", "H+", "O2(aq)", "Fe++", ">(s)FeOH", ">(w)FeOH"},
                                 {"Fe(OH)3(ppd)"},
                                 {},
                                 {},
                                 {},
                                 {});
}

/**
 * Test that:
 * - all elements in the gases list are gases in the database file
 * - no element appears more than once in this list
 * - the equilibrium reaction of each of these contains only species that are reducable to basis
 * species
 */
TEST(MinimalGeochemicalSystemTest, gasExceptions)
{
  GeochemicalDatabaseReader database("data/moose_testdb.json");

  try
  {
    MinimalGeochemicalSystem model(database,
                                   {"H2O", "Ca++", "H+", "HCO3-", "CH4(aq)"},
                                   {"Calcite"},
                                   {"CH4(g)", "CH4(g)"},
                                   {},
                                   {},
                                   {});
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
    MinimalGeochemicalSystem model(database,
                                   {"H2O", "Ca++", "H+", "HCO3-", "CH4(aq)"},
                                   {"Calcite"},
                                   {"CH4(g)", "does_not_exist"},
                                   {},
                                   {},
                                   {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("does_not_exist does not exist in database data/moose_testdb.json") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    MinimalGeochemicalSystem model(
        database, {"H2O", "Ca++", "H+", "HCO3-"}, {"Calcite"}, {"CH4(g)"}, {}, {}, {});
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
  MinimalGeochemicalSystem model(
      database, {"H2O", "Ca++", "H+", "HCO3-", "O2(aq)"}, {"Calcite"}, {"CH4(g)"}, {}, {}, {});
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
TEST(MinimalGeochemicalSystemTest, kineticMineralExceptions)
{
  GeochemicalDatabaseReader database("data/moose_testdb.json");

  try
  {
    MinimalGeochemicalSystem model(
        database, {"H2O"}, {}, {}, {"Calcite", "Calcite_asdf", "Calcite"}, {}, {});
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
    MinimalGeochemicalSystem model(database,
                                   {"H2O", "Ca++", "HCO3-", "H+"},
                                   {"Calcite_asdf"},
                                   {},
                                   {"Calcite", "Calcite_asdf"},
                                   {},
                                   {});
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
    MinimalGeochemicalSystem model(database,
                                   {"H2O", "Ca++", "HCO3-", "H+"},
                                   {"Calcite_asdf"},
                                   {},
                                   {"Calcite", "does_not_exist"},
                                   {},
                                   {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("does_not_exist does not exist in database data/moose_testdb.json") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    MinimalGeochemicalSystem model(
        database, {"H2O", "O2(aq)", "H+"}, {}, {}, {"Fe(OH)3(ppd)"}, {}, {});
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
    MinimalGeochemicalSystem model(
        database, {"H2O", "H+", "O2(aq)", "Fe++", ">(s)FeOH"}, {}, {}, {"Fe(OH)3(ppd)"}, {}, {});
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
  MinimalGeochemicalSystem model(database,
                                 {"H2O", "H+", "O2(aq)", "Fe++", ">(s)FeOH", ">(w)FeOH"},
                                 {},
                                 {},
                                 {"Fe(OH)3(ppd)"},
                                 {},
                                 {});
}

/**
 * Test that:
 * - all elements in the kinetic_redox list are "redox couples" in the database file
 * - no element appears more than once in this list
 * - no member of the kinetic_redox list can also appear in the basis_species list
 * - the equilibrium reaction of each of these contains only species that are reducable to basis
 * species
 */
TEST(MinimalGeochemicalSystemTest, kineticRedoxExceptions)
{
  GeochemicalDatabaseReader database("data/moose_testdb.json");

  try
  {
    MinimalGeochemicalSystem model(
        database, {"H2O"}, {}, {}, {}, {"(O-phth)--", "Am++++", "(O-phth)--"}, {});
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
    MinimalGeochemicalSystem model(
        database, {"H2O", "(O-phth)--"}, {}, {}, {}, {"Am++++", "(O-phth)--"}, {});
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
    MinimalGeochemicalSystem model(
        database, {"H2O"}, {}, {}, {}, {"(O-phth)--", "does_not_exist"}, {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("does_not_exist does not exist in database data/moose_testdb.json") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    MinimalGeochemicalSystem model(
        database, {"H2O", "HCO3-", "H+"}, {}, {}, {}, {"(O-phth)--"}, {});
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
  MinimalGeochemicalSystem model(
      database, {"H2O", "HCO3-", "H+", "O2(aq)"}, {}, {}, {}, {"(O-phth)--"}, {});
}

/**
 * Test that:
 * - all elements in the kinetic_surface_species list are "surface species" in the database file
 * - no element appears more than once in this list
 * - the equilibrium reaction of each of these contains only species that are reducable to basis
 * species
 */
TEST(MinimalGeochemicalSystemTest, kineticSurfaceExceptions)
{
  GeochemicalDatabaseReader database("data/moose_testdb.json");

  try
  {
    MinimalGeochemicalSystem model(
        database, {"H2O"}, {}, {}, {}, {}, {">(s)FeO-", ">(s)FeOCa+", ">(s)FeO-"});
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
    MinimalGeochemicalSystem model(
        database, {"H2O"}, {}, {}, {}, {}, {">(s)FeO-", ">(s)FeOCa+", "does_not_exist"});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("does_not_exist does not exist in database data/moose_testdb.json") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    MinimalGeochemicalSystem model(database, {"H2O", "H+"}, {}, {}, {}, {}, {">(s)FeO-"});
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
  MinimalGeochemicalSystem model(database, {"H2O", "H+", ">(s)FeOH"}, {}, {}, {}, {}, {">(s)FeO-"});
}

/// Test that MinimalGeochemicalSystem correctly extracts temperatures from the GeochemicalDatabaseReader
TEST(MinimalGeochemicalSystemTest, temperatures)
{
  GeochemicalDatabaseReader database("data/moose_testdb.json");

  MinimalGeochemicalSystem model(database, {"H2O"}, {}, {}, {}, {}, {});
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabaseCopy();

  ASSERT_EQ(mgd.temperatures, database.getTemperatures());
}

/**
 * Test that the names of basis and equilibrium species are correctly recorded by
 * MinimalGeochemicalSystem This mostly tests that given a set of basis species, the set of
 * equilibrium and kinetic species is correct
 */
TEST(MinimalGeochemicalSystemTest, names1)
{
  GeochemicalDatabaseReader database("data/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, CaCO3, CaOH+, OH-, (O-phth)--,
  // >(s)FeO-, e-
  MinimalGeochemicalSystem model(database,
                                 {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++", ">(s)FeOH"},
                                 {"Calcite"},
                                 {"O2(g)"},
                                 {"Calcite_asdf"},
                                 {"CH4(aq)"},
                                 {">(s)FeOCa+"});
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabaseCopy();

  ASSERT_EQ(mgd.basis_species_index.size(), 6);
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_name[sp.second], sp.first);

  ASSERT_EQ(mgd.eqm_species_index.size(), 10);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_name[sp.second], sp.first);

  ASSERT_EQ(mgd.kin_species_index.size(), 3);
  for (const auto & sp : mgd.kin_species_index)
    ASSERT_EQ(mgd.kin_species_name[sp.second], sp.first);
}

/**
 * Another test that the names of basis and equilibrium species are correctly recorded by
 * MinimalGeochemicalSystem This mostly tests that given a set of basis species, the set of
 * equilibrium and kinetic species is correct
 */
TEST(MinimalGeochemicalSystemTest, names2)
{
  GeochemicalDatabaseReader database("data/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, CaCO3, CaOH+, OH-, (O-phth)--, e-
  MinimalGeochemicalSystem model(database,
                                 {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++", ">(s)FeOH"},
                                 {},
                                 {"O2(g)"},
                                 {"Calcite_asdf", "Calcite"},
                                 {"CH4(aq)"},
                                 {">(s)FeOCa+", ">(s)FeO-"});
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabaseCopy();

  ASSERT_EQ(mgd.basis_species_index.size(), 6);
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_name[sp.second], sp.first);

  ASSERT_EQ(mgd.eqm_species_index.size(), 8);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_name[sp.second], sp.first);

  ASSERT_EQ(mgd.kin_species_index.size(), 5);
  for (const auto & sp : mgd.kin_species_index)
    ASSERT_EQ(mgd.kin_species_name[sp.second], sp.first);
}

/// Test that the charge of species is correctly recorded
TEST(MinimalGeochemicalSystemTest, charge)
{
  GeochemicalDatabaseReader database("data/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, CaCO3, CaOH+, OH-, (O-phth)--,
  // >(s)FeO-, e-
  MinimalGeochemicalSystem model(database,
                                 {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++", ">(s)FeOH"},
                                 {"Calcite"},
                                 {},
                                 {"Calcite_asdf"},
                                 {"CH4(aq)"},
                                 {">(s)FeOCa+"});
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabaseCopy();

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
TEST(MinimalGeochemicalSystemTest, radius)
{
  GeochemicalDatabaseReader database("data/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, CaCO3, CaOH+, OH-, (O-phth)--,
  // >(s)FeO-, >(s)FeOCa+, e-
  MinimalGeochemicalSystem model(database,
                                 {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++", ">(s)FeOH"},
                                 {"Calcite"},
                                 {},
                                 {},
                                 {"CH4(aq)"},
                                 {});
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabaseCopy();

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
  radius_gold[">(s)FeO-"] = 0.0;
  radius_gold[">(s)FeOCa+"] = 0.0;
  radius_gold["Calcite"] = 0.0;
  radius_gold["e-"] = 0.0;
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_radius[sp.second], radius_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_radius[sp.second], radius_gold[sp.first]);
}

/// Test that the molecular weight of each species is correctly recorded
TEST(MinimalGeochemicalSystemTest, molecular_weight)
{
  GeochemicalDatabaseReader database("data/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, CaCO3, CaOH+, OH-, (O-phth)--,
  // >(s)FeO-, >(s)FeOCa+, e-
  MinimalGeochemicalSystem model(database,
                                 {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++", ">(s)FeOH"},
                                 {"Calcite"},
                                 {},
                                 {"Calcite_asdf"},
                                 {"CH4(aq)"},
                                 {">(s)FeOCa+"});
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabaseCopy();

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
TEST(MinimalGeochemicalSystemTest, molecular_volume)
{
  GeochemicalDatabaseReader database("data/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, CaCO3, CaOH+, OH-, (O-phth)--,
  // >(s)FeO-, >(s)FeOCa+, e-
  MinimalGeochemicalSystem model(database,
                                 {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++", ">(s)FeOH"},
                                 {"Calcite"},
                                 {},
                                 {"Calcite_asdf"},
                                 {"CH4(aq)"},
                                 {">(s)FeOCa+"});
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabaseCopy();

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
TEST(MinimalGeochemicalSystemTest, surfaceComplexationInfo)
{
  GeochemicalDatabaseReader database("data/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, CaCO3, CaOH+, OH-, (O-phth)--,
  // >(s)FeO-, >(s)FeOCa+, e-
  MinimalGeochemicalSystem model1(database,
                                  {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++", ">(s)FeOH"},
                                  {"Calcite"},
                                  {},
                                  {},
                                  {"CH4(aq)"},
                                  {});
  ModelGeochemicalDatabase mgd1 = model1.modelGeochemicalDatabaseCopy();

  ASSERT_EQ(mgd1.surface_complexation_info.size(), 0);

  // The following system has secondary species: CO2(aq), CO3--, CaCO3, CaOH+, OH-, (O-phth)--,
  // >(s)FeO-, >(s)FeOCa+, e-
  MinimalGeochemicalSystem model2(database,
                                  {"H2O", "H+", "O2(aq)", "Fe++", ">(s)FeOH", ">(w)FeOH"},
                                  {"Fe(OH)3(ppd)"},
                                  {},
                                  {},
                                  {},
                                  {});
  ModelGeochemicalDatabase mgd2 = model2.modelGeochemicalDatabaseCopy();

  ASSERT_EQ(mgd2.surface_complexation_info.count("Fe(OH)3(ppd)"), 1);
  ASSERT_EQ(mgd2.surface_complexation_info["Fe(OH)3(ppd)"].surface_area, 600.0);
  ASSERT_EQ(mgd2.surface_complexation_info["Fe(OH)3(ppd)"].sorption_sites[">(s)FeOH"], 0.005);
  ASSERT_EQ(mgd2.surface_complexation_info["Fe(OH)3(ppd)"].sorption_sites[">(w)FeOH"], 0.2);

  // The following system has secondary species: CO2(aq), CO3--, CaCO3, CaOH+, OH-, (O-phth)--,
  // >(s)FeO-, >(s)FeOCa+, e-
  MinimalGeochemicalSystem model3(database,
                                  {"H2O", "H+", "O2(aq)", "Fe++", ">(s)FeOH", ">(w)FeOH"},
                                  {},
                                  {},
                                  {"Goethite"},
                                  {},
                                  {});
  ModelGeochemicalDatabase mgd3 = model3.modelGeochemicalDatabaseCopy();

  ASSERT_EQ(mgd3.surface_complexation_info.count("Goethite"), 1);
  ASSERT_EQ(mgd3.surface_complexation_info["Goethite"].surface_area, 60.0);
  ASSERT_EQ(mgd3.surface_complexation_info["Goethite"].sorption_sites[">(s)FeOH"], 0.05);
  ASSERT_EQ(mgd3.surface_complexation_info["Goethite"].sorption_sites[">(w)FeOH"], 0.222);
}

/// Test that the fugacity coefficients are correctly recorded
TEST(MinimalGeochemicalSystemTest, GasChi)
{
  GeochemicalDatabaseReader database("data/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, CaCO3, CaOH+, OH-, (O-phth)--,
  // >(s)FeO-, >(s)FeOCa+, e-
  MinimalGeochemicalSystem model1(database,
                                  {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++", ">(s)FeOH"},
                                  {"Calcite"},
                                  {},
                                  {},
                                  {"CH4(aq)"},
                                  {});
  ModelGeochemicalDatabase mgd1 = model1.modelGeochemicalDatabaseCopy();

  ASSERT_EQ(mgd1.gas_chi.size(), 0);

  MinimalGeochemicalSystem model2(
      database, {"H2O", "Ca++", "H+", "HCO3-", "O2(aq)"}, {"Calcite"}, {"CH4(g)"}, {}, {}, {});
  ModelGeochemicalDatabase mgd2 = model2.modelGeochemicalDatabaseCopy();

  ASSERT_EQ(mgd2.gas_chi.count("CH4(g)"), 1);
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
TEST(MinimalGeochemicalSystemTest, stoichiometry1)
{
  GeochemicalDatabaseReader database("data/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, CaCO3, CaOH+, OH-, (O-phth)--, e-
  MinimalGeochemicalSystem model(database,
                                 {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++", ">(s)FeOH"},
                                 {"Calcite"},
                                 {},
                                 {"Calcite_asdf"},
                                 {"CH4(aq)"},
                                 {">(s)FeO-", ">(s)FeOCa+"});
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabaseCopy();

  ASSERT_EQ(mgd.eqm_species_index.size(), 8);
  for (const auto & sp :
       {"CO2(aq)", "CO3--", "CaCO3", "CaOH+", "OH-", "(O-phth)--", "e-", "Calcite"})
    ASSERT_EQ(mgd.eqm_species_index.count(sp), 1);

  std::map<std::string, DenseMatrix<Real>> stoi_gold;
  for (const auto & sp : {"CO2(aq)",
                          "CO3--",
                          "CaCO3",
                          "CaOH+",
                          "OH-",
                          "(O-phth)--",
                          ">(s)FeO-",
                          ">(s)FeOCa+",
                          "e-",
                          "Calcite",
                          "Calcite_asdf",
                          "CH4(aq)"})
    stoi_gold[sp] = DenseMatrix<Real>(1, 6);
  // remember the order of primaries: {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++", ">(s)FeOH"},
  stoi_gold["e-"](0, 0) = 0.5;
  stoi_gold["e-"](0, 1) = -1.0;
  stoi_gold["e-"](0, 3) = -0.25;
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

  for (const auto & sp :
       {"CO2(aq)", "CO3--", "CaCO3", "CaOH+", "OH-", "(O-phth)--", "e-", "Calcite"})
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
TEST(MinimalGeochemicalSystemTest, log10K1)
{
  GeochemicalDatabaseReader database("data/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, CaCO3, CaOH+, OH-, (O-phth)--,
  // >(s)FeO-, >(s)FeOCa+, e-
  MinimalGeochemicalSystem model(database,
                                 {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++", ">(s)FeOH"},
                                 {"Calcite"},
                                 {},
                                 {},
                                 {"CH4(aq)"},
                                 {});
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabaseCopy();

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
  ASSERT_EQ(mgd.eqm_log10K(mgd.eqm_species_index["e-"], 0), 23.4266);
}

/**
 * Test that the stoichiometric coefficients are correctly computed and recorded, including the case
 * where secondary speices or minerals depend on the basis species only through redox or other
 * secondary species
 */
TEST(MinimalGeochemicalSystemTest, stoichiometry2)
{
  GeochemicalDatabaseReader database("data/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, OH-, (O-phth)--, CH4(aq), Fe+++,
  // >(s)FeO-, e-
  MinimalGeochemicalSystem model(database,
                                 {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe++", "HCO3-", "O2(aq)"},
                                 {"Fe(OH)3(ppd)fake"},
                                 {"CH4(g)fake"},
                                 {"Fe(OH)3(ppd)"},
                                 {},
                                 {});
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabaseCopy();

  ASSERT_EQ(mgd.eqm_species_index.size(), 10);
  for (const auto & sp : {"CO2(aq)",
                          "CO3--",
                          "OH-",
                          "(O-phth)--",
                          "CH4(aq)",
                          "Fe+++",
                          ">(s)FeO-",
                          "e-",
                          "Fe(OH)3(ppd)fake",
                          "CH4(g)fake"})
    ASSERT_EQ(mgd.eqm_species_index.count(sp), 1);

  std::map<std::string, DenseMatrix<Real>> stoi_gold;
  for (const auto & sp : {"CO2(aq)",
                          "CO3--",
                          "OH-",
                          "(O-phth)--",
                          "CH4(aq)",
                          "Fe+++",
                          ">(s)FeO-",
                          "e-",
                          "Fe(OH)3(ppd)fake",
                          "Fe(OH)3(ppd)",
                          "CH4(g)fake"})
    stoi_gold[sp] = DenseMatrix<Real>(1, 7);
  // remember the order of primaries:
  // {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe++", "HCO3-", "O2(aq)"}
  stoi_gold["e-"](0, 0) = 0.5;
  stoi_gold["e-"](0, 1) = -1;
  stoi_gold["e-"](0, 6) = -0.25;
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
                          "e-",
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
TEST(MinimalGeochemicalSystemTest, stoichiometry3)
{
  GeochemicalDatabaseReader database("data/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, OH-, CH4(aq), Fe+++, e-
  MinimalGeochemicalSystem model(database,
                                 {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe++", "HCO3-", "O2(aq)"},
                                 {},
                                 {"CH4(g)fake"},
                                 {"Fe(OH)3(ppd)", "Fe(OH)3(ppd)fake"},
                                 {"(O-phth)--"},
                                 {">(s)FeO-"});
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabaseCopy();

  ASSERT_EQ(mgd.eqm_species_index.size(), 7);
  for (const auto & sp : {"CO2(aq)", "CO3--", "OH-", "CH4(aq)", "Fe+++", "e-", "CH4(g)fake"})
    ASSERT_EQ(mgd.eqm_species_index.count(sp), 1);

  std::map<std::string, DenseMatrix<Real>> stoi_gold;
  for (const auto & sp : {"CO2(aq)",
                          "CO3--",
                          "OH-",
                          "(O-phth)--",
                          "CH4(aq)",
                          "Fe+++",
                          ">(s)FeO-",
                          "e-",
                          "Fe(OH)3(ppd)fake",
                          "Fe(OH)3(ppd)",
                          "CH4(g)fake"})
    stoi_gold[sp] = DenseMatrix<Real>(1, 7);
  // remember the order of primaries:
  // {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe++", "HCO3-", "O2(aq)"}
  stoi_gold["e-"](0, 0) = 0.5;
  stoi_gold["e-"](0, 1) = -1;
  stoi_gold["e-"](0, 6) = -0.25;
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

  for (const auto & sp : {"CO2(aq)", "CO3--", "OH-", "CH4(aq)", "Fe+++", "e-", "CH4(g)fake"})
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
TEST(MinimalGeochemicalSystemTest, log10K2)
{
  GeochemicalDatabaseReader database("data/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, OH-, (O-phth)--, CH4(aq), Fe+++,
  // >(s)FeO-, e-
  MinimalGeochemicalSystem model(database,
                                 {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe++", "HCO3-", "O2(aq)"},
                                 {"Fe(OH)3(ppd)fake"},
                                 {"CH4(g)fake"},
                                 {},
                                 {},
                                 {});
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabaseCopy();

  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO2(aq)"], 0), -6.5570, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO3--"], 0), 10.6169, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["OH-"], 0), 14.9325, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["(O-phth)--"], 0), 594.3211, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CH4(aq)"], 0), 157.8920, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["Fe+++"], 0), -10.0553, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["e-"], 0), 23.4266, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index[">(s)FeO-"], 0), 8.93, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index[">(s)FeO-"], 1), 8.93 + 2 * (25 - 0), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index[">(s)FeO-"], 2), 8.93 + 2 * (60 - 0), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index[">(s)FeO-"], 3), 8.93 + 2 * (100 - 0), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index[">(s)FeO-"], 4), 8.93 + 2 * (150 - 0), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index[">(s)FeO-"], 5), 8.93 + 2 * (200 - 0), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index[">(s)FeO-"], 6), 8.93 + 2 * (250 - 0), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index[">(s)FeO-"], 7), 8.93 + 2 * (300 - 0), eps);
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
TEST(MinimalGeochemicalSystemTest, secondarySpecies2)
{
  GeochemicalDatabaseReader database("data/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, CaCO3, CaOH+, OH-, >(s)FeO-,
  // >(s)FeOCa+, e-
  MinimalGeochemicalSystem model(database,
                                 {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++", ">(s)FeOH", "(O-phth)--"},
                                 {"Calcite"},
                                 {},
                                 {},
                                 {"CH4(aq)"},
                                 {});
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabaseCopy();

  ASSERT_EQ(mgd.eqm_species_index.size(), 9);
  for (const auto & sp :
       {"CO2(aq)", "CO3--", "CaCO3", "CaOH+", "OH-", ">(s)FeO-", ">(s)FeOCa+", "Calcite", "e-"})
    ASSERT_EQ(mgd.eqm_species_index.count(sp), 1);
}

/// Test that MinimalGeochemicalSystem correctly identifies minerals
TEST(MinimalGeochemicalSystemTest, isMineral)
{
  GeochemicalDatabaseReader database("data/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, OH-, (O-phth)--, CH4(aq), Fe+++,
  // >(s)FeO-, e-
  MinimalGeochemicalSystem model(database,
                                 {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe++", "HCO3-", "O2(aq)"},
                                 {"Fe(OH)3(ppd)fake"},
                                 {"CH4(g)fake"},
                                 {},
                                 {},
                                 {});
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabaseCopy();

  for (const auto & species : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_mineral[species.second], false);
  for (const auto & species : mgd.eqm_species_index)
    if (species.first == "Fe(OH)3(ppd)fake")
      ASSERT_EQ(mgd.eqm_species_mineral[species.second], true);
    else
      ASSERT_EQ(mgd.eqm_species_mineral[species.second], false);

  MinimalGeochemicalSystem model2(database,
                                  {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe++", "HCO3-", "O2(aq)"},
                                  {},
                                  {"CH4(g)fake"},
                                  {"Fe(OH)3(ppd)fake"},
                                  {"(O-phth)--"},
                                  {">(s)FeO-"});
  ModelGeochemicalDatabase mgd2 = model2.modelGeochemicalDatabaseCopy();

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

/// Test that MinimalGeochemicalSystem correctly identifies gases
TEST(MinimalGeochemicalSystemTest, isGas)
{
  GeochemicalDatabaseReader database("data/moose_testdb.json");

  // The following system has secondary species: CO2(aq), CO3--, OH-, (O-phth)--, CH4(aq), Fe+++,
  // >(s)FeO-, e-
  MinimalGeochemicalSystem model(database,
                                 {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe++", "HCO3-", "O2(aq)"},
                                 {"Fe(OH)3(ppd)fake"},
                                 {"CH4(g)fake", "O2(g)"},
                                 {},
                                 {},
                                 {});
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabaseCopy();

  for (const auto & species : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_gas[species.second], false);
  for (const auto & species : mgd.eqm_species_index)
    if (species.first == "CH4(g)fake" || species.first == "O2(g)")
      ASSERT_EQ(mgd.eqm_species_gas[species.second], true);
    else
      ASSERT_EQ(mgd.eqm_species_gas[species.second], false);
}
