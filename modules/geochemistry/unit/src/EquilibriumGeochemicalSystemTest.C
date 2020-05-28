//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "EquilibriumGeochemicalSystem.h"

const GeochemicalDatabaseReader database_good_calcite("database/moose_testdb.json");
const PertinentGeochemicalSystem model_good_calcite(database_good_calcite,
                                                    {"H2O", "H+", "HCO3-", "Ca++"},
                                                    {"Calcite"},
                                                    {},
                                                    {},
                                                    {},
                                                    {},
                                                    "O2(aq)",
                                                    "e-");
ModelGeochemicalDatabase mgd_good_calcite = model_good_calcite.modelGeochemicalDatabase();
GeochemistrySpeciesSwapper swapper3(3, 1E-6);
GeochemistrySpeciesSwapper swapper4(4, 1E-6);
GeochemistrySpeciesSwapper swapper5(5, 1E-6);
GeochemistrySpeciesSwapper swapper6(6, 1E-6);
GeochemistrySpeciesSwapper swapper7(7, 1E-6);
GeochemistrySpeciesSwapper swapper8(8, 1E-6);
const std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm_good_calcite = {
    EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY,
    EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
    EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
    EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY};
GeochemistryIonicStrength is3(3.0, 3.0, false);
GeochemistryActivityCoefficientsDebyeHuckel ac3(is3);
GeochemistryIonicStrength is0(0.0, 0.0, false);
GeochemistryActivityCoefficientsDebyeHuckel ac0(is0);
GeochemistryIonicStrength is8(1E-8, 1E-8, false);
GeochemistryActivityCoefficientsDebyeHuckel ac8(is8);
const EquilibriumGeochemicalSystem egs_good_calcite(mgd_good_calcite,
                                                    ac3,
                                                    is3,
                                                    swapper4,
                                                    {"Ca++"},
                                                    {"Calcite"},
                                                    "H+",
                                                    {"H2O", "Calcite", "H+", "HCO3-"},
                                                    {1.75, 3.0, 2.0, 1.0},
                                                    cm_good_calcite,
                                                    25,
                                                    0,
                                                    1E-20);
const std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> dummy = {};
const PertinentGeochemicalSystem model_redox(
    database_good_calcite,
    {"H2O", "H+", "HCO3-", "O2(aq)", "(O-phth)--", "CH4(aq)", "StoiCheckRedox", "Fe+++"},
    {"Fe(OH)3(ppd)fake"},
    {"CH4(g)fake", "O2(g)"},
    {},
    {},
    {},
    "O2(aq)",
    "e-");
ModelGeochemicalDatabase mgd_redox = model_redox.modelGeochemicalDatabase();
const std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm_redox = {
    EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY,
    EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY,
    EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY,
    EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY,
    EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY,
    EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY,
    EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY,
    EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES};
const EquilibriumGeochemicalSystem
    egs_redox(mgd_redox,
              ac3,
              is3,
              swapper8,
              {},
              {},
              "Fe+++",
              {"H2O", "H+", "HCO3-", "O2(aq)", "(O-phth)--", "CH4(aq)", "StoiCheckRedox", "Fe+++"},
              {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0},
              cm_redox,
              25,
              0,
              1E-20);

/// Check MultiMooseEnum constructor
TEST(EquilibiumGeochemicalSystemTest, constructWithMultiMooseEnum)
{
  MultiMooseEnum constraint_meaning("moles_bulk_water kg_solvent_water moles_bulk_species "
                                    "free_molality free_moles_mineral_species fugacity activity");
  constraint_meaning.push_back("activity moles_bulk_species moles_bulk_species free_molality");
  ModelGeochemicalDatabase mgd_good_calcite2 = model_good_calcite.modelGeochemicalDatabase();
  const EquilibriumGeochemicalSystem egs(mgd_good_calcite2,
                                         ac3,
                                         is3,
                                         swapper4,
                                         {"Ca++"},
                                         {"Calcite"},
                                         "H+",
                                         {"H2O", "Calcite", "H+", "HCO3-"},
                                         {1.75, 3.0, 2.0, 1.0},
                                         constraint_meaning,
                                         25,
                                         0,
                                         1E-20);
}

/// Check num in basis
TEST(EquilibiumGeochemicalSystemTest, numInBasis)
{
  EXPECT_EQ(egs_good_calcite.getNumInBasis(), 4);
}

/// Check num in equilibrium
TEST(EquilibiumGeochemicalSystemTest, numInEquilibrium)
{
  EXPECT_EQ(egs_good_calcite.getNumInEquilibrium(), 6);
}

/// Check no kinetic exception
TEST(EquilibiumGeochemicalSystemTest, kineticException)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe++", "HCO3-", "O2(aq)"},
                                   {},
                                   {},
                                   {"Fe(OH)3(ppd)"},
                                   {},
                                   {},
                                   "O2(aq)",
                                   "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  try
  {
    EquilibriumGeochemicalSystem egs(
        mgd, ac3, is3, swapper7, {}, {}, "H+", {}, {}, dummy, 25, 0, 1E-20);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find(
            "Equilibrium geochemical systems cannot use models that include kinetic species") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check swaps have correct size
TEST(EquilibiumGeochemicalSystemTest, swapSizeException)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe++", "HCO3-", "O2(aq)"},
                                   {},
                                   {},
                                   {},
                                   {},
                                   {},
                                   "O2(aq)",
                                   "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  try
  {
    EquilibriumGeochemicalSystem egs(
        mgd, ac3, is3, swapper7, {"H+"}, {}, "H+", {}, {}, dummy, 25, 0, 1E-20);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("swap_out_of_basis must have same length as swap_into_basis") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check swaps is not swapping charge-balance species out
TEST(EquilibiumGeochemicalSystemTest, swapChargeBalanceException)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe++", "HCO3-", "O2(aq)"},
                                   {},
                                   {},
                                   {},
                                   {},
                                   {},
                                   "O2(aq)",
                                   "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  try
  {
    EquilibriumGeochemicalSystem egs(
        mgd, ac3, is3, swapper7, {"H+"}, {"FeOH"}, "H+", {}, {}, dummy, 25, 0, 1E-20);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Cannot swap out H+ because it is the charge-balance species") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}
/// Check illegal swap produces an error
TEST(EquilibiumGeochemicalSystemTest, swapException)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe++", "HCO3-", "O2(aq)"},
                                   {},
                                   {},
                                   {},
                                   {},
                                   {},
                                   "O2(aq)",
                                   "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  try
  {
    EquilibriumGeochemicalSystem egs(
        mgd, ac3, is3, swapper7, {"CO2(aq)"}, {"CO2(aq)"}, "H+", {}, {}, dummy, 25, 0, 1E-20);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("CO2(aq) is not in the basis, so cannot be removed "
                         "from the basis") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check constraint sizes
TEST(EquilibiumGeochemicalSystemTest, constraintSizeExceptions)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm;
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_WATER);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY);

  std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm_poor;
  cm_poor.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_WATER);
  cm_poor.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES);

  try
  {
    EquilibriumGeochemicalSystem egs(mgd,
                                     ac3,
                                     is3,
                                     swapper3,
                                     {},
                                     {},
                                     "H+",
                                     {"H2O", "H+", "HCO3-"},
                                     {1.0, 2.0},
                                     cm,
                                     25,
                                     0,
                                     1E-20);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Constrained species names must have same length as constraint values") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    EquilibriumGeochemicalSystem egs(mgd,
                                     ac3,
                                     is3,
                                     swapper3,
                                     {},
                                     {},
                                     "H+",
                                     {"H2O", "H+", "HCO3-"},
                                     {1.0, 2.0, 3.0},
                                     cm_poor,
                                     25,
                                     0,
                                     1E-20);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("Constrained species names must have same length as constraint meanings") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    EquilibriumGeochemicalSystem egs(
        mgd, ac3, is3, swapper3, {}, {}, "H+", {"H2O", "H+"}, {1.0, 2.0}, cm_poor, 25, 0, 1E-20);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("Constrained species names must have same length as the number of species in the "
                 "basis (each component must be provided with a single constraint") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}
/// Check constraint sizes
TEST(EquilibiumGeochemicalSystemTest, constraintNameExceptions)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm;
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_WATER);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY);

  try
  {
    EquilibriumGeochemicalSystem egs(mgd,
                                     ac3,
                                     is3,
                                     swapper3,
                                     {},
                                     {},
                                     "H+",
                                     {"H2O", "H+", "H20"},
                                     {1.0, 2.0, 3.0},
                                     cm,
                                     25,
                                     0,
                                     1E-20);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("The basis species HCO3- must appear in the constrained species list") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    EquilibriumGeochemicalSystem egs(mgd,
                                     ac3,
                                     is3,
                                     swapper3,
                                     {"HCO3-"},
                                     {"CO2(aq)"},
                                     "H+",
                                     {"H2O", "H+", "HCO3-"},
                                     {1.0, 2.0, 3.0},
                                     cm,
                                     25,
                                     0,
                                     1E-20);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("The basis species CO2(aq) must appear in the constrained species list") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check appropriate positivity
TEST(EquilibiumGeochemicalSystemTest, positiveException1)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm;
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::KG_SOLVENT_WATER);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FUGACITY);

  try
  {
    EquilibriumGeochemicalSystem egs(mgd,
                                     ac3,
                                     is3,
                                     swapper3,
                                     {"HCO3-"},
                                     {"CO2(aq)"},
                                     "H+",
                                     {"H2O", "H+", "CO2(aq)"},
                                     {-1.0, 2.0, 3.0},
                                     cm,
                                     25,
                                     0,
                                     1E-20);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Specified mass of solvent water must be positive: you entered -1") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}
/// Check appropriate positivity
TEST(EquilibiumGeochemicalSystemTest, positiveException2)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm;
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY);

  try
  {
    EquilibriumGeochemicalSystem egs(mgd,
                                     ac3,
                                     is3,
                                     swapper3,
                                     {"HCO3-"},
                                     {"CO2(aq)"},
                                     "H+",
                                     {"H2O", "H+", "CO2(aq)"},
                                     {-1.0, 2.0, 3.0},
                                     cm,
                                     25,
                                     0,
                                     1E-20);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Specified activity values must be positive: you entered -1") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check appropriate positivity
TEST(EquilibiumGeochemicalSystemTest, positiveException3)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-", "O2(aq)"}, {}, {"O2(g)"}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm;
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FUGACITY);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES);

  try
  {
    EquilibriumGeochemicalSystem egs(mgd,
                                     ac3,
                                     is3,
                                     swapper4,
                                     {"O2(aq)"},
                                     {"O2(g)"},
                                     "H+",
                                     {"H2O", "H+", "O2(g)", "HCO3-"},
                                     {1.0, 2.0, -3.0, 4.0},
                                     cm,
                                     25,
                                     0,
                                     1E-20);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Specified fugacity values must be positive: you entered -3") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check appropriate positivity
TEST(EquilibiumGeochemicalSystemTest, positiveException4)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm;
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY);

  try
  {
    EquilibriumGeochemicalSystem egs(mgd,
                                     ac3,
                                     is3,
                                     swapper3,
                                     {"HCO3-"},
                                     {"CO2(aq)"},
                                     "H+",
                                     {"H2O", "H+", "CO2(aq)"},
                                     {1.0, 2.0, -2.0},
                                     cm,
                                     25,
                                     0,
                                     1E-20);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Specified free molality values must be positive: you entered -2") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check appropriate positivity
TEST(EquilibiumGeochemicalSystemTest, positiveException5)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-", "Ca++"}, {"Calcite"}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm;
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLES_MINERAL_SPECIES);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY);

  try
  {
    EquilibriumGeochemicalSystem egs(mgd,
                                     ac3,
                                     is3,
                                     swapper4,
                                     {"Ca++"},
                                     {"Calcite"},
                                     "H+",
                                     {"H2O", "Calcite", "H+", "HCO3-"},
                                     {1.0, -3.0, 2.0, 1.0},
                                     cm,
                                     25,
                                     0,
                                     1E-20);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find(
            "Specified free mole number of mineral species must be positive: you entered -3") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check water provided with appropriate value
TEST(EquilibiumGeochemicalSystemTest, waterException)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm;
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY);

  try
  {
    EquilibriumGeochemicalSystem egs(mgd,
                                     ac3,
                                     is3,
                                     swapper3,
                                     {"HCO3-"},
                                     {"CO2(aq)"},
                                     "H+",
                                     {"H2O", "H+", "CO2(aq)"},
                                     {1.0, 2.0, 3.0},
                                     cm,
                                     25,
                                     0,
                                     1E-20);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("H2O must be provided with either a mass of solvent water, a bulk number of "
                 "moles, or an activity") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check gases are provided with appropriate value
TEST(EquilibiumGeochemicalSystemTest, fugacityException)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-", "O2(aq)"}, {}, {"O2(g)"}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm;
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_WATER);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY);

  try
  {
    EquilibriumGeochemicalSystem egs(mgd,
                                     ac3,
                                     is3,
                                     swapper4,
                                     {"O2(aq)"},
                                     {"O2(g)"},
                                     "H+",
                                     {"H2O", "H+", "O2(g)", "HCO3-"},
                                     {1.0, 2.0, 3.0, 4.0},
                                     cm,
                                     25,
                                     0,
                                     1E-20);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("The gas O2(g) must be provided with a fugacity") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check minerals are provided with appropriate meaning
TEST(EquilibiumGeochemicalSystemTest, mineralMeaningExecption)
{
  std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm;
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY);

  try
  {
    EquilibriumGeochemicalSystem egs(mgd_good_calcite,
                                     ac3,
                                     is3,
                                     swapper4,
                                     {},
                                     {},
                                     "H+",
                                     {"H2O", "Calcite", "H+", "HCO3-"},
                                     {1.0, 3.0, 2.0, 1.0},
                                     cm,
                                     25,
                                     0,
                                     1E-20);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("The mineral Calcite must be provided with a free number of moles or a "
                         "bulk number of moles") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm2;
  cm2.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY);
  cm2.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY);
  cm2.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES);
  cm2.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY);

  try
  {
    EquilibriumGeochemicalSystem egs(mgd_good_calcite,
                                     ac3,
                                     is3,
                                     swapper4,
                                     {},
                                     {},
                                     "H+",
                                     {"H2O", "Calcite", "H+", "HCO3-"},
                                     {1.0, 3.0, 2.0, 1.0},
                                     cm2,
                                     25,
                                     0,
                                     1E-20);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("The mineral Calcite must be provided with a free number of moles or a "
                         "bulk number of moles") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check aqueous species are provided with appropriate meaning
TEST(EquilibiumGeochemicalSystemTest, aqueousSpeciesMeaningExecption)
{
  std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm;
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLES_MINERAL_SPECIES);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES);

  try
  {
    EquilibriumGeochemicalSystem egs(mgd_good_calcite,
                                     ac3,
                                     is3,
                                     swapper4,
                                     {},
                                     {},
                                     "HCO3-",
                                     {"H2O", "Calcite", "H+", "HCO3-"},
                                     {1.0, 3.0, 2.0, 1.0},
                                     cm,
                                     25,
                                     0,
                                     1E-20);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("The basis species H+ must be provided with a free molality, bulk number "
                         "of moles, or an activity") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm2;
  cm2.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY);
  cm2.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES);
  cm2.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES);
  cm2.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FUGACITY);

  try
  {
    EquilibriumGeochemicalSystem egs(mgd_good_calcite,
                                     ac3,
                                     is3,
                                     swapper4,
                                     {},
                                     {},
                                     "H+",
                                     {"H2O", "Calcite", "H+", "HCO3-"},
                                     {1.0, 3.0, 2.0, 1.0},
                                     cm2,
                                     25,
                                     0,
                                     1E-20);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("The basis species HCO3- must be provided with a free molality, bulk number "
                 "of moles, or an activity") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check charge-balance species is provided with appropriate meaning
TEST(EquilibiumGeochemicalSystemTest, chargeBalanceMeaningExecption)
{
  std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm;
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY);

  try
  {
    EquilibriumGeochemicalSystem egs(mgd_good_calcite,
                                     ac3,
                                     is3,
                                     swapper4,
                                     {},
                                     {},
                                     "H+",
                                     {"H2O", "Calcite", "H+", "HCO3-"},
                                     {1.0, 3.0, 2.0, 1.0},
                                     cm,
                                     25,
                                     0,
                                     1E-20);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find(
            "For code consistency, the species H+ must be provided with a bulk number of moles "
            "because it is the charge-balance species.  The value provided should be a reasonable "
            "estimate of the mole number, but will be overridden as the solve progresses") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm2;
  cm2.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY);
  cm2.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES);
  cm2.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY);
  cm2.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY);

  try
  {
    EquilibriumGeochemicalSystem egs(mgd_good_calcite,
                                     ac3,
                                     is3,
                                     swapper4,
                                     {},
                                     {},
                                     "H+",
                                     {"H2O", "Calcite", "H+", "HCO3-"},
                                     {1.0, 3.0, 2.0, 1.0},
                                     cm2,
                                     25,
                                     0,
                                     1E-20);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find(
            "For code consistency, the species H+ must be provided with a bulk number of moles "
            "because it is the charge-balance species.  The value provided should be a reasonable "
            "estimate of the mole number, but will be overridden as the solve progresses") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check charge-balance species is in the basis
TEST(EquilibiumGeochemicalSystemTest, chargeBalanceInBasisExecption)
{
  try
  {
    EquilibriumGeochemicalSystem egs(mgd_good_calcite,
                                     ac3,
                                     is3,
                                     swapper4,
                                     {},
                                     {},
                                     "OH-",
                                     {"H2O", "Calcite", "H+", "HCO3-"},
                                     {1.0, 3.0, 2.0, 1.0},
                                     cm_good_calcite,
                                     25,
                                     0,
                                     1E-20);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("Cannot enforce charge balance using OH- because it is not in the basis") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check charge-balance species has a charge
TEST(EquilibiumGeochemicalSystemTest, chargeBalanceHasChargeExecption)
{
  try
  {
    EquilibriumGeochemicalSystem egs(mgd_good_calcite,
                                     ac3,
                                     is3,
                                     swapper4,
                                     {},
                                     {},
                                     "Calcite",
                                     {"H2O", "Calcite", "H+", "HCO3-"},
                                     {1.0, 3.0, 2.0, 1.0},
                                     cm_good_calcite,
                                     25,
                                     0,
                                     1E-20);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("Cannot enforce charge balance using Calcite because it has zero charge") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check charge-balance species index
TEST(EquilibiumGeochemicalSystemTest, chargeBalanceIndex)
{
  EXPECT_EQ(egs_good_calcite.getChargeBalanceBasisIndex(), 1);
}

/// Check getLog10K
TEST(EquilibiumGeochemicalSystemTest, getLog10K)
{
  try
  {
    EXPECT_EQ(egs_good_calcite.getLog10K(123), 1);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Cannot retrieve log10K for equilibrium species 123 since there are only "
                         "6 equilibrium species") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
  EXPECT_EQ(egs_good_calcite.getLog10K(0), -6.3660); // CO2(aq) at 25degC
}

/// Check getInAlgebraicSystem
TEST(EquilibiumGeochemicalSystemTest, getInAlgebraicSystem)
{
  const std::vector<bool> as_gold = {false, true, false, false};
  EXPECT_EQ(egs_good_calcite.getInAlgebraicSystem().size(), 4);
  for (unsigned i = 0; i < as_gold.size(); ++i)
    EXPECT_EQ(egs_good_calcite.getInAlgebraicSystem()[i], as_gold[i]);
}

/// Check NumInAlgebraicSystem
TEST(EquilibiumGeochemicalSystemTest, NumInAlgebraicSystem)
{
  EXPECT_EQ(egs_good_calcite.getNumInAlgebraicSystem(), 1);
  EXPECT_EQ(egs_good_calcite.getNumBasisInAlgebraicSystem(), 1);
  EXPECT_EQ(egs_good_calcite.getNumSurfacePotentials(), 0);
}

/// Check getBasisIndexOfAlgebraicSystem()
TEST(EquilibiumGeochemicalSystemTest, getBasisIndexOfAlgebraicSystem)
{
  EXPECT_EQ(egs_good_calcite.getBasisIndexOfAlgebraicSystem()[0], 1);
}

/// Check getAlgebraicIndexOfBasisSystem()
TEST(EquilibiumGeochemicalSystemTest, getAlgebraicIndexOfBasisSystem)
{
  EXPECT_EQ(egs_good_calcite.getAlgebraicIndexOfBasisSystem()[1], 0);
}

/// Check getnw
TEST(EquilibiumGeochemicalSystemTest, getSolventWaterMass)
{
  EXPECT_EQ(egs_good_calcite.getSolventWaterMass(), 1.0);

  const std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm = {
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::KG_SOLVENT_WATER,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY};
  const EquilibriumGeochemicalSystem egs(mgd_good_calcite,
                                         ac3,
                                         is3,
                                         swapper4,
                                         {},
                                         {},
                                         "H+",
                                         {"H2O", "Calcite", "H+", "HCO3-"},
                                         {2.5, 3.0, 2.0, 1.0},
                                         cm,
                                         25,
                                         0,
                                         1E-20);
  EXPECT_EQ(egs.getSolventWaterMass(), 2.5);
}

/// Check getBulkMoles
TEST(EquilibiumGeochemicalSystemTest, getBulkMoles)
{
  EXPECT_EQ(egs_good_calcite.getBulkMoles()[3], 3.0); // Calcite
  EXPECT_EQ(egs_good_calcite.getBulkMoles()[1], 2.0); // H+

  const std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm = {
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_WATER,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES};
  const EquilibriumGeochemicalSystem egs(mgd_good_calcite,
                                         ac3,
                                         is3,
                                         swapper4,
                                         {},
                                         {},
                                         "H+",
                                         {"H2O", "Calcite", "H+", "HCO3-"},
                                         {-0.5, 3.5, 2.5, 1.0},
                                         cm,
                                         25,
                                         0,
                                         1E-20);
  EXPECT_EQ(egs.getBulkMoles()[0], -0.5); // H2O
  EXPECT_EQ(egs.getBulkMoles()[1], 1.0);  // H+ : note that charge neutrality has forced this
  EXPECT_EQ(egs.getBulkMoles()[2], 1.0);  // HCO3-
  EXPECT_EQ(egs.getBulkMoles()[3], 3.5);  // Calcite
}

/// Check getSolventMassAndFreeMolalityAndMineralMoles and getAlgebraicVariableValues
TEST(EquilibiumGeochemicalSystemTest, getSolventMassAndFreeMolalityAndMineralMoles)
{
  EXPECT_EQ(egs_good_calcite.getSolventMassAndFreeMolalityAndMineralMoles()[2], 1.0); // HCO3-
  EXPECT_EQ(egs_good_calcite.getAlgebraicVariableValues()[0],
            egs_good_calcite.getSolventMassAndFreeMolalityAndMineralMoles()[1]); // H+
  EXPECT_EQ(egs_good_calcite.getAlgebraicBasisValues()[0],
            egs_good_calcite.getSolventMassAndFreeMolalityAndMineralMoles()[1]); // H+
  EXPECT_EQ(egs_good_calcite.getAlgebraicVariableDenseValues()(0),
            egs_good_calcite.getSolventMassAndFreeMolalityAndMineralMoles()[1]); // H+

  const std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm = {
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::KG_SOLVENT_WATER,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLES_MINERAL_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY};
  const EquilibriumGeochemicalSystem egs(mgd_good_calcite,
                                         ac3,
                                         is3,
                                         swapper4,
                                         {},
                                         {},
                                         "H+",
                                         {"H2O", "Calcite", "H+", "HCO3-"},
                                         {0.5, 3.5, 2.5, 1.0},
                                         cm,
                                         25,
                                         0,
                                         1E-20);
  EXPECT_EQ(egs.getSolventMassAndFreeMolalityAndMineralMoles()[0], 0.5); // H2O
  EXPECT_EQ(egs.getSolventMassAndFreeMolalityAndMineralMoles()[2], 1.0); // HCO3-
  EXPECT_EQ(egs.getSolventMassAndFreeMolalityAndMineralMoles()[3], 3.5); // Calcite
  EXPECT_EQ(egs_good_calcite.getAlgebraicVariableValues()[0],
            egs_good_calcite.getSolventMassAndFreeMolalityAndMineralMoles()[1]); // H+
  EXPECT_EQ(egs_good_calcite.getAlgebraicBasisValues()[0],
            egs_good_calcite.getSolventMassAndFreeMolalityAndMineralMoles()[1]); // H+
  EXPECT_EQ(egs_good_calcite.getAlgebraicVariableDenseValues()(0),
            egs_good_calcite.getSolventMassAndFreeMolalityAndMineralMoles()[1]); // H+
}

/// Check getBasisActivityKnown
TEST(EquilibiumGeochemicalSystemTest, getBasisActivityKnown)
{
  EXPECT_EQ(egs_good_calcite.getBasisActivityKnown()[0], true);  // H2O
  EXPECT_EQ(egs_good_calcite.getBasisActivityKnown()[1], false); // H+
  EXPECT_EQ(egs_good_calcite.getBasisActivityKnown()[2], false); // HCO3-
  EXPECT_EQ(egs_good_calcite.getBasisActivityKnown()[3], true);  // Calcite

  GeochemicalDatabaseReader database("database/moose_testdb.json");
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-", "O2(aq)"}, {}, {"O2(g)"}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm;
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_WATER);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FUGACITY);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY);

  EquilibriumGeochemicalSystem egs(mgd,
                                   ac3,
                                   is3,
                                   swapper4,
                                   {"O2(aq)"},
                                   {"O2(g)"},
                                   "H+",
                                   {"H2O", "H+", "O2(g)", "HCO3-"},
                                   {1.0, 2.0, 3.0, 4.0},
                                   cm,
                                   25,
                                   0,
                                   1E-20);

  EXPECT_EQ(egs.getBasisActivityKnown()[0], false); // H2O
  EXPECT_EQ(egs.getBasisActivityKnown()[1], false); // H+
  EXPECT_EQ(egs.getBasisActivityKnown()[2], false); // HCO3-
  EXPECT_EQ(egs.getBasisActivityKnown()[3], true);  // O2(g)
}

/// Check getBasisActivity
TEST(EquilibiumGeochemicalSystemTest, getBasisActivity)
{
  EXPECT_EQ(egs_good_calcite.getBasisActivity(0), 1.75);   // H2O
  EXPECT_EQ(egs_good_calcite.getBasisActivity(3), 1.0);    // Calcite
  EXPECT_EQ(egs_good_calcite.getBasisActivity()[0], 1.75); // H2O
  EXPECT_EQ(egs_good_calcite.getBasisActivity()[3], 1.0);  // Calcite
  try
  {
    EXPECT_EQ(egs_good_calcite.getBasisActivity(4), 1);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Cannot retrieve basis activity for species 4 since there are only 4 "
                         "basis species") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  GeochemicalDatabaseReader database("database/moose_testdb.json");
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-", "O2(aq)"}, {}, {"O2(g)"}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm;
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_WATER);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FUGACITY);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY);

  EquilibriumGeochemicalSystem egs(
      mgd,
      ac0,
      is0,
      swapper4,
      {"O2(aq)"},
      {"O2(g)"},
      "H+",
      {"H2O", "H+", "O2(g)", "HCO3-"},
      {1.0, 1.5, 3.0, 2.5},
      cm,
      25,
      0,
      1E-20); // maximum ionic_strength = 0, so all activity_coefficients = 1

  EXPECT_EQ(egs.getBasisActivity(1), egs.getSolventMassAndFreeMolalityAndMineralMoles()[1]); // H+
  EXPECT_EQ(egs.getBasisActivity(2), 2.5);   // HCO3-
  EXPECT_EQ(egs.getBasisActivity(3), 3.0);   // O2(g)
  EXPECT_EQ(egs.getBasisActivity()[2], 2.5); // HCO3-
  EXPECT_EQ(egs.getBasisActivity()[3], 3.0); // O2(g)
}

/// Check getBasisActivityCoeff
TEST(EquilibiumGeochemicalSystemTest, getBasisActivityCoeff)
{
  try
  {
    EXPECT_EQ(egs_good_calcite.getBasisActivityCoefficient(4), 1);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("Cannot retrieve basis activity coefficient for species 4 since there are only 4 "
                 "basis species") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  GeochemicalDatabaseReader database("database/moose_testdb.json");
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-", "O2(aq)"}, {}, {"O2(g)"}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm;
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_WATER);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FUGACITY);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY);

  EquilibriumGeochemicalSystem egs(
      mgd,
      ac0,
      is0,
      swapper4,
      {"O2(aq)"},
      {"O2(g)"},
      "H+",
      {"H2O", "H+", "O2(g)", "HCO3-"},
      {1.0, 1.5, 3.0, 2.5},
      cm,
      25,
      0,
      1E-20); // maximum ionic_strength = 0, so all activity_coefficients = 1

  EXPECT_EQ(egs.getBasisActivityCoefficient(1), 1.0);   // H+
  EXPECT_EQ(egs.getBasisActivityCoefficient(2), 1.0);   // HCO3-
  EXPECT_EQ(egs.getBasisActivityCoefficient()[1], 1.0); // H+
  EXPECT_EQ(egs.getBasisActivityCoefficient()[2], 1.0); // HCO3-
}

/// Check getEqmActivityCoefficient exception
TEST(EquilibiumGeochemicalSystemTest, getEqmActivityCoefficientException)
{
  try
  {
    EXPECT_EQ(egs_good_calcite.getEquilibriumActivityCoefficient(44), 1);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Cannot retrieve activity coefficient for equilibrium species 44 since "
                         "there are only 6 "
                         "equilibrium species") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check getEqmMolality
TEST(EquilibiumGeochemicalSystemTest, getEqmMolality)
{
  try
  {
    EXPECT_EQ(egs_good_calcite.getEquilibriumMolality(44), 1);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("Cannot retrieve molality for equilibrium species 44 since there are only 6 "
                 "equilibrium species") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  // CO2(aq) = -H2O + H+ + HCO3-
  EXPECT_NEAR(egs_good_calcite.getEquilibriumMolality(0) /
                  (egs_good_calcite.getBasisActivity(1) * egs_good_calcite.getBasisActivity(2) /
                   egs_good_calcite.getBasisActivity(0) /
                   egs_good_calcite.getEquilibriumActivityCoefficient(0) /
                   std::pow(10.0, egs_good_calcite.getLog10K(0))),
              1.0,
              1.0E-8);
  EXPECT_NEAR(egs_good_calcite.getEquilibriumMolality()[0] /
                  (egs_good_calcite.getBasisActivity(1) * egs_good_calcite.getBasisActivity(2) /
                   egs_good_calcite.getBasisActivity(0) /
                   egs_good_calcite.getEquilibriumActivityCoefficient()[0] /
                   std::pow(10.0, egs_good_calcite.getLog10K(0))),
              1.0,
              1.0E-8);
  // CO3-- = HCO3- - H+
  EXPECT_NEAR(egs_good_calcite.getEquilibriumMolality(1) /
                  (egs_good_calcite.getBasisActivity(2) / egs_good_calcite.getBasisActivity(1) /
                   egs_good_calcite.getEquilibriumActivityCoefficient(1) /
                   std::pow(10.0, egs_good_calcite.getLog10K(1))),
              1.0,
              1.0E-8);
  EXPECT_NEAR(egs_good_calcite.getEquilibriumMolality()[1] /
                  (egs_good_calcite.getBasisActivity(2) / egs_good_calcite.getBasisActivity(1) /
                   egs_good_calcite.getEquilibriumActivityCoefficient()[1] /
                   std::pow(10.0, egs_good_calcite.getLog10K(1))),
              1.0,
              1.0E-8);
  // CaCO3 = Calcite
  EXPECT_NEAR(egs_good_calcite.getEquilibriumMolality(2) /
                  (1.0 / egs_good_calcite.getEquilibriumActivityCoefficient(2) /
                   std::pow(10.0, egs_good_calcite.getLog10K(2))),
              1.0,
              1.0E-8);
  EXPECT_NEAR(egs_good_calcite.getEquilibriumMolality()[2] /
                  (1.0 / egs_good_calcite.getEquilibriumActivityCoefficient()[2] /
                   std::pow(10.0, egs_good_calcite.getLog10K(2))),
              1.0,
              1.0E-8);
  // CaOH+ = Calcite - HCO3- + H20
  EXPECT_NEAR(egs_good_calcite.getEquilibriumMolality(3) /
                  (egs_good_calcite.getBasisActivity(0) / egs_good_calcite.getBasisActivity(2) /
                   egs_good_calcite.getEquilibriumActivityCoefficient()[3] /
                   std::pow(10.0, egs_good_calcite.getLog10K(3))),
              1.0,
              1.0E-8);
  EXPECT_NEAR(egs_good_calcite.getEquilibriumMolality()[3] /
                  (egs_good_calcite.getBasisActivity(0) / egs_good_calcite.getBasisActivity(2) /
                   egs_good_calcite.getEquilibriumActivityCoefficient(3) /
                   std::pow(10.0, egs_good_calcite.getLog10K(3))),
              1.0,
              1.0E-8);
  // OH- = H20 - H+
  EXPECT_NEAR(egs_good_calcite.getEquilibriumMolality(4) /
                  (egs_good_calcite.getBasisActivity(0) / egs_good_calcite.getBasisActivity(1) /
                   egs_good_calcite.getEquilibriumActivityCoefficient(4) /
                   std::pow(10.0, egs_good_calcite.getLog10K(4))),
              1.0,
              1.0E-8);
  EXPECT_NEAR(egs_good_calcite.getEquilibriumMolality()[4] /
                  (egs_good_calcite.getBasisActivity(0) / egs_good_calcite.getBasisActivity(1) /
                   egs_good_calcite.getEquilibriumActivityCoefficient()[4] /
                   std::pow(10.0, egs_good_calcite.getLog10K(4))),
              1.0,
              1.0E-8);
  // Ca++ = Calcite - HCO3- + H+
  EXPECT_NEAR(egs_good_calcite.getEquilibriumMolality(5) /
                  (egs_good_calcite.getBasisActivity(1) / egs_good_calcite.getBasisActivity(2) /
                   egs_good_calcite.getEquilibriumActivityCoefficient(5) /
                   std::pow(10.0, egs_good_calcite.getLog10K(5))),
              1.0,
              1.0E-8);
  EXPECT_NEAR(egs_good_calcite.getEquilibriumMolality()[5] /
                  (egs_good_calcite.getBasisActivity(1) / egs_good_calcite.getBasisActivity(2) /
                   egs_good_calcite.getEquilibriumActivityCoefficient()[5] /
                   std::pow(10.0, egs_good_calcite.getLog10K(5))),
              1.0,
              1.0E-8);
}

/// Check getTotalCharge
TEST(EquilibiumGeochemicalSystemTest, getTotalCharge)
{
  const GeochemicalDatabaseReader database("database/moose_testdb.json");
  const PertinentGeochemicalSystem model(
      database, {"H2O", "StoiCheckBasis", "Fe+++"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();
  const std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm = {
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::KG_SOLVENT_WATER,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES};
  const EquilibriumGeochemicalSystem egs(mgd,
                                         ac3,
                                         is3,
                                         swapper3,
                                         {},
                                         {},
                                         "StoiCheckBasis",
                                         {"H2O", "Fe+++", "StoiCheckBasis"},
                                         {1.75, 5.0, 1.0},
                                         cm,
                                         25,
                                         0,
                                         1E-20);
  EXPECT_EQ(egs.getBulkMoles()[2], 5.0); // Fe+++
  EXPECT_NEAR(egs.getBulkMoles()[1],
              -6.0,
              1.0E-12); // StoiCheckBasis (charge=2.5) computed to ensure charge neutrality
  EXPECT_NEAR(egs.getTotalCharge(), 0.0, 1.0E-12);
}

/// Check getresidual
TEST(EquilibiumGeochemicalSystemTest, getResidual)
{
  const std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm = {
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_WATER,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY};
  const EquilibriumGeochemicalSystem egs(mgd_good_calcite,
                                         ac3,
                                         is3,
                                         swapper4,
                                         {},
                                         {},
                                         "HCO3-",
                                         {"H2O", "Calcite", "HCO3-", "H+"},
                                         {175.0, 3.0, 2.0, 1.0},
                                         cm,
                                         25,
                                         0,
                                         1E-20);
  // Here, water and HCO3- are the components in the algebraic system
  EXPECT_EQ(egs.getNumInAlgebraicSystem(), 2);
  EXPECT_EQ(egs.getNumBasisInAlgebraicSystem(), 2);
  EXPECT_EQ(egs.getNumSurfacePotentials(), 0);

  try
  {
    EXPECT_EQ(egs.getResidualComponent(3), 2);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Cannot retrieve residual for algebraic index 3 because there are only 2 "
                         "molalities in the algebraic system and 0 surface potentials") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  const Real nw = egs.getSolventMassAndFreeMolalityAndMineralMoles()[0];
  const Real mHCO3 = egs.getSolventMassAndFreeMolalityAndMineralMoles()[2];
  // H2O has algebraic index = 0 and basis index = 0
  Real res = -175.0 + nw * GeochemistryConstants::MOLES_PER_KG_WATER;
  for (unsigned j = 0; j < mgd_good_calcite.eqm_species_name.size(); ++j)
    res += nw * mgd_good_calcite.eqm_stoichiometry(j, 0) * egs.getEquilibriumMolality(j);
  EXPECT_NEAR(egs.getResidualComponent(0) / res, 1.0, 1.0E-12);
  // HCO3- has algebraic index = 1 and basis index = 2
  res = -egs.getBulkMoles()[1] +
        nw * mHCO3; // first term is because of charge balance (H+ has basis index 1)
  for (unsigned j = 0; j < mgd_good_calcite.eqm_species_name.size(); ++j)
    res += nw * mgd_good_calcite.eqm_stoichiometry(j, 2) * egs.getEquilibriumMolality(j);
  EXPECT_NEAR(egs.getResidualComponent(1) / res, 1.0, 1.0E-12);

  // The above is a good test of the indexing, but due to crazy log10K it is difficult to
  // demonstrate correct residual, so instead:
  PertinentGeochemicalSystem model2(
      database_good_calcite, {"H2O", "H+", "StoiCheckBasis"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd2 = model2.modelGeochemicalDatabase();
  const std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm2 = {
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_WATER,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES};
  const EquilibriumGeochemicalSystem egs2(mgd2,
                                          ac3,
                                          is3,
                                          swapper3,
                                          {},
                                          {},
                                          "StoiCheckBasis",
                                          {"H2O", "H+", "StoiCheckBasis"},
                                          {175.0, 8.0, 2.0},
                                          cm2,
                                          25,
                                          0,
                                          1E-20);
  // Here, water and StoiCheckBasis are the components in the algebraic system
  EXPECT_EQ(egs2.getNumInAlgebraicSystem(), 2);
  EXPECT_EQ(egs2.getNumBasisInAlgebraicSystem(), 2);
  EXPECT_EQ(egs2.getNumSurfacePotentials(), 0);
  // The equilibrium species are StoiCheckRedox (m = 9.82934e+06) and OH- (m = 4.78251e-15)
  const Real nw2 = egs2.getSolventMassAndFreeMolalityAndMineralMoles()[0];
  const Real mscb = egs2.getSolventMassAndFreeMolalityAndMineralMoles()[2];
  const Real mscr = egs2.getEquilibriumMolality(0);
  // H2O
  res = -175.0 + nw2 * (GeochemistryConstants::MOLES_PER_KG_WATER + (-0.5) * mscr);
  EXPECT_NEAR(egs2.getResidualComponent(0) / res, 1.0, 1.0E-12);
  // StoiCheckBasis (algebraic index = 1, basis index = 2)
  res = (1.0 / 2.5) * egs2.getBulkMoles()[1] +
        nw2 * (mscb + 1.5 * mscr); // first term is from charge balance
  EXPECT_NEAR(egs2.getResidualComponent(1) / res, 1.0, 1.0E-12);
}

/// check that  enforcing charge balance works
TEST(EquilibiumGeochemicalSystemTest, enforceChargeBalance)
{
  EquilibriumGeochemicalSystem nonconst = egs_good_calcite;
  ASSERT_TRUE(std::abs(nonconst.getTotalCharge()) > 1.0); // initially there is nonzero charge
  nonconst.enforceChargeBalance();
  EXPECT_NEAR(nonconst.getTotalCharge(), 0.0, 1E-12);
}

/// Setting algebraic variables exception
TEST(EquilibiumGeochemicalSystemTest, setVarException)
{
  EquilibriumGeochemicalSystem egs(mgd_good_calcite,
                                   ac3,
                                   is3,
                                   swapper4,
                                   {},
                                   {},
                                   "H+",
                                   {"H2O", "Calcite", "H+", "HCO3-"},
                                   {1.75, 3.0, 2.0, 1.0},
                                   cm_good_calcite,
                                   25,
                                   0,
                                   1E-20);

  DenseVector<Real> var_incorrect_size(123);
  try
  {
    egs.setAlgebraicVariables(var_incorrect_size);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Incorrect size in setAlgebraicVariables") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  DenseVector<Real> var_neg(egs.getNumInAlgebraicSystem());
  var_neg(0) = -1.25;
  try
  {
    egs.setAlgebraicVariables(var_neg);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Cannot set algebraic variables to non-positive values such as -1.25") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/**
 * Check jacobian calculations when mass of solvent water is a variable.
 * Note that because the Jacobian calculations do not consider derivatives of activity or
 * activity coefficients, they are only roughly correct
 */
TEST(EquilibiumGeochemicalSystemTest, jac1)
{
  const std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm = {
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_WATER,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY};
  EquilibriumGeochemicalSystem egs(
      mgd_good_calcite,
      ac8,
      is8,
      swapper4,
      {},
      {},
      "HCO3-",
      {"H2O", "Calcite", "HCO3-", "H+"},
      {175.0, 3.0, 3.2E-2, 1E-8}, // the molality of H+ is carefully chosen so the secondary species
                                  // do not contribute much to the ionic strengths
      cm,
      25,
      4,
      1E-20);
  // Here, water and HCO3- are the components in the algebraic system
  EXPECT_EQ(egs.getNumInAlgebraicSystem(), 2);
  EXPECT_EQ(egs.getNumBasisInAlgebraicSystem(), 2);
  EXPECT_EQ(egs.getNumSurfacePotentials(), 0);
  DenseVector<Real> res_orig(2);
  res_orig(0) = egs.getResidualComponent(0);
  res_orig(1) = egs.getResidualComponent(1);
  const std::vector<Real> var_orig =
      egs.getAlgebraicVariableValues(); // var[0] ~ 175/55 = 3.2 (kg of solvent water).  var[1]
                                        // ~ 3.2E-2/3.2 = 0.01 (molality of HCO3-)
  DenseMatrix<Real> jac(1, 1); // deliberately size incorrectly to check resizing done correctly
  egs.computeJacobian(res_orig, jac);

  const std::vector<Real> mol_orig = egs.getAlgebraicBasisValues();
  EXPECT_EQ(mol_orig.size(), var_orig.size());
  for (unsigned a = 0; a < mol_orig.size(); ++a)
    EXPECT_EQ(mol_orig[a], var_orig[a]);

  const Real eps = 1E-9;
  for (unsigned var_num = 0; var_num < 2; ++var_num)
  {
    std::vector<Real> var_new = var_orig;
    var_new[var_num] += eps;
    egs.setAlgebraicVariables(var_new);
    for (unsigned res_comp = 0; res_comp < 2; ++res_comp)
    {
      const Real expected = (egs.getResidualComponent(res_comp) - res_orig(res_comp)) / eps;
      EXPECT_NEAR(jac(res_comp, var_num) / expected, 1.0, 1E-5);
    }
  }
}

/**
 * Check jacobian calculations when mass of solvent water is not a variable
 * Note that because the Jacobian calculations do not consider derivatives of activity or
 * activity coefficients, they are only roughly correct
 */
TEST(EquilibiumGeochemicalSystemTest, jac2)
{
  const std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm = {
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES};
  EquilibriumGeochemicalSystem egs(mgd_good_calcite,
                                   ac8,
                                   is8,
                                   swapper4,
                                   {},
                                   {},
                                   "HCO3-",
                                   {"H2O", "Calcite", "HCO3-", "H+"},
                                   {1.0, 3.0, 3.2E-4, 1E-4},
                                   cm,
                                   25,
                                   2,
                                   1E-20);
  // Here, H+ and HCO3- are the components in the algebraic system
  EXPECT_EQ(egs.getNumInAlgebraicSystem(), 2);
  EXPECT_EQ(egs.getNumBasisInAlgebraicSystem(), 2);
  EXPECT_EQ(egs.getNumSurfacePotentials(), 0);
  DenseVector<Real> res_orig(2);
  res_orig(0) = egs.getResidualComponent(0);
  res_orig(1) = egs.getResidualComponent(1);
  const std::vector<Real> var_orig = egs.getAlgebraicVariableValues(); // both are 9E-5
  DenseMatrix<Real> jac(1, 1); // deliberately size incorrectly to check resizing done correctly
  egs.computeJacobian(res_orig, jac);

  const Real eps = 1E-9;
  for (unsigned var_num = 0; var_num < 2; ++var_num)
  {
    std::vector<Real> var_new = var_orig;
    var_new[var_num] += eps;
    egs.setAlgebraicVariables(var_new);
    for (unsigned res_comp = 0; res_comp < 2; ++res_comp)
    {
      const Real expected = (egs.getResidualComponent(res_comp) - res_orig(res_comp)) / eps;
      if (var_num == 0)
        EXPECT_NEAR(jac(res_comp, var_num) / expected,
                    1.0,
                    1.0E-10); // everything happens to be linear in H+
      else
        EXPECT_NEAR(jac(res_comp, var_num) / expected, 1.0, 1.0E-4); // nonlinear in HCO3-
    }
  }
}

/**
 * Check jacobian calculations when mass of solvent water is not a variable, and activity is
 * fixed. Note that because the Jacobian calculations do not consider derivatives of activity or
 * activity coefficients, they are only roughly correct
 */
TEST(EquilibiumGeochemicalSystemTest, jac3)
{
  const std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm = {
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY};
  EquilibriumGeochemicalSystem egs(mgd_good_calcite,
                                   ac8,
                                   is8,
                                   swapper4,
                                   {},
                                   {},
                                   "H+",
                                   {"H2O", "Calcite", "H+", "HCO3-"},
                                   {1.0, 3.0, 3.2E-4, 1E-4},
                                   cm,
                                   25,
                                   0,
                                   1E-20);
  // Here H+ is the only component in the algebraic system
  EXPECT_EQ(egs.getNumInAlgebraicSystem(), 1);
  EXPECT_EQ(egs.getNumBasisInAlgebraicSystem(), 1);
  EXPECT_EQ(egs.getNumSurfacePotentials(), 0);
  DenseVector<Real> res_orig(1);
  res_orig(0) = egs.getResidualComponent(0);
  const std::vector<Real> var_orig = egs.getAlgebraicVariableValues(); // around 3E-4
  DenseMatrix<Real> jac(1, 1);
  egs.computeJacobian(res_orig, jac);

  const Real eps = 1E-11;
  std::vector<Real> var_new = var_orig;
  var_new[0] += eps;
  egs.setAlgebraicVariables(var_new);
  const Real expected = (egs.getResidualComponent(0) - res_orig(0)) / eps;
  EXPECT_NEAR(jac(0, 0) / expected, 1.0, 1.0E-7);
}

/**
 * Check jacobian calculations when mass of solvent water is a variable, and activity is fixed.
 * Note that because the Jacobian calculations do not consider derivatives of activity or
 * activity coefficients, they are only roughly correct
 */
TEST(EquilibiumGeochemicalSystemTest, jac4)
{
  const PertinentGeochemicalSystem model(database_good_calcite,
                                         {"H2O", "H+", "HCO3-", "Ca++"},
                                         {"Calcite"},
                                         {},
                                         {},
                                         {},
                                         {},
                                         "O2(aq)",
                                         "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();
  const std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm = {
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_WATER,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY};
  EquilibriumGeochemicalSystem egs(mgd,
                                   ac8,
                                   is8,
                                   swapper4,
                                   {},
                                   {},
                                   "H+",
                                   {"H2O", "Ca++", "H+", "HCO3-"},
                                   {175.0, 4E1, 1E1, 1E-4},
                                   cm,
                                   25,
                                   0,
                                   1E-20);
  EXPECT_EQ(egs.getNumInAlgebraicSystem(), 3);
  EXPECT_EQ(egs.getNumBasisInAlgebraicSystem(), 3);
  EXPECT_EQ(egs.getNumSurfacePotentials(), 0);
  DenseVector<Real> res_orig(3);
  res_orig(0) = egs.getResidualComponent(0);
  res_orig(1) = egs.getResidualComponent(1);
  res_orig(2) = egs.getResidualComponent(2);
  const std::vector<Real> var_orig = egs.getAlgebraicVariableValues(); // around {3.1, 2.9, 11.4}
  DenseMatrix<Real> jac(1, 1);
  egs.computeJacobian(res_orig, jac);

  const std::vector<Real> mol_orig = egs.getAlgebraicBasisValues();
  EXPECT_EQ(mol_orig.size(), var_orig.size());
  for (unsigned a = 0; a < mol_orig.size(); ++a)
    EXPECT_EQ(mol_orig[a], var_orig[a]);

  const Real eps = 1E-6;
  for (unsigned var_num = 0; var_num < 3; ++var_num)
  {
    std::vector<Real> var_new = var_orig;
    var_new[var_num] += eps;
    egs.setAlgebraicVariables(var_new);
    for (unsigned res_comp = 0; res_comp < 3; ++res_comp)
    {
      const Real expected = (egs.getResidualComponent(res_comp) - res_orig(res_comp)) / eps;
      if (std::abs(expected) < 1.0E-7)
        EXPECT_NEAR(jac(res_comp, var_num), 0.0, 1.0E-7);
      else
        EXPECT_NEAR(expected / jac(res_comp, var_num), 1.0, 1.0E-7);
    }
  }
}

/// check saturation indices
TEST(EquilibiumGeochemicalSystemTest, saturationIndices)
{
  const PertinentGeochemicalSystem model(database_good_calcite,
                                         {"H2O", "H+", "HCO3-", "Ca++"},
                                         {"Calcite"},
                                         {},
                                         {},
                                         {},
                                         {},
                                         "O2(aq)",
                                         "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();
  const std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm_fixed_activity = {
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::KG_SOLVENT_WATER,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY};
  const EquilibriumGeochemicalSystem egs(mgd,
                                         ac3,
                                         is3,
                                         swapper4,
                                         {},
                                         {},
                                         "H+",
                                         {"H2O", "Ca++", "H+", "HCO3-"},
                                         {1.11, 3.0, 2.0, 1.5},
                                         cm_fixed_activity,
                                         25,
                                         0,
                                         1E-20);
  std::vector<Real> si = egs.getSaturationIndices();
  for (const auto & name_index : mgd.eqm_species_index)
    if (name_index.first == "Calcite")
      EXPECT_NEAR(
          si[name_index.second],
          std::log10(egs.getBasisActivity(3) * egs.getBasisActivity(2) / egs.getBasisActivity(1)) -
              1.7130,
          1E-9);
    else
      EXPECT_EQ(si[name_index.second], 0.0);
}

/// check swap exceptions
TEST(EquilibiumGeochemicalSystemTest, swapExceptions)
{
  EquilibriumGeochemicalSystem nonconst = egs_good_calcite;
  try
  {
    nonconst.performSwap(0, 0);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find(
            "EquilibriumGeochemicalSystem: attempting to swap out water and replace it by CO2(aq)."
            "  This could be because the algorithm would like to "
            "swap out the charge-balance species, in which case you should choose a "
            "different charge-balance species") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    nonconst.performSwap(1, 0);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("EquilibriumGeochemicalSystem: attempting to swap the charge-balance "
                         "species out of the basis") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  GeochemicalDatabaseReader database("database/moose_testdb.json");
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-", "O2(aq)"}, {}, {"O2(g)"}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm;
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_WATER);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY);
  cm.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY);

  EquilibriumGeochemicalSystem egs(mgd,
                                   ac3,
                                   is3,
                                   swapper4,
                                   {},
                                   {},
                                   "H+",
                                   {"H2O", "H+", "O2(aq)", "HCO3-"},
                                   {1.0, 1.5, 3.0, 2.5},
                                   cm,
                                   25,
                                   0,
                                   1E-20);

  try
  {
    egs.performSwap(3, 6);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("EquilibriumGeochemicalSystem: attempting to swap a gas into the basis") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm2;
  cm2.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_WATER);
  cm2.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES);
  cm2.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FUGACITY);
  cm2.push_back(EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY);
  EquilibriumGeochemicalSystem egs2(mgd,
                                    ac3,
                                    is3,
                                    swapper4,
                                    {"O2(aq)"},
                                    {"O2(g)"},
                                    "H+",
                                    {"H2O", "H+", "O2(g)", "HCO3-"},
                                    {1.0, 1.5, 3.0, 2.5},
                                    cm2,
                                    25,
                                    0,
                                    1E-20);

  try
  {
    egs2.performSwap(3, 0);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("EquilibriumGeochemicalSystem: attempting to swap a gas out of the basis") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// check swap
TEST(EquilibiumGeochemicalSystemTest, swap)
{
  const std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm = {
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::KG_SOLVENT_WATER,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLES_MINERAL_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY};
  ModelGeochemicalDatabase mgd = model_good_calcite.modelGeochemicalDatabase();
  EquilibriumGeochemicalSystem egs(mgd,
                                   ac3,
                                   is3,
                                   swapper4,
                                   {"Ca++"},
                                   {"Calcite"},
                                   "H+",
                                   {"H2O", "Calcite", "H+", "HCO3-"},
                                   {1.75, 3.0, 2.0, 1.0},
                                   cm,
                                   25,
                                   0,
                                   1E-20);

  EXPECT_EQ(egs.getNumInAlgebraicSystem(), 1);
  EXPECT_EQ(egs.getNumBasisInAlgebraicSystem(), 1);
  EXPECT_EQ(egs.getNumSurfacePotentials(), 0);
  EXPECT_EQ(egs.getInAlgebraicSystem()[0], false);
  EXPECT_EQ(egs.getInAlgebraicSystem()[1], true);
  EXPECT_EQ(egs.getInAlgebraicSystem()[2], false);
  EXPECT_EQ(egs.getInAlgebraicSystem()[3], false);
  EXPECT_EQ(egs.getChargeBalanceBasisIndex(), 1);
  const std::vector<Real> bm = egs.getBulkMoles();

  egs.performSwap(3, 5); // swap out Calcite and replace by Ca++

  EXPECT_EQ(egs.getNumInAlgebraicSystem(), 2);      // now Ca++ has a bulk moles attached to it
  EXPECT_EQ(egs.getNumBasisInAlgebraicSystem(), 2); // now Ca++ has a bulk moles attached to it
  EXPECT_EQ(egs.getNumSurfacePotentials(), 0);
  EXPECT_EQ(egs.getInAlgebraicSystem()[0], false);
  EXPECT_EQ(egs.getInAlgebraicSystem()[1], true);
  EXPECT_EQ(egs.getInAlgebraicSystem()[2], false);
  EXPECT_EQ(egs.getInAlgebraicSystem()[3], true);
  // Ca++ = Calcite - HCO3- + H+.  So, same number of bulk moles of Ca++ as Calcite, but number of
  // bulk moles of H+ = (original bulk moles of H+) - bulk moles of Calcite
  EXPECT_EQ(egs.getChargeBalanceBasisIndex(), 1);
  EXPECT_NEAR(egs.getBulkMoles()[1], bm[1] - bm[3], 1.0E-7);
  EXPECT_NEAR(egs.getBulkMoles()[3], bm[3], 1.0E-7);
}

/// check get ionic strengths
TEST(EquilibiumGeochemicalSystemTest, getIS)
{
  const std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm = {
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::KG_SOLVENT_WATER,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY};
  GeochemistryIonicStrength is(0.0078125, 0.0078125, false);
  GeochemistryActivityCoefficientsDebyeHuckel ac(is);
  ModelGeochemicalDatabase mgd = model_good_calcite.modelGeochemicalDatabase();
  const EquilibriumGeochemicalSystem egs_small_IS(mgd,
                                                  ac,
                                                  is,
                                                  swapper4,
                                                  {},
                                                  {},
                                                  "H+",
                                                  {"H2O", "Ca++", "H+", "HCO3-"},
                                                  {1.75, 3.0E-1, 2.0E-1, 1.0E-1},
                                                  cm,
                                                  25,
                                                  0,
                                                  1E-20);
  EXPECT_EQ(egs_small_IS.getIonicStrength(), 0.0078125);
  EXPECT_EQ(egs_small_IS.getStoichiometricIonicStrength(), 0.0078125);

  GeochemistryIonicStrength is_false(3.0, 3.0, true);
  GeochemistryActivityCoefficientsDebyeHuckel ac_false(is_false);
  const EquilibriumGeochemicalSystem egs(
      mgd,
      ac_false,
      is_false,
      swapper4,
      {},
      {},
      "H+",
      {"H2O", "Ca++", "H+", "HCO3-"},
      {1.75, 1E-10, 1E-10, 1.0}, // up to 1E-10, the only contributor to IS is HCO3-
      cm,
      25,
      0,
      1E-20);
  EXPECT_NEAR(egs.getIonicStrength(), 0.5, 1.0E-8);
  EXPECT_NEAR(egs.getStoichiometricIonicStrength(), 0.5, 1.0E-8);
}

/// Check alterChargeBalanceSpecies and revert
TEST(EquilibiumGeochemicalSystemTest, alterAndRevertChargeBalance)
{
  const GeochemicalDatabaseReader database("database/moose_testdb.json");
  const PertinentGeochemicalSystem model(
      database, {"H2O", "StoiCheckBasis", "HCO3-"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();
  const std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm = {
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::KG_SOLVENT_WATER,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES};
  EquilibriumGeochemicalSystem egs(mgd,
                                   ac3,
                                   is3,
                                   swapper3,
                                   {},
                                   {},
                                   "StoiCheckBasis",
                                   {"H2O", "StoiCheckBasis", "HCO3-"},
                                   {1.75, 5.0, 1.0},
                                   cm,
                                   25,
                                   0,
                                   1E-20);
  // in this case, charge-neutrality can be enforced immediately
  EXPECT_NEAR(egs.getTotalCharge(), 0.0, 1.0E-12);
  EXPECT_NEAR(egs.getBulkMoles()[1],
              1.0 / 2.5,
              1E-9); // this has been changed from 5.0 by the charge-balancing
  EXPECT_EQ(egs.getBulkMoles()[2], 1.0);
  EXPECT_EQ(egs.getChargeBalanceBasisIndex(), 1);

  // StoiCheckBasis has molality 0.2, while HCO3- has molality 0.5, so can change the charge-balance
  // species via:
  egs.alterChargeBalanceSpecies(0.3); // now the charge-balance species should be HCO3-
  EXPECT_EQ(egs.getChargeBalanceBasisIndex(), 2);
  EXPECT_EQ(egs.getBulkMoles()[1], 5.0);
  EXPECT_EQ(egs.getBulkMoles()[2],
            5 * 2.5); // this has been changed from 1.0 by the charge-balancing

  // revert to the original
  EXPECT_TRUE(egs.revertToOriginalChargeBalanceSpecies());
  EXPECT_EQ(egs.getChargeBalanceBasisIndex(), 1);
  EXPECT_NEAR(egs.getBulkMoles()[1], 1.0 / 2.5, 1E-9);
  EXPECT_EQ(egs.getBulkMoles()[2], 1.0);
}

/// Check getNumRedox
TEST(EquilibiumGeochemicalSystemTest, getNumRedox) { EXPECT_EQ(egs_redox.getNumRedox(), 2); }

/// Check getRedoxLog10K
TEST(EquilibiumGeochemicalSystemTest, getRedoxLog10K)
{
  Real boa = 1.0 / 4.0 / 7.5;
  EXPECT_NEAR(egs_redox.getRedoxLog10K(0), -boa * 542.8292 + 20.7757 - 0.25 * (-2.8990), 1E-8);
  boa = 1.0 / 8.0;
  EXPECT_NEAR(egs_redox.getRedoxLog10K(1), -boa * 144.1080 + 20.7757 - 0.25 * (-2.8990), 1E-8);
  try
  {
    egs_redox.getRedoxLog10K(2);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find(
            "Cannot retrieve log10K for redox species 2 since there are only 2 redox species") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check log10RedoxActivityProduct
TEST(EquilibiumGeochemicalSystemTest, log10RedoxActivityProduct)
{
  // e- = (1/4/7.5)(O-phth)-- + (1/2 + 5/4/7.5)H2O + (-1 - 6/4/7.5)H+ - 8/4/7.5HCO3-
  Real boa = 1.0 / 4.0 / 7.5;
  const Real log10ap_o =
      boa * std::log10(5.0) + (-1.0 - 6.0 * boa) * std::log10(2.0) - 8 * boa * std::log10(3.0);
  EXPECT_NEAR(egs_redox.log10RedoxActivityProduct(0), log10ap_o, 1E-8);
  // e- = (1/8)CH4(aq) + (1/2 - 1/8)H2O - (1+1/8)H+ - (1/8)HCO3-
  boa = 1.0 / 8.0;
  const Real log10ap_c =
      boa * std::log10(6.0) - (1.0 + boa) * std::log10(2.0) - boa * std::log10(3.0);
  EXPECT_NEAR(egs_redox.log10RedoxActivityProduct(1), log10ap_c, 1E-8);
  try
  {
    egs_redox.log10RedoxActivityProduct(2);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Cannot retrieve activity product for redox species 2 since there are "
                         "only 2 redox species") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check getSurfacePotential exception
TEST(EquilibiumGeochemicalSystemTest, getSurfacePotentialException)
{
  try
  {
    egs_good_calcite.getSurfacePotential(0);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Cannot retrieve the surface potential for surface 0 since there are only "
                         "0 surfaces involved in surface complexation") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check getSurfaceCharge exception
TEST(EquilibiumGeochemicalSystemTest, getSurfaceChargeException)
{
  try
  {
    egs_good_calcite.getSurfaceCharge(0);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Cannot retrieve the surface charge for surface 0 since there are only 0 "
                         "surfaces involved in surface complexation") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check sorbing surface area
TEST(EquilibiumGeochemicalSystemTest, sorbingSurfaceArea)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe+++"},
                                   {"Fe(OH)3(ppd)"},
                                   {},
                                   {},
                                   {},
                                   {},
                                   "O2(aq)",
                                   "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();
  const std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm = {
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::KG_SOLVENT_WATER,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLES_MINERAL_SPECIES};
  EquilibriumGeochemicalSystem egs(mgd,
                                   ac3,
                                   is3,
                                   swapper5,
                                   {"Fe+++"},
                                   {"Fe(OH)3(ppd)"},
                                   "H+",
                                   {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe(OH)3(ppd)"},
                                   {1.75, 1.0, 2.0, 1.0, 1.5},
                                   cm,
                                   25.0,
                                   0,
                                   1E-20);
  EXPECT_EQ(egs.getSorbingSurfaceArea().size(), 1);
  EXPECT_EQ(egs.getSorbingSurfaceArea()[0], 1.5 * 106.8689 * 600.0);
}

/// Check surface potential things (not jacobian)
TEST(EquilibiumGeochemicalSystemTest, surfacePot)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe+++", "HCO3-"},
                                   {"Fe(OH)3(ppd)"},
                                   {},
                                   {},
                                   {},
                                   {},
                                   "O2(aq)",
                                   "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();
  const std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm = {
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::KG_SOLVENT_WATER,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES};
  const Real temp = 45.0;
  EquilibriumGeochemicalSystem egs(mgd,
                                   ac3,
                                   is3,
                                   swapper6,
                                   {},
                                   {},
                                   "H+",
                                   {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe+++", "HCO3-"},
                                   {1.75, 1.0, 2.0, 3.0, 1.0, 1.0},
                                   cm,
                                   temp,
                                   0,
                                   1E-20);

  // test basic things like sizes and that the algebraic variables are correctly set
  EXPECT_EQ(egs.getNumInAlgebraicSystem(), 3);
  EXPECT_EQ(egs.getNumBasisInAlgebraicSystem(), 2);
  EXPECT_EQ(egs.getNumSurfacePotentials(), 1);
  EXPECT_EQ(egs.getBasisIndexOfAlgebraicSystem()[0],
            1); // H+ is slot=0 in algebraic system and slot=1 in basis
  EXPECT_EQ(egs.getBasisIndexOfAlgebraicSystem()[1],
            5); // HCO3-
  EXPECT_EQ(egs.getAlgebraicIndexOfBasisSystem()[1], 0);
  EXPECT_EQ(egs.getAlgebraicIndexOfBasisSystem()[5], 1);
  DenseVector<Real> alg_vars = egs.getAlgebraicVariableDenseValues();
  EXPECT_EQ(alg_vars.size(), 3);
  std::vector<Real> mols = egs.getAlgebraicBasisValues();
  EXPECT_EQ(mols.size(), 2);
  EXPECT_EQ(mols[0], alg_vars(0));
  EXPECT_EQ(mols[1], alg_vars(1));
  EXPECT_EQ(egs.getAlgebraicVariableValues().size(), 3);

  // try setting a _surface_pot_expr to a non-positive quantity
  try
  {
    alg_vars(2) = -1.0;
    egs.setAlgebraicVariables(alg_vars);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Cannot set algebraic variables to non-positive values such as -1") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  // set the _surface_pot_expr to 2.0
  alg_vars(2) = 2.0;
  egs.setAlgebraicVariables(alg_vars);

  // check that the setting worked
  const std::vector<Real> av = egs.getAlgebraicVariableValues();
  EXPECT_EQ(av.size(), 3);
  for (unsigned i = 0; i < 3; ++i)
    EXPECT_EQ(av[i], alg_vars(i));

  // check the surface potential is correctly computed
  const Real surf_pot_gold = -std::log(alg_vars(2)) * 2.0 * 8.314472 * 318.15 / 96485.3415;
  EXPECT_NEAR(egs.getSurfacePotential(0), surf_pot_gold, 1.0E-9);
  try
  {
    egs.getSurfacePotential(1);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Cannot retrieve the surface potential for surface 1 since there are only "
                         "1 surfaces involved in surface complexation") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  // check the specific surface charge is correctly computed
  const Real pref =
      0.5 / 96485.3415 *
      std::sqrt(8.314472 * 318.15 * 8.8541878128E-12 * 78.5 * 1000.0 * egs.getIonicStrength()) *
      (-alg_vars(2) + 1.0 / alg_vars(2));
  const Real surf_charge_gold = pref * 96485.3415;
  EXPECT_NEAR(egs.getSurfaceCharge(0), surf_charge_gold, 1.0E-9);
  try
  {
    egs.getSurfaceCharge(1);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Cannot retrieve the surface charge for surface 1 since there are only 1 "
                         "surfaces involved in surface complexation") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  // check that equilibrium molality for the surface species is correctly computed
  const Real act_h = egs.getBasisActivity(1);
  const Real act_bicarb = egs.getBasisActivity(2);
  const unsigned ind_surf = model.modelGeochemicalDatabase().eqm_species_index.at(">(s)FeO-");
  const Real mol_surf = egs.getEquilibriumMolality(ind_surf);
  EXPECT_NEAR(mol_surf,
              act_bicarb / act_h / std::pow(10.0, egs.getLog10K(ind_surf)) *
                  std::pow(alg_vars(2), -2.0),
              1.0E-9);

  // check that residuals are correctly calculated (non-surface stuff is checked in other tests)
  EXPECT_NEAR(egs.getResidualComponent(2), -pref * 600.0 + 1.75 * (-1.0) * mol_surf, 1.0E-9);
  try
  {
    egs.getResidualComponent(3);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Cannot retrieve residual for algebraic index 3 because there are only 2 "
                         "molalities in the algebraic system and 1 surface potentials") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check jacobian when there is a surface potential
TEST(EquilibiumGeochemicalSystemTest, surfacePotJac)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe+++", "HCO3-"},
                                   {"Fe(OH)3(ppd)"},
                                   {},
                                   {},
                                   {},
                                   {},
                                   "O2(aq)",
                                   "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();
  const std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm = {
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_WATER,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES};
  const Real temp = 45.0;
  GeochemistryIonicStrength is2(1E-2, 1E-2, false);
  GeochemistryActivityCoefficientsDebyeHuckel ac2(is2);
  EquilibriumGeochemicalSystem egs(mgd,
                                   ac2,
                                   is2,
                                   swapper6,
                                   {},
                                   {},
                                   "H+",
                                   {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe+++", "HCO3-"},
                                   {1.75, 1.0, 2.0, 3.0, 0.5, 1.0},
                                   cm,
                                   temp,
                                   0,
                                   1E-20);
  const unsigned num_alg = egs.getNumInAlgebraicSystem();
  DenseVector<Real> res_orig(num_alg);
  for (unsigned i = 0; i < num_alg; ++i)
    res_orig(i) = egs.getResidualComponent(i);
  const std::vector<Real> var_orig = egs.getAlgebraicVariableValues();
  DenseMatrix<Real> jac(1, 1);
  egs.computeJacobian(res_orig, jac);

  const Real eps = 1E-3;
  for (unsigned var_num = 0; var_num < num_alg; ++var_num)
  {
    std::vector<Real> var_new = var_orig;
    var_new[var_num] *= (1.0 + eps);
    egs.setAlgebraicVariables(var_new);
    for (unsigned res_comp = 0; res_comp < num_alg; ++res_comp)
    {
      const Real expected = (egs.getResidualComponent(res_comp) - res_orig(res_comp)) /
                            (var_new[var_num] - var_orig[var_num]);
      if (std::abs(expected) < 1.0E-7)
        EXPECT_NEAR(jac(res_comp, var_num), 0.0, 1.0E-7);
      else
        EXPECT_NEAR(expected / jac(res_comp, var_num), 1.0, 1.0E-2);
    }
  }
}
