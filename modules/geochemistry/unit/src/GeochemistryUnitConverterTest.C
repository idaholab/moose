//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "GeochemistryUnitConverter.h"

const GeochemicalDatabaseReader db("database/moose_testdb.json", true, true, false);
const PertinentGeochemicalSystem
    model(db,
          {"H2O", "Ca++", "HCO3-", "H+", ">(s)FeOH", "Fe++", "CH4(aq)", "O2(aq)", ">(w)FeOH"},
          {"Calcite", "Fe(OH)3(ppd)"},
          {"CH4(g)"},
          {"Something"},
          {"(O-phth)--"},
          {">(s)FeO-"},
          "O2(aq)",
          "e-");
const ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

// Test exceptions
TEST(GeochemistryUnitConverterTest, exceptions)
{
  try
  {
    GeochemistryUnitConverter::toMoles(
        1.0, GeochemistryUnitConverter::GeochemistryUnit::MOLES, "bad_name", mgd);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find(
            "GeochemistryUnitConverter: bad_name is not a basis, equilibrium or kinetic species") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    GeochemistryUnitConverter::toMoles(
        1.0, GeochemistryUnitConverter::GeochemistryUnit::CM3, "Ca++", mgd);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("GeochemistryUnitConverter: Cannot use CM3 units for species Ca++ because "
                         "it is not a mineral") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    GeochemistryUnitConverter::toMoles(
        1.0, GeochemistryUnitConverter::GeochemistryUnit::CM3, "CO3--", mgd);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("GeochemistryUnitConverter: Cannot use CM3 units for species CO3-- because "
                 "it is not a mineral") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    GeochemistryUnitConverter::toMoles(
        1.0, GeochemistryUnitConverter::GeochemistryUnit::CM3, "CH4(g)", mgd);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("GeochemistryUnitConverter: Cannot use CM3 units for species CH4(g) because "
                 "it is not a mineral") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    GeochemistryUnitConverter::toMoles(
        1.0, GeochemistryUnitConverter::GeochemistryUnit::CM3, "(O-phth)--", mgd);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("GeochemistryUnitConverter: Cannot use CM3 units for species (O-phth)-- because "
                 "it is not a mineral") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    GeochemistryUnitConverter::toMoles(
        1.0, GeochemistryUnitConverter::GeochemistryUnit::CM3, ">(s)FeO-", mgd);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("GeochemistryUnitConverter: Cannot use CM3 units for species >(s)FeO- because "
                 "it is not a mineral") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

// Test fromMoles
TEST(GeochemistryUnitConverterTest, fromMoles)
{
  EXPECT_EQ(GeochemistryUnitConverter::fromMoles(
                11.0, GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS, "H2O", mgd),
            11.0);
  EXPECT_EQ(GeochemistryUnitConverter::fromMoles(
                11.0, GeochemistryUnitConverter::GeochemistryUnit::MOLES, "CO3--", mgd),
            11.0);
  EXPECT_EQ(GeochemistryUnitConverter::fromMoles(
                11.0, GeochemistryUnitConverter::GeochemistryUnit::MOLAL, "CaCO3", mgd),
            11.0);
  EXPECT_NEAR(GeochemistryUnitConverter::fromMoles(
                  11.0, GeochemistryUnitConverter::GeochemistryUnit::G, "Ca++", mgd),
              11.0 * 40.08,
              1.0E-8);
  EXPECT_NEAR(GeochemistryUnitConverter::fromMoles(
                  11.0, GeochemistryUnitConverter::GeochemistryUnit::KG, "Ca++", mgd),
              11.0 * 40.08 / 1.0E3,
              1.0E-8);
  EXPECT_NEAR(GeochemistryUnitConverter::fromMoles(
                  11.0, GeochemistryUnitConverter::GeochemistryUnit::MG, "HCO3-", mgd),
              11.0 * 61.0171 * 1.0E3,
              1.0E-8);
  EXPECT_NEAR(GeochemistryUnitConverter::fromMoles(
                  11.0, GeochemistryUnitConverter::GeochemistryUnit::UG, "H+", mgd),
              11.0 * 1.0079 * 1.0E6,
              1.0E-3);
  EXPECT_NEAR(
      GeochemistryUnitConverter::fromMoles(
          11.0, GeochemistryUnitConverter::GeochemistryUnit::KG_PER_KG_SOLVENT, ">(s)FeOH", mgd),
      11.0 * 72.8543 * 1E-3,
      1.0E-8);
  EXPECT_NEAR(
      GeochemistryUnitConverter::fromMoles(
          11.0, GeochemistryUnitConverter::GeochemistryUnit::G_PER_KG_SOLVENT, ">(w)FeOH", mgd),
      11.0 * 1234.567,
      1.0E-8);
  EXPECT_NEAR(
      GeochemistryUnitConverter::fromMoles(
          11.0, GeochemistryUnitConverter::GeochemistryUnit::MG_PER_KG_SOLVENT, "CO2(aq)", mgd),
      11.0 * 44.0098 * 1E3,
      1.0E-8);
  EXPECT_NEAR(
      GeochemistryUnitConverter::fromMoles(
          11.0, GeochemistryUnitConverter::GeochemistryUnit::UG_PER_KG_SOLVENT, "Calcite", mgd),
      11.0 * 100.0892 * 1E6,
      1.0E-3);
  EXPECT_NEAR(GeochemistryUnitConverter::fromMoles(
                  11.0,
                  GeochemistryUnitConverter::GeochemistryUnit::UG_PER_KG_SOLVENT,
                  "Fe(OH)3(ppd)",
                  mgd),
              11.0 * 106.8689 * 1E6,
              1.0E-3);
  EXPECT_NEAR(
      GeochemistryUnitConverter::fromMoles(
          11.0, GeochemistryUnitConverter::GeochemistryUnit::G_PER_KG_SOLVENT, "CH4(g)", mgd),
      11.0 * 16.0426,
      1.0E-8);
  EXPECT_NEAR(
      GeochemistryUnitConverter::fromMoles(
          11.0, GeochemistryUnitConverter::GeochemistryUnit::MG_PER_KG_SOLVENT, "Something", mgd),
      11.0 * 88.8537 * 1E3,
      1.0E-8);
  EXPECT_NEAR(GeochemistryUnitConverter::fromMoles(
                  11.0, GeochemistryUnitConverter::GeochemistryUnit::UG, "(O-phth)--", mgd),
              11.0 * 164.1172 * 1E6,
              1.0E-3);
  EXPECT_NEAR(GeochemistryUnitConverter::fromMoles(
                  11.0, GeochemistryUnitConverter::GeochemistryUnit::UG, ">(s)FeO-", mgd),
              11.0 * 71.8464 * 1E6,
              1.0E-3);
  EXPECT_NEAR(GeochemistryUnitConverter::fromMoles(
                  11.0, GeochemistryUnitConverter::GeochemistryUnit::CM3, "Calcite", mgd),
              11.0 * 36.9340,
              1.0E-8);
  EXPECT_NEAR(GeochemistryUnitConverter::fromMoles(
                  11.0, GeochemistryUnitConverter::GeochemistryUnit::CM3, "Fe(OH)3(ppd)", mgd),
              11.0 * 34.3200,
              1.0E-8);
  EXPECT_NEAR(GeochemistryUnitConverter::fromMoles(
                  11.0, GeochemistryUnitConverter::GeochemistryUnit::CM3, "Something", mgd),
              11.0 * 20.8200,
              1.0E-8);
}

// Test toMoles
TEST(GeochemistryUnitConverterTest, toMoles)
{
  EXPECT_EQ(GeochemistryUnitConverter::toMoles(
                11.0, GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS, "H2O", mgd),
            11.0);
  EXPECT_EQ(GeochemistryUnitConverter::toMoles(
                11.0, GeochemistryUnitConverter::GeochemistryUnit::MOLES, "CO3--", mgd),
            11.0);
  EXPECT_EQ(GeochemistryUnitConverter::toMoles(
                11.0, GeochemistryUnitConverter::GeochemistryUnit::MOLAL, "CaCO3", mgd),
            11.0);
  EXPECT_NEAR(GeochemistryUnitConverter::toMoles(
                  11.0 * 40.08, GeochemistryUnitConverter::GeochemistryUnit::G, "Ca++", mgd),
              11.0,
              1.0E-8);
  EXPECT_NEAR(
      GeochemistryUnitConverter::toMoles(
          11.0 * 40.08 / 1.0E3, GeochemistryUnitConverter::GeochemistryUnit::KG, "Ca++", mgd),
      11.0,
      1.0E-8);
  EXPECT_NEAR(
      GeochemistryUnitConverter::toMoles(
          11.0 * 61.0171 * 1.0E3, GeochemistryUnitConverter::GeochemistryUnit::MG, "HCO3-", mgd),
      11.0,
      1.0E-8);
  EXPECT_NEAR(
      GeochemistryUnitConverter::toMoles(
          11.0 * 1.0079 * 1.0E6, GeochemistryUnitConverter::GeochemistryUnit::UG, "H+", mgd),
      11.0,
      1.0E-3);
  EXPECT_NEAR(GeochemistryUnitConverter::toMoles(
                  11.0 * 72.8543 * 1E-3,
                  GeochemistryUnitConverter::GeochemistryUnit::KG_PER_KG_SOLVENT,
                  ">(s)FeOH",
                  mgd),
              11.0,
              1.0E-8);
  EXPECT_NEAR(GeochemistryUnitConverter::toMoles(
                  11.0 * 1234.567,
                  GeochemistryUnitConverter::GeochemistryUnit::G_PER_KG_SOLVENT,
                  ">(w)FeOH",
                  mgd),
              11.0,
              1.0E-8);
  EXPECT_NEAR(GeochemistryUnitConverter::toMoles(
                  11.0 * 44.0098 * 1E3,
                  GeochemistryUnitConverter::GeochemistryUnit::MG_PER_KG_SOLVENT,
                  "CO2(aq)",
                  mgd),
              11.0,
              1.0E-8);
  EXPECT_NEAR(GeochemistryUnitConverter::toMoles(
                  11.0 * 100.0892 * 1E6,
                  GeochemistryUnitConverter::GeochemistryUnit::UG_PER_KG_SOLVENT,
                  "Calcite",
                  mgd),
              11.0,
              1.0E-3);
  EXPECT_NEAR(GeochemistryUnitConverter::toMoles(
                  11.0 * 106.8689 * 1E6,
                  GeochemistryUnitConverter::GeochemistryUnit::UG_PER_KG_SOLVENT,
                  "Fe(OH)3(ppd)",
                  mgd),
              11.0,
              1.0E-3);
  EXPECT_NEAR(GeochemistryUnitConverter::toMoles(
                  11.0 * 16.0426,
                  GeochemistryUnitConverter::GeochemistryUnit::G_PER_KG_SOLVENT,
                  "CH4(g)",
                  mgd),
              11.0,
              1.0E-8);
  EXPECT_NEAR(GeochemistryUnitConverter::toMoles(
                  11.0 * 88.8537 * 1E3,
                  GeochemistryUnitConverter::GeochemistryUnit::MG_PER_KG_SOLVENT,
                  "Something",
                  mgd),
              11.0,
              1.0E-8);
  EXPECT_NEAR(GeochemistryUnitConverter::toMoles(11.0 * 164.1172 * 1E6,
                                                 GeochemistryUnitConverter::GeochemistryUnit::UG,
                                                 "(O-phth)--",
                                                 mgd),
              11.0,
              1.0E-3);
  EXPECT_NEAR(
      GeochemistryUnitConverter::toMoles(
          11.0 * 71.8464 * 1E6, GeochemistryUnitConverter::GeochemistryUnit::UG, ">(s)FeO-", mgd),
      11.0,
      1.0E-3);
  EXPECT_NEAR(GeochemistryUnitConverter::toMoles(
                  11.0 * 36.9340, GeochemistryUnitConverter::GeochemistryUnit::CM3, "Calcite", mgd),
              11.0,
              1.0E-8);
  EXPECT_NEAR(
      GeochemistryUnitConverter::toMoles(
          11.0 * 34.3200, GeochemistryUnitConverter::GeochemistryUnit::CM3, "Fe(OH)3(ppd)", mgd),
      11.0,
      1.0E-8);
  EXPECT_NEAR(
      GeochemistryUnitConverter::toMoles(
          11.0 * 20.8200, GeochemistryUnitConverter::GeochemistryUnit::CM3, "Something", mgd),
      11.0,
      1.0E-8);
}
