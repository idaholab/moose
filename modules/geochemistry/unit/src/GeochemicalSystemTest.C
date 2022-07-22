//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeochemicalSystemTest.h"

/// Check MultiMooseEnum constructor
TEST_F(GeochemicalSystemTest, constructWithMultiMooseEnum)
{
  MultiMooseEnum constraint_user_meaning(
      "kg_solvent_water bulk_composition bulk_composition_with_kinetic free_concentration "
      "free_mineral activity log10activity fugacity log10fugacity");
  constraint_user_meaning.push_back(
      "activity bulk_composition bulk_composition free_concentration");
  MultiMooseEnum constraint_unit("dimensionless moles molal kg g mg ug kg_per_kg_solvent "
                                 "g_per_kg_solvent mg_per_kg_solvent ug_per_kg_solvent cm3");
  constraint_unit.push_back("dimensionless moles moles molal");
  ModelGeochemicalDatabase mgd_calcite2 = _model_calcite.modelGeochemicalDatabase();
  const GeochemicalSystem egs(mgd_calcite2,
                              _ac3,
                              _is3,
                              _swapper4,
                              {"Ca++"},
                              {"Calcite"},
                              "H+",
                              {"H2O", "Calcite", "H+", "HCO3-"},
                              {1.75, 3.0, 2.0, 1.0},
                              constraint_unit,
                              constraint_user_meaning,
                              25,
                              0,
                              1E-20,
                              {},
                              {},
                              MultiMooseEnum(""));
}

/// Check num in basis
TEST_F(GeochemicalSystemTest, numInBasis) { EXPECT_EQ(_egs_calcite.getNumInBasis(), (unsigned)4); }

/// Check num in equilibrium
TEST_F(GeochemicalSystemTest, numInEquilibrium)
{
  EXPECT_EQ(_egs_calcite.getNumInEquilibrium(), (unsigned)6);
}

/// Check swaps have correct size
TEST_F(GeochemicalSystemTest, swapSizeException)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
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
    GeochemicalSystem egs(mgd,
                          _ac3,
                          _is3,
                          _swapper7,
                          {"H+"},
                          {},
                          "H+",
                          {},
                          {},
                          _cu_dummy,
                          _cm_dummy,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
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
TEST_F(GeochemicalSystemTest, swapChargeBalanceException)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
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
    GeochemicalSystem egs(mgd,
                          _ac3,
                          _is3,
                          _swapper7,
                          {"H+"},
                          {"FeOH"},
                          "H+",
                          {},
                          {},
                          _cu_dummy,
                          _cm_dummy,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
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
TEST_F(GeochemicalSystemTest, swapException)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
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
    GeochemicalSystem egs(mgd,
                          _ac3,
                          _is3,
                          _swapper7,
                          {"CO2(aq)"},
                          {"CO2(aq)"},
                          "H+",
                          {},
                          {},
                          _cu_dummy,
                          _cm_dummy,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
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
TEST_F(GeochemicalSystemTest, constraintSizeExceptions)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm_poor;
  cm_poor.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm_poor.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu_poor;
  cu_poor.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu_poor.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);

  try
  {
    GeochemicalSystem egs(mgd,
                          _ac3,
                          _is3,
                          _swapper3,
                          {},
                          {},
                          "H+",
                          {"H2O", "H+", "HCO3-"},
                          {1.0, 2.0},
                          cu,
                          cm,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
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
    GeochemicalSystem egs(mgd,
                          _ac3,
                          _is3,
                          _swapper3,
                          {},
                          {},
                          "H+",
                          {"H2O", "H+", "HCO3-"},
                          {1.0, 2.0, 3.0},
                          cu,
                          cm_poor,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
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
    GeochemicalSystem egs(mgd,
                          _ac3,
                          _is3,
                          _swapper3,
                          {},
                          {},
                          "H+",
                          {"H2O", "H+", "HCO3-"},
                          {1.0, 2.0, 3.0},
                          cu_poor,
                          cm,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Constrained species names must have same length as constraint units") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    GeochemicalSystem egs(mgd,
                          _ac3,
                          _is3,
                          _swapper3,
                          {},
                          {},
                          "H+",
                          {"H2O", "H+"},
                          {1.0, 2.0},
                          cu_poor,
                          cm_poor,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
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
TEST_F(GeochemicalSystemTest, constraintNameExceptions)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);

  try
  {
    GeochemicalSystem egs(mgd,
                          _ac3,
                          _is3,
                          _swapper3,
                          {},
                          {},
                          "H+",
                          {"H2O", "H+", "H20"},
                          {1.0, 2.0, 3.0},
                          cu,
                          cm,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
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
    GeochemicalSystem egs(mgd,
                          _ac3,
                          _is3,
                          _swapper3,
                          {"HCO3-"},
                          {"CO2(aq)"},
                          "H+",
                          {"H2O", "H+", "HCO3-"},
                          {1.0, 2.0, 3.0},
                          cu,
                          cm,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
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
TEST_F(GeochemicalSystemTest, positiveException1)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FUGACITY);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::KG);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);

  try
  {
    GeochemicalSystem egs(mgd,
                          _ac3,
                          _is3,
                          _swapper3,
                          {"HCO3-"},
                          {"CO2(aq)"},
                          "H+",
                          {"H2O", "H+", "CO2(aq)"},
                          {-1.0, 2.0, 3.0},
                          cu,
                          cm,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
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
TEST_F(GeochemicalSystemTest, positiveException2)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);

  try
  {
    GeochemicalSystem egs(mgd,
                          _ac3,
                          _is3,
                          _swapper3,
                          {"HCO3-"},
                          {"CO2(aq)"},
                          "H+",
                          {"H2O", "H+", "CO2(aq)"},
                          {-1.0, 2.0, 3.0},
                          cu,
                          cm,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
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
TEST_F(GeochemicalSystemTest, positiveException3)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-", "O2(aq)"}, {}, {"O2(g)"}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FUGACITY);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);

  try
  {
    GeochemicalSystem egs(mgd,
                          _ac3,
                          _is3,
                          _swapper4,
                          {"O2(aq)"},
                          {"O2(g)"},
                          "H+",
                          {"H2O", "H+", "O2(g)", "HCO3-"},
                          {1.0, 2.0, -3.0, 4.0},
                          cu,
                          cm,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
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
TEST_F(GeochemicalSystemTest, positiveException4)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);

  try
  {
    GeochemicalSystem egs(mgd,
                          _ac3,
                          _is3,
                          _swapper3,
                          {"HCO3-"},
                          {"CO2(aq)"},
                          "H+",
                          {"H2O", "H+", "CO2(aq)"},
                          {1.0, 2.0, -2.0},
                          cu,
                          cm,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Specified free concentration values must be positive: you entered -2") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check appropriate positivity
TEST_F(GeochemicalSystemTest, positiveException5)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-", "Ca++"}, {"Calcite"}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_MINERAL);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);

  try
  {
    GeochemicalSystem egs(mgd,
                          _ac3,
                          _is3,
                          _swapper4,
                          {"Ca++"},
                          {"Calcite"},
                          "H+",
                          {"H2O", "Calcite", "H+", "HCO3-"},
                          {1.0, -3.0, 2.0, 1.0},
                          cu,
                          cm,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Specified free mineral values must be positive: you entered -3") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check water provided with appropriate value
TEST_F(GeochemicalSystemTest, waterException)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);

  try
  {
    GeochemicalSystem egs(mgd,
                          _ac3,
                          _is3,
                          _swapper3,
                          {"HCO3-"},
                          {"CO2(aq)"},
                          "H+",
                          {"H2O", "H+", "CO2(aq)"},
                          {1.0, 2.0, 3.0},
                          cu,
                          cm,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("H2O must be provided with either a mass of solvent water, a bulk "
                         "composition in moles or mass, or an activity") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check gases are provided with appropriate value
TEST_F(GeochemicalSystemTest, fugacityException)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-", "O2(aq)"}, {}, {"O2(g)"}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);

  try
  {
    GeochemicalSystem egs(mgd,
                          _ac3,
                          _is3,
                          _swapper4,
                          {"O2(aq)"},
                          {"O2(g)"},
                          "H+",
                          {"H2O", "H+", "O2(g)", "HCO3-"},
                          {1.0, 2.0, 3.0, 4.0},
                          cu,
                          cm,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
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
TEST_F(GeochemicalSystemTest, mineralMeaningExecption)
{
  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);

  try
  {
    GeochemicalSystem egs(_mgd_calcite,
                          _ac3,
                          _is3,
                          _swapper4,
                          {},
                          {},
                          "H+",
                          {"H2O", "Calcite", "H+", "HCO3-"},
                          {1.0, 3.0, 2.0, 1.0},
                          cu,
                          cm,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("The mineral Calcite must be provided with either: a free number of moles, a free "
                 "mass or a free volume; or a bulk composition of moles or mass") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm2;
  cm2.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY);
  cm2.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION);
  cm2.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm2.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu2;
  cu2.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu2.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  cu2.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu2.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);

  try
  {
    GeochemicalSystem egs(_mgd_calcite,
                          _ac3,
                          _is3,
                          _swapper4,
                          {},
                          {},
                          "H+",
                          {"H2O", "Calcite", "H+", "HCO3-"},
                          {1.0, 3.0, 2.0, 1.0},
                          cu2,
                          cm2,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("The mineral Calcite must be provided with either: a free number of "
                 "moles, a free mass or a free volume; or a bulk composition of moles or mass") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check aqueous species are provided with appropriate meaning
TEST_F(GeochemicalSystemTest, aqueousSpeciesMeaningExecption)
{
  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_MINERAL);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);

  try
  {
    GeochemicalSystem egs(_mgd_calcite,
                          _ac3,
                          _is3,
                          _swapper4,
                          {},
                          {},
                          "HCO3-",
                          {"H2O", "Calcite", "H+", "HCO3-"},
                          {1.0, 3.0, 2.0, 1.0},
                          cu,
                          cm,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("The basis species H+ must be provided with a free concentration, bulk "
                         "composition or an activity") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm2;
  cm2.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY);
  cm2.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm2.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm2.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FUGACITY);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu2;
  cu2.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu2.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu2.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu2.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);

  try
  {
    GeochemicalSystem egs(_mgd_calcite,
                          _ac3,
                          _is3,
                          _swapper4,
                          {},
                          {},
                          "H+",
                          {"H2O", "Calcite", "H+", "HCO3-"},
                          {1.0, 3.0, 2.0, 1.0},
                          cu2,
                          cm2,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("The basis species HCO3- must be provided with a free concentration, bulk "
                         "composition or an activity") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Exception check units for water
TEST_F(GeochemicalSystemTest, waterUnitsException)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::G);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);

  try
  {
    GeochemicalSystem egs(mgd,
                          _ac3,
                          _is3,
                          _swapper3,
                          {},
                          {},
                          "H+",
                          {"H2O", "H+", "HCO3-"},
                          {1.0, 2.0, 3.0},
                          cu,
                          cm,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Units for kg_solvent_water must be kg") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Exception check units for bulk composition
TEST_F(GeochemicalSystemTest, bulkUnitsException)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::KG);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);

  try
  {
    GeochemicalSystem egs(mgd,
                          _ac3,
                          _is3,
                          _swapper3,
                          {},
                          {},
                          "H+",
                          {"H2O", "H+", "HCO3-"},
                          {1.0, 2.0, 3.0},
                          cu,
                          cm,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Species H+: units for bulk composition must be moles or mass") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Exception check units for free concentration
TEST_F(GeochemicalSystemTest, freeConcUnitsException)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::KG);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);

  try
  {
    GeochemicalSystem egs(mgd,
                          _ac3,
                          _is3,
                          _swapper3,
                          {},
                          {},
                          "H+",
                          {"H2O", "H+", "HCO3-"},
                          {1.0, 2.0, 3.0},
                          cu,
                          cm,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Species HCO3-: units for free concentration quantities must be molal or "
                         "mass_per_kg_solvent") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Exception check units for free mineral
TEST_F(GeochemicalSystemTest, freeMineralUnitsException)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-", "Ca++"}, {"Calcite"}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_MINERAL);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);

  try
  {
    GeochemicalSystem egs(mgd,
                          _ac3,
                          _is3,
                          _swapper4,
                          {"Ca++"},
                          {"Calcite"},
                          "H+",
                          {"H2O", "Calcite", "H+", "HCO3-"},
                          {1.0, 3.0, 2.0, 1.0},
                          cu,
                          cm,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find(
            "Species Calcite: units for free mineral quantities must be moles, mass or volume") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Exception check units for activity
TEST_F(GeochemicalSystemTest, activityUnitsException)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::KG);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);

  try
  {
    GeochemicalSystem egs(mgd,
                          _ac3,
                          _is3,
                          _swapper3,
                          {},
                          {},
                          "H+",
                          {"H2O", "H+", "HCO3-"},
                          {1.0, 2.0, 3.0},
                          cu,
                          cm,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("Species HCO3-: dimensionless units must be used when specifying activity") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  cm[2] = GeochemicalSystem::ConstraintUserMeaningEnum::LOG10ACTIVITY;
  try
  {
    GeochemicalSystem egs(mgd,
                          _ac3,
                          _is3,
                          _swapper3,
                          {},
                          {},
                          "H+",
                          {"H2O", "H+", "HCO3-"},
                          {1.0, 2.0, 3.0},
                          cu,
                          cm,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("Species HCO3-: dimensionless units must be used when specifying log10activity") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Exception check units for fugacity
TEST_F(GeochemicalSystemTest, fugacityUnitsException)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-", "O2(aq)"}, {}, {"O2(g)"}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FUGACITY);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);

  try
  {
    GeochemicalSystem egs(mgd,
                          _ac3,
                          _is3,
                          _swapper4,
                          {"O2(aq)"},
                          {"O2(g)"},
                          "H+",
                          {"H2O", "H+", "O2(g)", "HCO3-"},
                          {1.0, 2.0, 3.0, 4.0},
                          cu,
                          cm,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("Species O2(g): dimensionless units must be used when specifying fugacity") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  cm[2] = GeochemicalSystem::ConstraintUserMeaningEnum::LOG10FUGACITY;
  try
  {
    // no need to swap, since the above constructor of egs will have performed the swaps in mgd
    GeochemicalSystem egs(mgd,
                          _ac3,
                          _is3,
                          _swapper4,
                          {},
                          {},
                          "H+",
                          {"H2O", "H+", "O2(g)", "HCO3-"},
                          {1.0, 2.0, 3.0, 4.0},
                          cu,
                          cm,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("Species O2(g): dimensionless units must be used when specifying log10fugacity") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check units conversion occurs and constraint_meaning is built correctly
TEST_F(GeochemicalSystemTest, unitsConversion1)
{
  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_MINERAL);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::G);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::G_PER_KG_SOLVENT);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::CM3);

  const GeochemicalSystem egs(_mgd_calcite,
                              _ac3,
                              _is3,
                              _swapper4,
                              {},
                              {},
                              "H+",
                              {"H2O", "H+", "HCO3-", "Calcite"},
                              {100.0, 3.0, 1.0, 2.0},
                              cu,
                              cm,
                              25,
                              0,
                              1E-20,
                              {},
                              {},
                              {});
  const std::vector<GeochemicalSystem::ConstraintMeaningEnum> cim = {
      GeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_WATER,
      GeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      GeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY,
      GeochemicalSystem::ConstraintMeaningEnum::FREE_MOLES_MINERAL_SPECIES};
  for (unsigned i = 0; i < 4; ++i)
    EXPECT_EQ(egs.getConstraintMeaning()[i], cim[i]);
  EXPECT_NEAR(egs.getBulkMolesOld()[0], 100.0 / 18.0152, 1.0E-6); // H2O
  EXPECT_NEAR(egs.getBulkMolesOld()[1], 3.0, 1.0E-6);             // H+
  EXPECT_NEAR(
      egs.getSolventMassAndFreeMolalityAndMineralMoles()[2], 1.0 / 61.0171, 1.0E-6); // HCO3-
  EXPECT_NEAR(
      egs.getSolventMassAndFreeMolalityAndMineralMoles()[3], 2.0 / 36.9340, 1.0E-6); // Calcite
}

/// Check units conversion occurs and constraint_meaning is built correctly
TEST_F(GeochemicalSystemTest, unitsConversion2)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-", "O2(aq)"}, {}, {"O2(g)"}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::LOG10ACTIVITY);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::LOG10FUGACITY);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::KG);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::UG);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);

  const GeochemicalSystem egs(mgd,
                              _ac3,
                              _is3,
                              _swapper4,
                              {"O2(aq)"},
                              {"O2(g)"},
                              "HCO3-",
                              {"H2O", "H+", "HCO3-", "O2(g)"},
                              {1.0, 2.0, 3.0, 4.0},
                              cu,
                              cm,
                              25,
                              0,
                              1E-20,
                              {},
                              {},
                              {});
  const std::vector<GeochemicalSystem::ConstraintMeaningEnum> cim = {
      GeochemicalSystem::ConstraintMeaningEnum::KG_SOLVENT_WATER,
      GeochemicalSystem::ConstraintMeaningEnum::ACTIVITY,
      GeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      GeochemicalSystem::ConstraintMeaningEnum::FUGACITY};
  for (unsigned i = 0; i < 4; ++i)
    EXPECT_EQ(egs.getConstraintMeaning()[i], cim[i]);
  EXPECT_NEAR(egs.getSolventMassAndFreeMolalityAndMineralMoles()[0], 1.0, 1.0E-6); // H2O
  EXPECT_NEAR(egs.getBasisActivity(1), 1E2, 1.0E-6);                               // H+
  EXPECT_NEAR(egs.getBulkMolesOld()[2], 2.0E-6 / 61.0171, 1.0E-6);                 // HCO3-
  EXPECT_NEAR(egs.getBasisActivity(3), 1E4, 1.0E-6);                               // O2(g)
}

/// Check charge-balance species is provided with appropriate meaning
TEST_F(GeochemicalSystemTest, chargeBalanceMeaningExecption)
{
  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);

  try
  {
    GeochemicalSystem egs(_mgd_calcite,
                          _ac3,
                          _is3,
                          _swapper4,
                          {},
                          {},
                          "H+",
                          {"H2O", "Calcite", "H+", "HCO3-"},
                          {1.0, 3.0, 2.0, 1.0},
                          cu,
                          cm,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find(
            "For code consistency, the species H+ must be provided with a bulk composition "
            "because it is the charge-balance species.  The value provided should be a reasonable "
            "estimate of the mole number, but will be overridden as the solve progresses") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm2;
  cm2.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY);
  cm2.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm2.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION);
  cm2.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu2;
  cu2.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu2.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu2.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  cu2.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);

  try
  {
    GeochemicalSystem egs(_mgd_calcite,
                          _ac3,
                          _is3,
                          _swapper4,
                          {},
                          {},
                          "H+",
                          {"H2O", "Calcite", "H+", "HCO3-"},
                          {1.0, 3.0, 2.0, 1.0},
                          cu2,
                          cm2,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find(
            "For code consistency, the species H+ must be provided with a bulk composition "
            "because it is the charge-balance species.  The value provided should be a reasonable "
            "estimate of the mole number, but will be overridden as the solve progresses") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check charge-balance species is in the basis
TEST_F(GeochemicalSystemTest, chargeBalanceInBasisExecption)
{
  try
  {
    GeochemicalSystem egs(_mgd_calcite,
                          _ac3,
                          _is3,
                          _swapper4,
                          {},
                          {},
                          "OH-",
                          {"H2O", "Calcite", "H+", "HCO3-"},
                          {1.0, 3.0, 2.0, 1.0},
                          _cu_calcite,
                          _cm_calcite,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
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
TEST_F(GeochemicalSystemTest, chargeBalanceHasChargeExecption)
{
  try
  {
    GeochemicalSystem egs(_mgd_calcite,
                          _ac3,
                          _is3,
                          _swapper4,
                          {},
                          {},
                          "Calcite",
                          {"H2O", "Calcite", "H+", "HCO3-"},
                          {1.0, 3.0, 2.0, 1.0},
                          _cu_calcite,
                          _cm_calcite,
                          25,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
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
TEST_F(GeochemicalSystemTest, chargeBalanceIndex)
{
  EXPECT_EQ(_egs_calcite.getChargeBalanceBasisIndex(), (unsigned)1);
}

/// Check getLog10K
TEST_F(GeochemicalSystemTest, getLog10K)
{
  try
  {
    EXPECT_EQ(_egs_calcite.getLog10K(123), 1);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Cannot retrieve log10K for equilibrium species 123 since there are only "
                         "6 equilibrium species") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
  EXPECT_EQ(_egs_calcite.getLog10K(0), -6.3660); // CO2(aq) at 25degC
}

/// Check getInAlgebraicSystem
TEST_F(GeochemicalSystemTest, getInAlgebraicSystem)
{
  const std::vector<bool> as_gold = {false, true, false, false};
  EXPECT_EQ(_egs_calcite.getInAlgebraicSystem().size(), (std::size_t)4);
  for (unsigned i = 0; i < as_gold.size(); ++i)
    EXPECT_EQ(_egs_calcite.getInAlgebraicSystem()[i], as_gold[i]);
}

/// Check NumInAlgebraicSystem
TEST_F(GeochemicalSystemTest, NumInAlgebraicSystem)
{
  EXPECT_EQ(_egs_calcite.getNumInAlgebraicSystem(), (unsigned)1);
  EXPECT_EQ(_egs_calcite.getNumBasisInAlgebraicSystem(), (unsigned)1);
  EXPECT_EQ(_egs_calcite.getNumSurfacePotentials(), (unsigned)0);
  EXPECT_EQ(_egs_kinetic_calcite.getNumInAlgebraicSystem(), (unsigned)3);
  EXPECT_EQ(_egs_kinetic_calcite.getNumBasisInAlgebraicSystem(), (unsigned)2);
  EXPECT_EQ(_egs_kinetic_calcite.getNumSurfacePotentials(), (unsigned)0);
}

/// Check getBasisIndexOfAlgebraicSystem()
TEST_F(GeochemicalSystemTest, getBasisIndexOfAlgebraicSystem)
{
  EXPECT_EQ(_egs_calcite.getBasisIndexOfAlgebraicSystem()[0], (unsigned)1);
  EXPECT_EQ(_egs_kinetic_calcite.getBasisIndexOfAlgebraicSystem()[0], (unsigned)1);
  EXPECT_EQ(_egs_kinetic_calcite.getBasisIndexOfAlgebraicSystem()[1], (unsigned)3);
}

/// Check getAlgebraicIndexOfBasisSystem()
TEST_F(GeochemicalSystemTest, getAlgebraicIndexOfBasisSystem)
{
  EXPECT_EQ(_egs_calcite.getAlgebraicIndexOfBasisSystem()[1], (unsigned)0);
  EXPECT_EQ(_egs_kinetic_calcite.getAlgebraicIndexOfBasisSystem()[1], (unsigned)0);
  EXPECT_EQ(_egs_kinetic_calcite.getAlgebraicIndexOfBasisSystem()[3], (unsigned)1);
}

/// Check getnw
TEST_F(GeochemicalSystemTest, getSolventWaterMass)
{
  EXPECT_EQ(_egs_calcite.getSolventWaterMass(), 1.0);

  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::KG);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  const GeochemicalSystem egs(_mgd_calcite,
                              _ac3,
                              _is3,
                              _swapper4,
                              {},
                              {},
                              "H+",
                              {"H2O", "Calcite", "H+", "HCO3-"},
                              {2.5, 3.0, 2.0, 1.0},
                              cu,
                              cm,
                              25,
                              0,
                              1E-20,
                              {},
                              {},
                              {});
  EXPECT_EQ(egs.getSolventWaterMass(), 2.5);
}

/// Check getBulkMolesOld
TEST_F(GeochemicalSystemTest, getBulkMolesOld)
{
  EXPECT_EQ(_egs_calcite.getBulkMolesOld()[3], 3.0); // Calcite
  EXPECT_EQ(_egs_calcite.getBulkMolesOld()[1], 2.0); // H+

  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  const GeochemicalSystem egs(_mgd_calcite,
                              _ac3,
                              _is3,
                              _swapper4,
                              {},
                              {},
                              "H+",
                              {"H2O", "Calcite", "H+", "HCO3-"},
                              {-0.5, 3.5, 2.5, 1.0},
                              cu,
                              cm,
                              25,
                              0,
                              1E-20,
                              {},
                              {},
                              {});
  EXPECT_EQ(egs.getBulkMolesOld()[0], -0.5); // H2O
  EXPECT_EQ(egs.getBulkMolesOld()[1], 1.0);  // H+ : note that charge neutrality has forced this
  EXPECT_EQ(egs.getBulkMolesOld()[2], 1.0);  // HCO3-
  EXPECT_EQ(egs.getBulkMolesOld()[3], 3.5);  // Calcite
}

/// Check getSolventMassAndFreeMolalityAndMineralMoles and getAlgebraicVariableValues
TEST_F(GeochemicalSystemTest, getSolventMassAndFreeMolalityAndMineralMoles)
{
  EXPECT_EQ(_egs_calcite.getSolventMassAndFreeMolalityAndMineralMoles()[2], 1.0); // HCO3-
  EXPECT_EQ(_egs_calcite.getAlgebraicVariableValues()[0],
            _egs_calcite.getSolventMassAndFreeMolalityAndMineralMoles()[1]); // H+
  EXPECT_EQ(_egs_calcite.getAlgebraicBasisValues()[0],
            _egs_calcite.getSolventMassAndFreeMolalityAndMineralMoles()[1]); // H+
  EXPECT_EQ(_egs_calcite.getAlgebraicVariableDenseValues()(0),
            _egs_calcite.getSolventMassAndFreeMolalityAndMineralMoles()[1]); // H+

  EXPECT_EQ(_egs_kinetic_calcite.getSolventMassAndFreeMolalityAndMineralMoles()[2], 1.0); // HCO3-
  EXPECT_EQ(_egs_kinetic_calcite.getAlgebraicVariableValues()[0],
            _egs_kinetic_calcite.getSolventMassAndFreeMolalityAndMineralMoles()[1]); // H+
  EXPECT_EQ(_egs_kinetic_calcite.getAlgebraicBasisValues()[0],
            _egs_kinetic_calcite.getSolventMassAndFreeMolalityAndMineralMoles()[1]); // H+
  EXPECT_EQ(_egs_kinetic_calcite.getAlgebraicVariableDenseValues()(0),
            _egs_kinetic_calcite.getSolventMassAndFreeMolalityAndMineralMoles()[1]); // H+
  EXPECT_EQ(_egs_kinetic_calcite.getAlgebraicVariableValues()[1],
            _egs_kinetic_calcite.getSolventMassAndFreeMolalityAndMineralMoles()[3]); // Ca++
  EXPECT_EQ(_egs_kinetic_calcite.getAlgebraicBasisValues()[1],
            _egs_kinetic_calcite.getSolventMassAndFreeMolalityAndMineralMoles()[3]); // Ca++
  EXPECT_EQ(_egs_kinetic_calcite.getAlgebraicVariableDenseValues()(1),
            _egs_kinetic_calcite.getSolventMassAndFreeMolalityAndMineralMoles()[3]); // Ca++
  EXPECT_EQ(_egs_kinetic_calcite.getAlgebraicVariableValues()[2], 1.1);              // Calcite
  EXPECT_EQ(_egs_kinetic_calcite.getAlgebraicVariableDenseValues()(2), 1.1);         // Calcite

  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_MINERAL,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::KG);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  const GeochemicalSystem egs(_mgd_calcite,
                              _ac3,
                              _is3,
                              _swapper4,
                              {},
                              {},
                              "H+",
                              {"H2O", "Calcite", "H+", "HCO3-"},
                              {0.5, 3.5, 2.5, 1.0},
                              cu,
                              cm,
                              25,
                              0,
                              1E-20,
                              {},
                              {},
                              {});
  EXPECT_EQ(egs.getSolventMassAndFreeMolalityAndMineralMoles()[0], 0.5); // H2O
  EXPECT_EQ(egs.getSolventMassAndFreeMolalityAndMineralMoles()[2], 1.0); // HCO3-
  EXPECT_EQ(egs.getSolventMassAndFreeMolalityAndMineralMoles()[3], 3.5); // Calcite
  EXPECT_EQ(_egs_calcite.getAlgebraicVariableValues()[0],
            _egs_calcite.getSolventMassAndFreeMolalityAndMineralMoles()[1]); // H+
  EXPECT_EQ(_egs_calcite.getAlgebraicBasisValues()[0],
            _egs_calcite.getSolventMassAndFreeMolalityAndMineralMoles()[1]); // H+
  EXPECT_EQ(_egs_calcite.getAlgebraicVariableDenseValues()(0),
            _egs_calcite.getSolventMassAndFreeMolalityAndMineralMoles()[1]); // H+
}

/// Check getBasisActivityKnown
TEST_F(GeochemicalSystemTest, getBasisActivityKnown)
{
  EXPECT_EQ(_egs_calcite.getBasisActivityKnown()[0], true);  // H2O
  EXPECT_EQ(_egs_calcite.getBasisActivityKnown()[1], false); // H+
  EXPECT_EQ(_egs_calcite.getBasisActivityKnown()[2], false); // HCO3-
  EXPECT_EQ(_egs_calcite.getBasisActivityKnown()[3], true);  // Calcite

  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-", "O2(aq)"}, {}, {"O2(g)"}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FUGACITY);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);

  GeochemicalSystem egs(mgd,
                        _ac3,
                        _is3,
                        _swapper4,
                        {"O2(aq)"},
                        {"O2(g)"},
                        "H+",
                        {"H2O", "H+", "O2(g)", "HCO3-"},
                        {1.0, 2.0, 3.0, 4.0},
                        cu,
                        cm,
                        25,
                        0,
                        1E-20,
                        {},
                        {},
                        {});

  EXPECT_EQ(egs.getBasisActivityKnown()[0], false); // H2O
  EXPECT_EQ(egs.getBasisActivityKnown()[1], false); // H+
  EXPECT_EQ(egs.getBasisActivityKnown()[2], false); // HCO3-
  EXPECT_EQ(egs.getBasisActivityKnown()[3], true);  // O2(g)
}

/// Check getBasisActivity
TEST_F(GeochemicalSystemTest, getBasisActivity)
{
  EXPECT_EQ(_egs_calcite.getBasisActivity(0), 1.75);   // H2O
  EXPECT_EQ(_egs_calcite.getBasisActivity(3), 1.0);    // Calcite
  EXPECT_EQ(_egs_calcite.getBasisActivity()[0], 1.75); // H2O
  EXPECT_EQ(_egs_calcite.getBasisActivity()[3], 1.0);  // Calcite
  try
  {
    EXPECT_EQ(_egs_calcite.getBasisActivity(4), 1);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Cannot retrieve basis activity for species 4 since there are only 4 "
                         "basis species") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-", "O2(aq)"}, {}, {"O2(g)"}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FUGACITY);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);

  GeochemicalSystem egs(mgd,
                        _ac0,
                        _is0,
                        _swapper4,
                        {"O2(aq)"},
                        {"O2(g)"},
                        "H+",
                        {"H2O", "H+", "O2(g)", "HCO3-"},
                        {1.0, 1.5, 3.0, 2.5},
                        cu,
                        cm,
                        25,
                        0,
                        1E-20,
                        {},
                        {},
                        {}); // maximum ionic_strength = 0, so all activity_coefficients = 1

  EXPECT_EQ(egs.getBasisActivity(1), egs.getSolventMassAndFreeMolalityAndMineralMoles()[1]); // H+
  EXPECT_EQ(egs.getBasisActivity(2), 2.5);   // HCO3-
  EXPECT_EQ(egs.getBasisActivity(3), 3.0);   // O2(g)
  EXPECT_EQ(egs.getBasisActivity()[2], 2.5); // HCO3-
  EXPECT_EQ(egs.getBasisActivity()[3], 3.0); // O2(g)
}

/// Check getBasisActivityCoeff
TEST_F(GeochemicalSystemTest, getBasisActivityCoeff)
{
  try
  {
    EXPECT_EQ(_egs_calcite.getBasisActivityCoefficient(4), 1);
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

  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-", "O2(aq)"}, {}, {"O2(g)"}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FUGACITY);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);

  GeochemicalSystem egs(mgd,
                        _ac0,
                        _is0,
                        _swapper4,
                        {"O2(aq)"},
                        {"O2(g)"},
                        "H+",
                        {"H2O", "H+", "O2(g)", "HCO3-"},
                        {1.0, 1.5, 3.0, 2.5},
                        cu,
                        cm,
                        25,
                        0,
                        1E-20,
                        {},
                        {},
                        {}); // maximum ionic_strength = 0, so all activity_coefficients = 1

  EXPECT_EQ(egs.getBasisActivityCoefficient(1), 1.0);   // H+
  EXPECT_EQ(egs.getBasisActivityCoefficient(2), 1.0);   // HCO3-
  EXPECT_EQ(egs.getBasisActivityCoefficient()[1], 1.0); // H+
  EXPECT_EQ(egs.getBasisActivityCoefficient()[2], 1.0); // HCO3-
}

/// Check getEqmActivityCoefficient exception
TEST_F(GeochemicalSystemTest, getEqmActivityCoefficientException)
{
  try
  {
    EXPECT_EQ(_egs_calcite.getEquilibriumActivityCoefficient(44), 1);
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
TEST_F(GeochemicalSystemTest, getEqmMolality)
{
  try
  {
    EXPECT_EQ(_egs_calcite.getEquilibriumMolality(44), 1);
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
  EXPECT_NEAR(_egs_calcite.getEquilibriumMolality(0) /
                  (_egs_calcite.getBasisActivity(1) * _egs_calcite.getBasisActivity(2) /
                   _egs_calcite.getBasisActivity(0) /
                   _egs_calcite.getEquilibriumActivityCoefficient(0) /
                   std::pow(10.0, _egs_calcite.getLog10K(0))),
              1.0,
              1.0E-8);
  EXPECT_NEAR(_egs_calcite.getEquilibriumMolality()[0] /
                  (_egs_calcite.getBasisActivity(1) * _egs_calcite.getBasisActivity(2) /
                   _egs_calcite.getBasisActivity(0) /
                   _egs_calcite.getEquilibriumActivityCoefficient()[0] /
                   std::pow(10.0, _egs_calcite.getLog10K(0))),
              1.0,
              1.0E-8);
  // CO3-- = HCO3- - H+
  EXPECT_NEAR(_egs_calcite.getEquilibriumMolality(1) /
                  (_egs_calcite.getBasisActivity(2) / _egs_calcite.getBasisActivity(1) /
                   _egs_calcite.getEquilibriumActivityCoefficient(1) /
                   std::pow(10.0, _egs_calcite.getLog10K(1))),
              1.0,
              1.0E-8);
  EXPECT_NEAR(_egs_calcite.getEquilibriumMolality()[1] /
                  (_egs_calcite.getBasisActivity(2) / _egs_calcite.getBasisActivity(1) /
                   _egs_calcite.getEquilibriumActivityCoefficient()[1] /
                   std::pow(10.0, _egs_calcite.getLog10K(1))),
              1.0,
              1.0E-8);
  // CaCO3 = Calcite
  EXPECT_NEAR(_egs_calcite.getEquilibriumMolality(2) /
                  (1.0 / _egs_calcite.getEquilibriumActivityCoefficient(2) /
                   std::pow(10.0, _egs_calcite.getLog10K(2))),
              1.0,
              1.0E-8);
  EXPECT_NEAR(_egs_calcite.getEquilibriumMolality()[2] /
                  (1.0 / _egs_calcite.getEquilibriumActivityCoefficient()[2] /
                   std::pow(10.0, _egs_calcite.getLog10K(2))),
              1.0,
              1.0E-8);
  // CaOH+ = Calcite - HCO3- + H20
  EXPECT_NEAR(_egs_calcite.getEquilibriumMolality(3) /
                  (_egs_calcite.getBasisActivity(0) / _egs_calcite.getBasisActivity(2) /
                   _egs_calcite.getEquilibriumActivityCoefficient()[3] /
                   std::pow(10.0, _egs_calcite.getLog10K(3))),
              1.0,
              1.0E-8);
  EXPECT_NEAR(_egs_calcite.getEquilibriumMolality()[3] /
                  (_egs_calcite.getBasisActivity(0) / _egs_calcite.getBasisActivity(2) /
                   _egs_calcite.getEquilibriumActivityCoefficient(3) /
                   std::pow(10.0, _egs_calcite.getLog10K(3))),
              1.0,
              1.0E-8);
  // OH- = H20 - H+
  EXPECT_NEAR(_egs_calcite.getEquilibriumMolality(4) /
                  (_egs_calcite.getBasisActivity(0) / _egs_calcite.getBasisActivity(1) /
                   _egs_calcite.getEquilibriumActivityCoefficient(4) /
                   std::pow(10.0, _egs_calcite.getLog10K(4))),
              1.0,
              1.0E-8);
  EXPECT_NEAR(_egs_calcite.getEquilibriumMolality()[4] /
                  (_egs_calcite.getBasisActivity(0) / _egs_calcite.getBasisActivity(1) /
                   _egs_calcite.getEquilibriumActivityCoefficient()[4] /
                   std::pow(10.0, _egs_calcite.getLog10K(4))),
              1.0,
              1.0E-8);
  // Ca++ = Calcite - HCO3- + H+
  EXPECT_NEAR(_egs_calcite.getEquilibriumMolality(5) /
                  (_egs_calcite.getBasisActivity(1) / _egs_calcite.getBasisActivity(2) /
                   _egs_calcite.getEquilibriumActivityCoefficient(5) /
                   std::pow(10.0, _egs_calcite.getLog10K(5))),
              1.0,
              1.0E-8);
  EXPECT_NEAR(_egs_calcite.getEquilibriumMolality()[5] /
                  (_egs_calcite.getBasisActivity(1) / _egs_calcite.getBasisActivity(2) /
                   _egs_calcite.getEquilibriumActivityCoefficient()[5] /
                   std::pow(10.0, _egs_calcite.getLog10K(5))),
              1.0,
              1.0E-8);
}

/// Check computeAndGetEquilibriumActivity)
TEST_F(GeochemicalSystemTest, computeAndGetEquilibriumActivity)
{
  GeochemicalSystem nonconst = _egs_redox;
  const ModelGeochemicalDatabase & mgd = nonconst.getModelGeochemicalDatabase();
  const std::vector<Real> & eqm_m = nonconst.getEquilibriumMolality();
  const std::vector<Real> & eqm_ga = nonconst.getEquilibriumActivityCoefficient();
  const std::vector<Real> & eqm_act = nonconst.computeAndGetEquilibriumActivity();
  for (unsigned j = 0; j < nonconst.getNumInEquilibrium(); ++j)
  {
    EXPECT_EQ(eqm_act[j], nonconst.getEquilibriumActivity(j));
    if (mgd.eqm_species_mineral[j])
      EXPECT_EQ(eqm_act[j], 1.0);
    else if (mgd.eqm_species_name[j] == "O2(g)")
      EXPECT_NEAR(eqm_act[j],
                  nonconst.getBasisActivity(mgd.basis_species_index.at("O2(aq)")) /
                      std::pow(10.0, nonconst.getLog10K(j)),
                  1E-3);
    else if (mgd.eqm_species_name[j] == "CH4(g)fake")
      EXPECT_NEAR(
          eqm_act[j],
          std::pow(nonconst.getBasisActivity(mgd.basis_species_index.at("CH4(aq)")), 2.0) *
              std::pow(nonconst.getBasisActivity(mgd.basis_species_index.at("Fe+++")), -2.0) *
              std::pow(nonconst.getBasisActivity(mgd.basis_species_index.at("HCO3-")), 1.5) /
              std::pow(10.0, nonconst.getLog10K(j)),
          1E-3);
    else
      EXPECT_EQ(eqm_act[j], eqm_m[j] * eqm_ga[j]);
  }
}

/// Check getEquilibriumActivity execption
TEST_F(GeochemicalSystemTest, getEquilibriumActivityExeception)
{
  try
  {
    EXPECT_EQ(_egs_calcite.getEquilibriumActivity(6), 1);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("Cannot retrieve activity for equilibrium species 6 since there are only 6 "
                 "equilibrium species") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check getTotalChargeOld
TEST_F(GeochemicalSystemTest, getTotalChargeOld)
{
  const GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  const PertinentGeochemicalSystem model(
      database, {"H2O", "StoiCheckBasis", "Fe+++"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::KG);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  const GeochemicalSystem egs(mgd,
                              _ac3,
                              _is3,
                              _swapper3,
                              {},
                              {},
                              "StoiCheckBasis",
                              {"H2O", "Fe+++", "StoiCheckBasis"},
                              {1.75, 5.0, 1.0},
                              cu,
                              cm,
                              25,
                              0,
                              1E-20,
                              {},
                              {},
                              {});
  EXPECT_EQ(egs.getBulkMolesOld()[2], 5.0); // Fe+++
  EXPECT_NEAR(egs.getBulkMolesOld()[1],
              -6.0,
              1.0E-12); // StoiCheckBasis (charge=2.5) computed to ensure charge neutrality
  EXPECT_NEAR(egs.getTotalChargeOld(), 0.0, 1.0E-12);
}

/// Check getresidual
TEST_F(GeochemicalSystemTest, getResidual)
{
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  const GeochemicalSystem egs(_mgd_calcite,
                              _ac3,
                              _is3,
                              _swapper4,
                              {},
                              {},
                              "HCO3-",
                              {"H2O", "Calcite", "HCO3-", "H+"},
                              {175.0, 3.0, 2.0, 1.0},
                              cu,
                              cm,
                              25,
                              0,
                              1E-20,
                              {},
                              {},
                              {});
  // Here, water and HCO3- are the components in the algebraic system
  EXPECT_EQ(egs.getNumInAlgebraicSystem(), (unsigned)2);
  EXPECT_EQ(egs.getNumBasisInAlgebraicSystem(), (unsigned)2);
  EXPECT_EQ(egs.getNumSurfacePotentials(), (unsigned)0);

  const DenseVector<Real> mole_additions(4);

  try
  {
    EXPECT_EQ(egs.getResidualComponent(3, mole_additions), 2);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find(
            "Cannot retrieve residual for algebraic index 3 because there are only 2 "
            "molalities in the algebraic system and 0 surface potentials and 0 kinetic species") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    const DenseVector<Real> bad(5);
    EXPECT_EQ(egs.getResidualComponent(0, bad), 2);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("The increment in mole numbers (mole_additions) needs to be of size 4 + 0 "
                         "but it is of size 5") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  const Real nw = egs.getSolventMassAndFreeMolalityAndMineralMoles()[0];
  const Real mHCO3 = egs.getSolventMassAndFreeMolalityAndMineralMoles()[2];
  // H2O has algebraic index = 0 and basis index = 0
  Real res = -175.0 + nw * GeochemistryConstants::MOLES_PER_KG_WATER;
  for (unsigned j = 0; j < _mgd_calcite.eqm_species_name.size(); ++j)
    res += nw * _mgd_calcite.eqm_stoichiometry(j, 0) * egs.getEquilibriumMolality(j);
  EXPECT_NEAR(egs.getResidualComponent(0, mole_additions) / res, 1.0, 1.0E-12);
  // HCO3- has algebraic index = 1 and basis index = 2
  res = -egs.getBulkMolesOld()[1] +
        nw * mHCO3; // first term is because of charge balance (H+ has basis index 1)
  for (unsigned j = 0; j < _mgd_calcite.eqm_species_name.size(); ++j)
    res += nw * _mgd_calcite.eqm_stoichiometry(j, 2) * egs.getEquilibriumMolality(j);
  EXPECT_NEAR(egs.getResidualComponent(1, mole_additions) / res, 1.0, 1.0E-12);

  // The above is a good test of the indexing, but due to crazy log10K it is difficult to
  // demonstrate correct residual, so instead:
  PertinentGeochemicalSystem model2(
      _db_calcite, {"H2O", "H+", "StoiCheckBasis"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd2 = model2.modelGeochemicalDatabase();
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm2 = {
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu2;
  cu2.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu2.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu2.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  const GeochemicalSystem egs2(mgd2,
                               _ac3,
                               _is3,
                               _swapper3,
                               {},
                               {},
                               "StoiCheckBasis",
                               {"H2O", "H+", "StoiCheckBasis"},
                               {175.0, 8.0, 2.0},
                               cu2,
                               cm2,
                               25,
                               0,
                               1E-20,
                               {},
                               {},
                               {});
  DenseVector<Real> mole_add3(3);
  // Here, water and StoiCheckBasis are the components in the algebraic system
  EXPECT_EQ(egs2.getNumInAlgebraicSystem(), (unsigned)2);
  EXPECT_EQ(egs2.getNumBasisInAlgebraicSystem(), (unsigned)2);
  EXPECT_EQ(egs2.getNumSurfacePotentials(), (unsigned)0);
  // The equilibrium species are StoiCheckRedox (m = 9.82934e+06) and OH- (m = 4.78251e-15)
  const Real nw2 = egs2.getSolventMassAndFreeMolalityAndMineralMoles()[0];
  const Real mscb = egs2.getSolventMassAndFreeMolalityAndMineralMoles()[2];
  const Real mscr = egs2.getEquilibriumMolality(0);
  // H2O
  const Real resh2o = -175.0 + nw2 * (GeochemistryConstants::MOLES_PER_KG_WATER + (-0.5) * mscr);
  EXPECT_NEAR(egs2.getResidualComponent(0, mole_add3) / resh2o, 1.0, 1.0E-12);
  // StoiCheckBasis (algebraic index = 1, basis index = 2)
  const Real resscb = (1.0 / 2.5) * egs2.getBulkMolesOld()[1] +
                      nw2 * (mscb + 1.5 * mscr); // first term is from charge balance
  EXPECT_NEAR(egs2.getResidualComponent(1, mole_add3) / resscb, 1.0, 1.0E-12);

  for (unsigned i = 0; i < 3; ++i)
    mole_add3(i) =
        (i + 1) *
        1E6; // although moles of StoiCheckBasis are added, they are removed by charge balance
  EXPECT_NEAR((egs2.getResidualComponent(0, mole_add3) + 1E6) / resh2o, 1.0, 1.0E-12);
  EXPECT_NEAR(egs2.getResidualComponent(1, mole_add3) / resscb, 1.0, 1.0E-12);
}

/// check that  enforcing charge balance works
TEST_F(GeochemicalSystemTest, enforceChargeBalance)
{
  GeochemicalSystem nonconst = _egs_calcite;
  ASSERT_TRUE(std::abs(nonconst.getTotalChargeOld()) > 1.0); // initially there is nonzero charge
  nonconst.enforceChargeBalance();
  EXPECT_NEAR(nonconst.getTotalChargeOld(), 0.0, 1E-12);
}

/// Setting algebraic variables exception
TEST_F(GeochemicalSystemTest, setVarException)
{
  GeochemicalSystem egs(_mgd_calcite,
                        _ac3,
                        _is3,
                        _swapper4,
                        {},
                        {},
                        "H+",
                        {"H2O", "Calcite", "H+", "HCO3-"},
                        {1.75, 3.0, 2.0, 1.0},
                        _cu_calcite,
                        _cm_calcite,
                        25,
                        0,
                        1E-20,
                        {},
                        {},
                        {});

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
TEST_F(GeochemicalSystemTest, jac1)
{
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  GeochemicalSystem egs(
      _mgd_calcite,
      _ac8,
      _is8,
      _swapper4,
      {},
      {},
      "HCO3-",
      {"H2O", "Calcite", "HCO3-", "H+"},
      {175.0, 3.0, 3.2E-2, 1E-8}, // the molality of H+ is carefully chosen so the secondary species
                                  // do not contribute much to the ionic strengths
      cu,
      cm,
      25,
      4,
      1E-20,
      {},
      {},
      {});
  DenseVector<Real> mole_add(4);
  DenseMatrix<Real> dmole_add(4, 4);

  // Here, water and HCO3- are the components in the algebraic system
  EXPECT_EQ(egs.getNumInAlgebraicSystem(), (unsigned)2);
  EXPECT_EQ(egs.getNumBasisInAlgebraicSystem(), (unsigned)2);
  EXPECT_EQ(egs.getNumSurfacePotentials(), (unsigned)0);
  DenseVector<Real> res_orig(2);
  res_orig(0) = egs.getResidualComponent(0, mole_add);
  res_orig(1) = egs.getResidualComponent(1, mole_add);
  const std::vector<Real> var_orig =
      egs.getAlgebraicVariableValues(); // var[0] ~ 175/55 = 3.2 (kg of solvent water).  var[1]
                                        // ~ 3.2E-2/3.2 = 0.01 (molality of HCO3-)
  DenseMatrix<Real> jac(1, 1); // deliberately size incorrectly to check resizing done correctly
  egs.computeJacobian(res_orig, jac, mole_add, dmole_add);

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
      const Real expected =
          (egs.getResidualComponent(res_comp, mole_add) - res_orig(res_comp)) / eps;
      EXPECT_NEAR(jac(res_comp, var_num) / expected, 1.0, 1E-5);
    }
  }

  // now check the derivatives including mole_additions
  for (unsigned i = 0; i < 4; ++i)
    mole_add(i) = i + 1; // add 1 mole of solvent water and of 3 of HCO3-.  The HCO3- addition gets
                         // removed by charge-balance
  egs.setAlgebraicVariables(var_orig);
  res_orig(0) = egs.getResidualComponent(0, mole_add);
  res_orig(1) = egs.getResidualComponent(1, mole_add);
  for (unsigned i = 0; i < 4; ++i)
    for (unsigned j = 0; j < 4; ++j)
      dmole_add(i, j) = std::sin(i + 1.0) * (j + 1); // just some randomish derivatives
  egs.computeJacobian(res_orig, jac, mole_add, dmole_add);
  for (unsigned var_num = 0; var_num < 2; ++var_num)
  {
    std::vector<Real> var_new = var_orig;
    var_new[var_num] += eps;
    egs.setAlgebraicVariables(var_new);
    for (unsigned i = 0; i < 4; ++i)
    {
      if (var_num == 0)
        mole_add(i) = i + 1 + dmole_add(i, 0) * eps;
      else
        mole_add(i) = i + 1 + dmole_add(i, 2) * eps;
    }
    for (unsigned res_comp = 0; res_comp < 2; ++res_comp)
    {
      const Real expected =
          (egs.getResidualComponent(res_comp, mole_add) - res_orig(res_comp)) / eps;
      EXPECT_NEAR(jac(res_comp, var_num) / expected, 1.0, 1E-5);
    }
  }
}

/**
 * Check jacobian calculations when mass of solvent water is not a variable
 * Note that because the Jacobian calculations do not consider derivatives of activity or
 * activity coefficients, they are only roughly correct
 */
TEST_F(GeochemicalSystemTest, jac2)
{
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  GeochemicalSystem egs(_mgd_calcite,
                        _ac8,
                        _is8,
                        _swapper4,
                        {},
                        {},
                        "HCO3-",
                        {"H2O", "Calcite", "HCO3-", "H+"},
                        {1.0, 3.0, 3.2E-4, 1E-4},
                        cu,
                        cm,
                        25,
                        2,
                        1E-20,
                        {},
                        {},
                        {});
  DenseVector<Real> mole_add(4);
  DenseMatrix<Real> dmole_add(4, 4);

  // Here, H+ and HCO3- are the components in the algebraic system
  EXPECT_EQ(egs.getNumInAlgebraicSystem(), (unsigned)2);
  EXPECT_EQ(egs.getNumBasisInAlgebraicSystem(), (unsigned)2);
  EXPECT_EQ(egs.getNumSurfacePotentials(), (unsigned)0);
  DenseVector<Real> res_orig(2);
  res_orig(0) = egs.getResidualComponent(0, mole_add);
  res_orig(1) = egs.getResidualComponent(1, mole_add);
  const std::vector<Real> var_orig = egs.getAlgebraicVariableValues(); // both are 9E-5
  DenseMatrix<Real> jac(1, 1); // deliberately size incorrectly to check resizing done correctly
  egs.computeJacobian(res_orig, jac, mole_add, dmole_add);

  const Real eps = 1E-9;
  for (unsigned var_num = 0; var_num < 2; ++var_num)
  {
    std::vector<Real> var_new = var_orig;
    var_new[var_num] += eps;
    egs.setAlgebraicVariables(var_new);
    for (unsigned res_comp = 0; res_comp < 2; ++res_comp)
    {
      const Real expected =
          (egs.getResidualComponent(res_comp, mole_add) - res_orig(res_comp)) / eps;
      if (var_num == 0)
        EXPECT_NEAR(jac(res_comp, var_num) / expected,
                    1.0,
                    1.0E-10); // everything happens to be linear in H+
      else
        EXPECT_NEAR(jac(res_comp, var_num) / expected, 1.0, 1.0E-4); // nonlinear in HCO3-
    }
  }

  // now check the derivatives including mole_additions
  for (unsigned i = 0; i < 4; ++i)
    mole_add(i) = (i + 1) * 1E4;
  egs.setAlgebraicVariables(var_orig);
  res_orig(0) = egs.getResidualComponent(0, mole_add);
  res_orig(1) = egs.getResidualComponent(1, mole_add);
  for (unsigned i = 0; i < 4; ++i)
    for (unsigned j = 0; j < 4; ++j)
      dmole_add(i, j) = std::sin(i + 1.0) * (j + 1) * 1E5; // just some randomish derivatives
  egs.computeJacobian(res_orig, jac, mole_add, dmole_add);
  for (unsigned var_num = 0; var_num < 2; ++var_num)
  {
    std::vector<Real> var_new = var_orig;
    var_new[var_num] += eps;
    egs.setAlgebraicVariables(var_new);
    for (unsigned i = 0; i < 4; ++i)
    {
      if (var_num == 0)
        mole_add(i) = (i + 1) * 1E4 + dmole_add(i, 1) * eps; // change H+
      else
        mole_add(i) = (i + 1) * 1E4 + dmole_add(i, 2) * eps; // change HCO3-
    }
    for (unsigned res_comp = 0; res_comp < 2; ++res_comp)
    {
      const Real expected =
          (egs.getResidualComponent(res_comp, mole_add) - res_orig(res_comp)) / eps;
      if (var_num == 0)
        EXPECT_NEAR(jac(res_comp, var_num) / expected,
                    1.0,
                    1.0E-7); // everything happens to be linear in H+
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
TEST_F(GeochemicalSystemTest, jac3)
{
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  GeochemicalSystem egs(_mgd_calcite,
                        _ac8,
                        _is8,
                        _swapper4,
                        {},
                        {},
                        "H+",
                        {"H2O", "Calcite", "H+", "HCO3-"},
                        {1.0, 3.0, 3.2E-4, 1E-4},
                        cu,
                        cm,
                        25,
                        0,
                        1E-20,
                        {},
                        {},
                        {});
  DenseVector<Real> mole_add(4);
  DenseMatrix<Real> dmole_add(4, 4);

  // Here H+ is the only component in the algebraic system
  EXPECT_EQ(egs.getNumInAlgebraicSystem(), (unsigned)1);
  EXPECT_EQ(egs.getNumBasisInAlgebraicSystem(), (unsigned)1);
  EXPECT_EQ(egs.getNumSurfacePotentials(), (unsigned)0);
  DenseVector<Real> res_orig(1);
  res_orig(0) = egs.getResidualComponent(0, mole_add);
  const std::vector<Real> var_orig = egs.getAlgebraicVariableValues(); // around 3E-4
  DenseMatrix<Real> jac(1, 1);
  egs.computeJacobian(res_orig, jac, mole_add, dmole_add);

  const Real eps = 1E-11;
  std::vector<Real> var_new = var_orig;
  var_new[0] += eps;
  egs.setAlgebraicVariables(var_new);
  Real expected = (egs.getResidualComponent(0, mole_add) - res_orig(0)) / eps;
  EXPECT_NEAR(jac(0, 0) / expected, 1.0, 1.0E-7);

  // now check the derivatives including mole_additions
  // because of charge balance there is actually no dependence on mole_add
  for (unsigned i = 0; i < 4; ++i)
    mole_add(i) = i + 1;
  egs.setAlgebraicVariables(var_orig);
  res_orig(0) = egs.getResidualComponent(0, mole_add);
  for (unsigned i = 0; i < 4; ++i)
    for (unsigned j = 0; j < 4; ++j)
      dmole_add(i, j) = std::sin(i + 1.0) * (j + 1) * 1E10; // huge randomish derivatives
  egs.computeJacobian(res_orig, jac, mole_add, dmole_add);
  egs.setAlgebraicVariables(var_new);
  for (unsigned i = 0; i < 4; ++i)
    mole_add(i) = (i + 1) + dmole_add(i, 1) * eps; // change H+
  expected = (egs.getResidualComponent(0, mole_add) - res_orig(0)) / eps;
  EXPECT_NEAR(jac(0, 0) / expected, 1.0, 1.0E-7);
}

/**
 * Check jacobian calculations when mass of solvent water is a variable, and activity is fixed.
 * Note that because the Jacobian calculations do not consider derivatives of activity or
 * activity coefficients, they are only roughly correct
 */
TEST_F(GeochemicalSystemTest, jac4)
{
  const PertinentGeochemicalSystem model(
      _db_calcite, {"H2O", "H+", "Ca++", "HCO3-"}, {"Calcite"}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  GeochemicalSystem egs(mgd,
                        _ac8,
                        _is8,
                        _swapper4,
                        {},
                        {},
                        "H+",
                        {"H2O", "H+", "Ca++", "HCO3-"},
                        {175.0, 1E1, 4E1, 1E-4},
                        cu,
                        cm,
                        25,
                        0,
                        1E-20,
                        {},
                        {},
                        {});
  DenseVector<Real> mole_add(4);
  DenseMatrix<Real> dmole_add(4, 4);

  EXPECT_EQ(egs.getNumInAlgebraicSystem(), (unsigned)3);
  EXPECT_EQ(egs.getNumBasisInAlgebraicSystem(), (unsigned)3);
  EXPECT_EQ(egs.getNumSurfacePotentials(), (unsigned)0);
  DenseVector<Real> res_orig(3);
  res_orig(0) = egs.getResidualComponent(0, mole_add);
  res_orig(1) = egs.getResidualComponent(1, mole_add);
  res_orig(2) = egs.getResidualComponent(2, mole_add);
  const std::vector<Real> var_orig = egs.getAlgebraicVariableValues(); // around {3.1, 2.9, 11.4}
  DenseMatrix<Real> jac(1, 1);
  egs.computeJacobian(res_orig, jac, mole_add, dmole_add);

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
      const Real expected =
          (egs.getResidualComponent(res_comp, mole_add) - res_orig(res_comp)) / eps;
      if (std::abs(expected) < 1.0E-6)
        EXPECT_NEAR(jac(res_comp, var_num), 0.0, 1.0E-6);
      else
        EXPECT_NEAR(expected / jac(res_comp, var_num), 1.0, 1.0E-6);
    }
  }

  // now check the derivatives including mole_additions
  for (unsigned i = 0; i < 4; ++i)
    mole_add(i) = i + 1;
  egs.setAlgebraicVariables(var_orig);
  res_orig(0) = egs.getResidualComponent(0, mole_add);
  res_orig(1) = egs.getResidualComponent(1, mole_add);
  res_orig(2) = egs.getResidualComponent(2, mole_add);
  for (unsigned i = 0; i < 4; ++i)
    for (unsigned j = 0; j < 4; ++j)
      dmole_add(i, j) = 10 * std::sin(i + 1.0) * (j + 1); // just some randomish derivatives
  egs.computeJacobian(res_orig, jac, mole_add, dmole_add);
  for (unsigned var_num = 0; var_num < 3; ++var_num)
  {
    std::vector<Real> var_new = var_orig;
    var_new[var_num] += eps;
    egs.setAlgebraicVariables(var_new);
    for (unsigned i = 0; i < 4; ++i)
      mole_add(i) = i + 1 + dmole_add(i, var_num) * eps;
    for (unsigned res_comp = 0; res_comp < 3; ++res_comp)
    {
      const Real expected =
          (egs.getResidualComponent(res_comp, mole_add) - res_orig(res_comp)) / eps;
      EXPECT_NEAR(jac(res_comp, var_num) / expected, 1.0, 1E-7);
    }
  }
}

/// check saturation indices
TEST_F(GeochemicalSystemTest, saturationIndices)
{
  const PertinentGeochemicalSystem model(
      _db_calcite, {"H2O", "H+", "HCO3-", "Ca++"}, {"Calcite"}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm_fixed_activity = {
      GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
      GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu_fixed_activity;
  cu_fixed_activity.push_back(GeochemistryUnitConverter::GeochemistryUnit::KG);
  cu_fixed_activity.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu_fixed_activity.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu_fixed_activity.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  const GeochemicalSystem egs(mgd,
                              _ac3,
                              _is3,
                              _swapper4,
                              {},
                              {},
                              "H+",
                              {"H2O", "Ca++", "H+", "HCO3-"},
                              {1.11, 3.0, 2.0, 1.5},
                              cu_fixed_activity,
                              cm_fixed_activity,
                              25,
                              0,
                              1E-20,
                              {},
                              {},
                              {});
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
TEST_F(GeochemicalSystemTest, swapExceptions)
{
  GeochemicalSystem nonconst = _egs_calcite;
  try
  {
    nonconst.performSwap(0, 0);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("GeochemicalSystem: attempting to swap out water and replace it by CO2(aq)."
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
    ASSERT_TRUE(msg.find("GeochemicalSystem: attempting to swap the charge-balance "
                         "species out of the basis") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-", "O2(aq)"}, {}, {"O2(g)"}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);

  GeochemicalSystem egs(mgd,
                        _ac3,
                        _is3,
                        _swapper4,
                        {},
                        {},
                        "H+",
                        {"H2O", "H+", "O2(aq)", "HCO3-"},
                        {1.0, 1.5, 3.0, 2.5},
                        cu,
                        cm,
                        25,
                        0,
                        1E-20,
                        {},
                        {},
                        {});

  try
  {
    egs.performSwap(3, 5);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("GeochemicalSystem: attempting to swap a gas into the basis") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm2;
  cm2.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm2.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm2.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FUGACITY);
  cm2.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu2;
  cu2.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu2.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu2.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu2.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  GeochemicalSystem egs2(mgd,
                         _ac3,
                         _is3,
                         _swapper4,
                         {"O2(aq)"},
                         {"O2(g)"},
                         "H+",
                         {"H2O", "H+", "O2(g)", "HCO3-"},
                         {1.0, 1.5, 3.0, 2.5},
                         cu2,
                         cm2,
                         25,
                         0,
                         1E-20,
                         {},
                         {},
                         {});

  try
  {
    egs2.performSwap(3, 0);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("GeochemicalSystem: attempting to swap a gas out of the basis") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// check swap
TEST_F(GeochemicalSystemTest, swap)
{
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_MINERAL,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::KG);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  ModelGeochemicalDatabase mgd = _model_calcite.modelGeochemicalDatabase();
  GeochemicalSystem egs(mgd,
                        _ac3,
                        _is3,
                        _swapper4,
                        {"Ca++"},
                        {"Calcite"},
                        "H+",
                        {"H2O", "Calcite", "H+", "HCO3-"},
                        {1.75, 3.0, 2.0, 1.0},
                        cu,
                        cm,
                        25,
                        0,
                        1E-20,
                        {},
                        {},
                        {});

  EXPECT_EQ(egs.getNumInAlgebraicSystem(), (unsigned)1);
  EXPECT_EQ(egs.getNumBasisInAlgebraicSystem(), (unsigned)1);
  EXPECT_EQ(egs.getNumSurfacePotentials(), (unsigned)0);
  EXPECT_EQ(egs.getInAlgebraicSystem()[0], false);
  EXPECT_EQ(egs.getInAlgebraicSystem()[1], true);
  EXPECT_EQ(egs.getInAlgebraicSystem()[2], false);
  EXPECT_EQ(egs.getInAlgebraicSystem()[3], false);
  EXPECT_EQ(egs.getChargeBalanceBasisIndex(), (unsigned)1);
  const std::vector<Real> bm = egs.getBulkMolesOld();

  egs.performSwap(3, 5); // swap out Calcite and replace by Ca++

  EXPECT_EQ(egs.getNumInAlgebraicSystem(), (unsigned)2); // now Ca++ has a bulk moles attached to it
  EXPECT_EQ(egs.getNumBasisInAlgebraicSystem(),
            (unsigned)2); // now Ca++ has a bulk moles attached to it
  EXPECT_EQ(egs.getNumSurfacePotentials(), (unsigned)0);
  EXPECT_EQ(egs.getInAlgebraicSystem()[0], false);
  EXPECT_EQ(egs.getInAlgebraicSystem()[1], true);
  EXPECT_EQ(egs.getInAlgebraicSystem()[2], false);
  EXPECT_EQ(egs.getInAlgebraicSystem()[3], true);
  // Ca++ = Calcite - HCO3- + H+.  So, same number of bulk moles of Ca++ as Calcite, but number of
  // bulk moles of H+ = (original bulk moles of H+) - bulk moles of Calcite
  EXPECT_EQ(egs.getChargeBalanceBasisIndex(), (unsigned)1);
  EXPECT_NEAR(egs.getBulkMolesOld()[1], bm[1] - bm[3], 1.0E-7);
  EXPECT_NEAR(egs.getBulkMolesOld()[3], bm[3], 1.0E-7);
}

/// check get ionic strengths
TEST_F(GeochemicalSystemTest, getIS)
{
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::KG);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  GeochemistryIonicStrength is(0.0078125, 0.0078125, false, false);
  GeochemistryActivityCoefficientsDebyeHuckel ac(is, _db_calcite);
  ModelGeochemicalDatabase mgd = _model_calcite.modelGeochemicalDatabase();
  const GeochemicalSystem egs_small_IS(mgd,
                                       ac,
                                       is,
                                       _swapper4,
                                       {},
                                       {},
                                       "H+",
                                       {"H2O", "Ca++", "H+", "HCO3-"},
                                       {1.75, 3.0E-1, 2.0E-1, 1.0E-1},
                                       cu,
                                       cm,
                                       25,
                                       0,
                                       1E-20,
                                       {},
                                       {},
                                       {});
  EXPECT_EQ(egs_small_IS.getIonicStrength(), 0.0078125);
  EXPECT_EQ(egs_small_IS.getStoichiometricIonicStrength(), 0.0078125);

  GeochemistryIonicStrength is_false(3.0, 3.0, true, false);
  GeochemistryActivityCoefficientsDebyeHuckel ac_false(is_false, _db_calcite);
  const GeochemicalSystem egs(
      mgd,
      ac_false,
      is_false,
      _swapper4,
      {},
      {},
      "H+",
      {"H2O", "Ca++", "H+", "HCO3-"},
      {1.75, 1E-10, 1E-10, 1.0}, // up to 1E-10, the only contributor to IS is HCO3-
      cu,
      cm,
      25,
      0,
      1E-20,
      {},
      {},
      {});
  EXPECT_NEAR(egs.getIonicStrength(), 0.5, 1.0E-8);
  EXPECT_NEAR(egs.getStoichiometricIonicStrength(), 0.5, 1.0E-8);
}

/// Check alterChargeBalanceSpecies and revert
TEST_F(GeochemicalSystemTest, alterAndRevertChargeBalance)
{
  const GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  const PertinentGeochemicalSystem model(
      database, {"H2O", "StoiCheckBasis", "HCO3-"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::KG);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  GeochemicalSystem egs(mgd,
                        _ac3,
                        _is3,
                        _swapper3,
                        {},
                        {},
                        "StoiCheckBasis",
                        {"H2O", "StoiCheckBasis", "HCO3-"},
                        {1.75, 5.0, 1.0},
                        cu,
                        cm,
                        25,
                        0,
                        1E-20,
                        {},
                        {},
                        {});
  // in this case, charge-neutrality can be enforced immediately
  EXPECT_NEAR(egs.getTotalChargeOld(), 0.0, 1.0E-12);
  EXPECT_NEAR(egs.getBulkMolesOld()[1],
              1.0 / 2.5,
              1E-9); // this has been changed from 5.0 by the charge-balancing
  EXPECT_EQ(egs.getBulkMolesOld()[2], 1.0);
  EXPECT_EQ(egs.getChargeBalanceBasisIndex(), (unsigned)1);

  // StoiCheckBasis has molality 0.2, while HCO3- has molality 0.5, so can change the charge-balance
  // species via:
  egs.alterChargeBalanceSpecies(0.3); // now the charge-balance species should be HCO3-
  EXPECT_EQ(egs.getChargeBalanceBasisIndex(), (unsigned)2);
  EXPECT_EQ(egs.getBulkMolesOld()[1], 5.0);
  EXPECT_EQ(egs.getBulkMolesOld()[2],
            5 * 2.5); // this has been changed from 1.0 by the charge-balancing

  // revert to the original
  EXPECT_TRUE(egs.revertToOriginalChargeBalanceSpecies());
  EXPECT_EQ(egs.getChargeBalanceBasisIndex(), (unsigned)1);
  EXPECT_NEAR(egs.getBulkMolesOld()[1], 1.0 / 2.5, 1E-9);
  EXPECT_EQ(egs.getBulkMolesOld()[2], 1.0);
}

/// Check getNumRedox
TEST_F(GeochemicalSystemTest, getNumRedox) { EXPECT_EQ(_egs_redox.getNumRedox(), (unsigned)3); }

/// Check getOriginalRedoxLHS
TEST_F(GeochemicalSystemTest, getOriginalRedoxLHS)
{
  EXPECT_EQ(_egs_redox.getOriginalRedoxLHS(), "e-");
}

/// Check getRedoxLog10K
TEST_F(GeochemicalSystemTest, getRedoxLog10K)
{
  // not sure which order the redox has been ordered in.  The reactions are:
  // e- = (1/4/7.5)(O-phth)-- + (1/2 + 5/4/7.5)H2O + (-1 - 6/4/7.5)H+ - 8/4/7.5HCO3-
  // e- = (1/8)CH4(aq) + (1/2 - 1/8)H2O - (1+1/8)H+ - (1/8)HCO3-
  const bool ophth_is_slot_one = (_mgd_redox.redox_stoichiometry(1, 4) > 1.0E-6);
  const unsigned ophth_slot = (ophth_is_slot_one ? 1 : 2);
  const unsigned ch4_slot = (ophth_is_slot_one ? 2 : 1);
  Real boa = 1.0 / 4.0 / 7.5;
  EXPECT_NEAR(
      _egs_redox.getRedoxLog10K(ophth_slot), -boa * 542.8292 + 20.7757 - 0.25 * (-2.8990), 1E-8);
  boa = 1.0 / 8.0;
  EXPECT_NEAR(
      _egs_redox.getRedoxLog10K(ch4_slot), -boa * 144.1080 + 20.7757 - 0.25 * (-2.8990), 1E-8);
  try
  {
    _egs_redox.getRedoxLog10K(3);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find(
            "Cannot retrieve log10K for redox species 3 since there are only 3 redox species") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check log10RedoxActivityProduct
TEST_F(GeochemicalSystemTest, log10RedoxActivityProduct)
{
  // not sure which order the redox has been ordered in.  The reactions are:
  // e- = (1/4/7.5)(O-phth)-- + (1/2 + 5/4/7.5)H2O + (-1 - 6/4/7.5)H+ - 8/4/7.5HCO3-
  // e- = (1/8)CH4(aq) + (1/2 - 1/8)H2O - (1+1/8)H+ - (1/8)HCO3-
  const bool ophth_is_slot_one = (_mgd_redox.redox_stoichiometry(1, 4) > 1.0E-6);
  const unsigned ophth_slot = (ophth_is_slot_one ? 1 : 2);
  const unsigned ch4_slot = (ophth_is_slot_one ? 2 : 1);

  Real boa = 1.0 / 4.0 / 7.5;
  const Real log10ap_o =
      boa * std::log10(5.0) + (-1.0 - 6.0 * boa) * std::log10(2.0) - 8 * boa * std::log10(3.0);
  EXPECT_NEAR(_egs_redox.log10RedoxActivityProduct(ophth_slot), log10ap_o, 1E-8);

  boa = 1.0 / 8.0;
  const Real log10ap_c =
      boa * std::log10(6.0) - (1.0 + boa) * std::log10(2.0) - boa * std::log10(3.0);
  EXPECT_NEAR(_egs_redox.log10RedoxActivityProduct(ch4_slot), log10ap_c, 1E-8);
  try
  {
    _egs_redox.log10RedoxActivityProduct(3);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Cannot retrieve activity product for redox species 3 since there are "
                         "only 3 redox species") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check getSurfacePotential exception
TEST_F(GeochemicalSystemTest, getSurfacePotentialException)
{
  try
  {
    _egs_calcite.getSurfacePotential(0);
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
TEST_F(GeochemicalSystemTest, getSurfaceChargeException)
{
  try
  {
    _egs_calcite.getSurfaceCharge(0);
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
TEST_F(GeochemicalSystemTest, sorbingSurfaceArea)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
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
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_MINERAL};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::KG);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  GeochemicalSystem egs(mgd,
                        _ac3,
                        _is3,
                        _swapper5,
                        {"Fe+++"},
                        {"Fe(OH)3(ppd)"},
                        "H+",
                        {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe(OH)3(ppd)"},
                        {1.75, 1.0, 2.0, 1.0, 1.5},
                        cu,
                        cm,
                        25.0,
                        0,
                        1E-20,
                        {},
                        {},
                        {});
  EXPECT_EQ(egs.getSorbingSurfaceArea().size(), (std::size_t)1);
  EXPECT_EQ(egs.getSorbingSurfaceArea()[0], 1.5 * 106.8689 * 600.0);
}

/// Check surface potential things (not jacobian)
TEST_F(GeochemicalSystemTest, surfacePot)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
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
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::KG);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  const Real temp = 45.0;
  GeochemicalSystem egs(mgd,
                        _ac3,
                        _is3,
                        _swapper6,
                        {},
                        {},
                        "H+",
                        {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe+++", "HCO3-"},
                        {1.75, 1.0, 2.0, 3.0, 1.0, 1.0},
                        cu,
                        cm,
                        temp,
                        0,
                        1E-20,
                        {},
                        {},
                        {});
  const DenseVector<Real> mole_add(6);
  const DenseMatrix<Real> dmole_add(6, 6);

  // test basic things like sizes and that the algebraic variables are correctly set
  EXPECT_EQ(egs.getNumInAlgebraicSystem(), (unsigned)3);
  EXPECT_EQ(egs.getNumBasisInAlgebraicSystem(), (unsigned)2);
  EXPECT_EQ(egs.getNumSurfacePotentials(), (unsigned)1);
  EXPECT_EQ(egs.getBasisIndexOfAlgebraicSystem()[0],
            (unsigned)1); // H+ is slot=0 in algebraic system and slot=1 in basis
  EXPECT_EQ(egs.getBasisIndexOfAlgebraicSystem()[1],
            (unsigned)5); // HCO3-
  EXPECT_EQ(egs.getAlgebraicIndexOfBasisSystem()[1], (unsigned)0);
  EXPECT_EQ(egs.getAlgebraicIndexOfBasisSystem()[5], (unsigned)1);
  DenseVector<Real> alg_vars = egs.getAlgebraicVariableDenseValues();
  EXPECT_EQ(alg_vars.size(), (std::size_t)3);
  std::vector<Real> mols = egs.getAlgebraicBasisValues();
  EXPECT_EQ(mols.size(), (std::size_t)2);
  EXPECT_EQ(mols[0], alg_vars(0));
  EXPECT_EQ(mols[1], alg_vars(1));
  EXPECT_EQ(egs.getAlgebraicVariableValues().size(), (std::size_t)3);

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
  EXPECT_EQ(av.size(), (std::size_t)3);
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
  const Real pref = 0.5 / 96485.3415 *
                    std::sqrt(8.0 * 8.314472 * 318.15 * 8.8541878128E-12 * 78.5 * 1000.0 *
                              egs.getIonicStrength()) *
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
  EXPECT_NEAR(
      egs.getResidualComponent(2, mole_add), -pref * 600.0 + 1.75 * (-1.0) * mol_surf, 1.0E-9);
  try
  {
    egs.getResidualComponent(3, mole_add);
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
TEST_F(GeochemicalSystemTest, surfacePotJac)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
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
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION,
      GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  const Real temp = 45.0;
  GeochemistryIonicStrength is2(1E-2, 1E-2, false, false);
  GeochemistryActivityCoefficientsDebyeHuckel ac2(is2, database);
  GeochemicalSystem egs(mgd,
                        ac2,
                        is2,
                        _swapper6,
                        {},
                        {},
                        "H+",
                        {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe+++", "HCO3-"},
                        {1.75, 1.0, 2.0, 3.0, 0.5, 1.0},
                        cu,
                        cm,
                        temp,
                        0,
                        1E-20,
                        {},
                        {},
                        {});
  const DenseVector<Real> mole_add(6);
  const DenseMatrix<Real> dmole_add(6, 6);

  const unsigned num_alg = egs.getNumInAlgebraicSystem();
  DenseVector<Real> res_orig(num_alg);
  for (unsigned i = 0; i < num_alg; ++i)
    res_orig(i) = egs.getResidualComponent(i, mole_add);
  const std::vector<Real> var_orig = egs.getAlgebraicVariableValues();
  DenseMatrix<Real> jac(1, 1);
  egs.computeJacobian(res_orig, jac, mole_add, dmole_add);

  const Real eps = 1E-3;
  for (unsigned var_num = 0; var_num < num_alg; ++var_num)
  {
    std::vector<Real> var_new = var_orig;
    var_new[var_num] *= (1.0 + eps);
    egs.setAlgebraicVariables(var_new);
    for (unsigned res_comp = 0; res_comp < num_alg; ++res_comp)
    {
      const Real expected = (egs.getResidualComponent(res_comp, mole_add) - res_orig(res_comp)) /
                            (var_new[var_num] - var_orig[var_num]);
      if (std::abs(expected) < 1.0E-7)
        EXPECT_NEAR(jac(res_comp, var_num), 0.0, 1.0E-7);
      else
        EXPECT_NEAR(expected / jac(res_comp, var_num), 1.0, 1.0E-2);
    }
  }
}

/// Check jacobian when there is a surface potential and kinetic species
TEST_F(GeochemicalSystemTest, bigJac)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe+++", "HCO3-"},
                                   {"Fe(OH)3(ppd)"},
                                   {},
                                   {"Something", "Fe(OH)3(ppd)fake"},
                                   {},
                                   {},
                                   "O2(aq)",
                                   "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION_WITH_KINETIC,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION_WITH_KINETIC,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION_WITH_KINETIC,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION,
      GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION_WITH_KINETIC};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> ku;
  ku.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  ku.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);

  const Real temp = 45.0;
  GeochemistryIonicStrength is2(1E-2, 1E-2, false, false);
  GeochemistryActivityCoefficientsDebyeHuckel ac2(is2, database);
  GeochemicalSystem egs(mgd,
                        ac2,
                        is2,
                        _swapper6,
                        {},
                        {},
                        "H+",
                        {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe+++", "HCO3-"},
                        {1.75, 1.0, 2.0, 3.0, 0.5, 1.0},
                        cu,
                        cm,
                        temp,
                        0,
                        1E-20,
                        {"Something", "Fe(OH)3(ppd)fake"},
                        {4.4, 5.5},
                        ku);
  const DenseVector<Real> mole_add(8);
  const DenseMatrix<Real> dmole_add(8, 8);

  const unsigned num_alg = egs.getNumInAlgebraicSystem();
  DenseVector<Real> res_orig(num_alg);
  for (unsigned i = 0; i < num_alg; ++i)
    res_orig(i) = egs.getResidualComponent(i, mole_add);
  const std::vector<Real> var_orig = egs.getAlgebraicVariableValues();
  DenseMatrix<Real> jac(1, 1);
  egs.computeJacobian(res_orig, jac, mole_add, dmole_add);

  const Real eps = 1E-3;
  for (unsigned var_num = 0; var_num < num_alg; ++var_num)
  {
    std::vector<Real> var_new = var_orig;
    var_new[var_num] *= (1.0 + eps);
    egs.setAlgebraicVariables(var_new);
    for (unsigned res_comp = 0; res_comp < num_alg; ++res_comp)
    {
      const Real expected = (egs.getResidualComponent(res_comp, mole_add) - res_orig(res_comp)) /
                            (var_new[var_num] - var_orig[var_num]);
      if (std::abs(expected) < 1.0E-7)
        EXPECT_NEAR(jac(res_comp, var_num), 0.0, 1.0E-7);
      else
        EXPECT_NEAR(expected / jac(res_comp, var_num), 1.0, 1.0E-2);
    }
  }
}

/// Check jacobian when there is a surface potential and kinetic species and mole additions
TEST_F(GeochemicalSystemTest, bigJac2)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe+++", "HCO3-"},
                                   {"Fe(OH)3(ppd)"},
                                   {},
                                   {"Something", "Fe(OH)3(ppd)fake"},
                                   {},
                                   {},
                                   "O2(aq)",
                                   "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION_WITH_KINETIC,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION_WITH_KINETIC,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION_WITH_KINETIC,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION,
      GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION_WITH_KINETIC};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> ku;
  ku.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  ku.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  const Real temp = 45.0;
  GeochemistryIonicStrength is2(1E-2, 1E-2, false, false);
  GeochemistryActivityCoefficientsDebyeHuckel ac2(is2, database);
  GeochemicalSystem egs(mgd,
                        ac2,
                        is2,
                        _swapper6,
                        {},
                        {},
                        "H+",
                        {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe+++", "HCO3-"},
                        {1.75, 1.0, 2.0, 3.0, 0.5, 1.0},
                        cu,
                        cm,
                        temp,
                        0,
                        1E-20,
                        {"Something", "Fe(OH)3(ppd)fake"},
                        {4.4, 5.5},
                        ku);
  DenseVector<Real> mole_add(8);
  for (unsigned i = 0; i < 8; ++i)
    mole_add(i) = 1.0 + i;
  DenseMatrix<Real> dmole_add(8, 8);
  dmole_add(0, 0) = 1E15;
  for (unsigned i = 0; i < 8; ++i)
    for (unsigned j = 0; j < 8; ++j)
      dmole_add(i, j) = std::sin(i + 1.0) * (j + 1); // just some randomish derivatives

  const unsigned num_alg = egs.getNumInAlgebraicSystem();
  DenseVector<Real> res_orig(num_alg);
  for (unsigned i = 0; i < num_alg; ++i)
    res_orig(i) = egs.getResidualComponent(i, mole_add);
  const std::vector<Real> var_orig = egs.getAlgebraicVariableValues();
  DenseMatrix<Real> jac(1, 1);
  egs.computeJacobian(res_orig, jac, mole_add, dmole_add);

  const Real eps = 1E-3;
  for (unsigned var_num = 0; var_num < num_alg; ++var_num)
  {
    std::vector<Real> var_new = var_orig;
    var_new[var_num] *= (1.0 + eps);
    const Real del = var_new[var_num] - var_orig[var_num];
    egs.setAlgebraicVariables(var_new);
    for (unsigned i = 0; i < 8; ++i)
    {
      if (var_num < 3) // H2O, H+, >(s)FeOH
        mole_add(i) = 1.0 + i + dmole_add(i, var_num) * del;
      else if (var_num == 3) // HCO3-
        mole_add(i) = 1.0 + i + dmole_add(i, 5) * del;
      else if (var_num == 4) // surface potential
        mole_add(i) = 1.0 + i;
      else if (var_num == 5 || var_num == 6) // kinetic species
        mole_add(i) = 1.0 + i + dmole_add(i, var_num + 1) * del;
    }

    for (unsigned res_comp = 0; res_comp < num_alg; ++res_comp)
    {
      const Real expected =
          (egs.getResidualComponent(res_comp, mole_add) - res_orig(res_comp)) / del;
      if (std::abs(expected) < 1.0E-7)
        EXPECT_NEAR(jac(res_comp, var_num), 0.0, 1.0E-7);
      else
        EXPECT_NEAR(expected / jac(res_comp, var_num), 1.0, 1.0E-2);
    }
  }
}

/// Check setTemperature and getTemperature
TEST_F(GeochemicalSystemTest, setGetTemperature)
{
  GeochemicalSystem nonconst = _egs_calcite;
  EXPECT_EQ(nonconst.getTemperature(), 25.0);
  nonconst.setTemperature(40.0);
  EXPECT_EQ(nonconst.getTemperature(), 40.0);
}

/// Check exceptions for setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePots
TEST_F(GeochemicalSystemTest, setMolalitiesExcept1)
{
  GeochemicalSystem nonconst = _egs_calcite;

  try
  {
    nonconst.setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
        {"H2O", "H+", "HCO3-", "Ca++", "Calcite", "CO2(aq)", "CO3--", "CaCO3", "CaOH+", "OH-"},
        {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9},
        {false, true, true, true});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("When setting all molalities, names and values must have same size") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    nonconst.setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
        {"H2O", "H+", "HCO3-", "Ca++", "Calcite", "CO2(aq)", "CO3--", "CaCO3", "CaOH+"},
        {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9},
        {false, true, true, true});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find(
            "When setting all molalities, values must be provided for every species and surface "
            "potentials") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    nonconst.setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
        {"H2O", "H+", "HCO3-", "Ca++", "Calcite", "CO2(aq)", "CO3--", "CaCO3", "CaOH+", "OH-"},
        {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1},
        {false, true, true});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("constraints_from_molalities has size 3 while number of basis species is 4") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    nonconst.setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
        {"H2O", "H+", "HCO3-", "Ca++", "Calcite", "CO2(aq)", "CO3--", "CaCO3", "CaOH+", "OH-"},
        {1.1, 2.2, 3.3, 4.4, -1.0, 6.6, 7.7, 8.8, 9.9, 10.1},
        {false, true, true, true});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Molality for mineral Calcite cannot be -1: it must be non-negative") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    nonconst.setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
        {"H2O", "H+", "HCO3-", "Ca++", "Calcite", "CO2(aq)", "CO3--", "CaCO3", "CaOH+", "OH-"},
        {1.1, 2.2, 3.3, -1.0, 1.0, 6.6, 7.7, 8.8, 9.9, 10.1},
        {false, true, true, true});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Molality for species Ca++ cannot be -1: it must be non-negative") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    nonconst.setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
        {"H2O", "H+", "HCO3-", "Ca++", "Calcite", "CO2(aq)", "CO3--", "CaCO3", "CaOH+", "OH-"},
        {1.1, 0.0, 3.3, 1.0, 1.0, 6.6, 7.7, 8.8, 9.9, 10.1},
        {false, true, true, true});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Molality for species H+ cannot be 0: it must be positive") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    nonconst.setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
        {"OH-", "H+", "HCO3-", "Ca++", "Calcite", "CO2(aq)", "CO3--", "CaCO3", "CaOH+", "OH-"},
        {1.1, 2.2, 3.3, 4.4, 1.0, 6.6, 7.7, 8.8, 9.9, 10.1},
        {false, true, true, true});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Molality (or free mineral moles, etc - whatever is appropriate) for "
                         "species H2O needs to be provided when setting all molalities") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    nonconst.setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
        {"H2O", "H+", "HCO3-", "Ca++", "Calcite", "CO2(aq)", "CO3--", "CaCO3", "CaOH+", "OH-"},
        {1.1, 2.2, 3.3, 4.4, 1.0, -1.0, 7.7, 8.8, 9.9, 10.1},
        {false, true, true, true});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Molality for species CO2(aq) cannot be -1: it must be non-negative") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    nonconst.setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
        {"H2O", "H+", "HCO3-", "Ca++", "Calcite", "CO2(aq)", "CO2(aq)", "CaCO3", "CaOH+", "OH-"},
        {1.1, 2.2, 3.3, 4.4, 1.0, 6.6, 7.7, 8.8, 9.9, 10.1},
        {false, true, true, true});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("Molality for species CO3-- needs to be provided when setting all molalities") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    nonconst.setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
        {"H2O", "H+", "HCO3-", "Ca++", "Calcite", "CO2(aq)", "CO3--", "CaCO3", "CaOH+", "OH-"},
        {1.1, 2.2, 3.3, 4.4, 1.0, 6.6, 7.7, 8.8, 9.9, 10.1},
        {true, true, true, true});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("Water activity cannot be determined from molalities: constraints_from_molalities "
                 "must be set to false for water if activity of water is fixed") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check exceptions for setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePots
TEST_F(GeochemicalSystemTest, setMolalitiesExcept2)
{
  const PertinentGeochemicalSystem model_gas_and_sorption(
      _db_calcite,
      {"H2O", "H+", "HCO3-", "O2(aq)", "Fe+++", ">(s)FeOH", ">(w)FeOH"},
      {"Goethite"},
      {"O2(g)"},
      {},
      {},
      {},
      "O2(aq)",
      "e-");
  ModelGeochemicalDatabase mgd = model_gas_and_sorption.modelGeochemicalDatabase();
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FUGACITY,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::KG);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  GeochemicalSystem nonconst(mgd,
                             _ac3,
                             _is3,
                             _swapper7,
                             {"O2(aq)"},
                             {"O2(g)"},
                             "H+",
                             {"H2O", "H+", "HCO3-", "O2(g)", "Fe+++", ">(s)FeOH", ">(w)FeOH"},
                             {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0},
                             cu,
                             cm,
                             25,
                             0,
                             1E-20,
                             {},
                             {},
                             {});

  try
  {
    nonconst.setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
        {"H2O",
         "H+",
         "HCO3-",
         "O2(g)",
         "Fe+++",
         ">(s)FeOH",
         ">(w)FeOH",
         "(O-phth)--",
         "CH4(aq)",
         "CO2(aq)",
         "CO3--",
         "OH-",
         ">(s)FeO-",
         "Goethite",
         "O2(aq)"},
        {1.1, 2.2, 3.3, 0.0, 5.5, 6.6, 7.7, 8.8, 9.9, 10, 11, 12, 14, 15, 16},
        {false, true, true, false, true, true, true});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("When setting all molalities, values must be provided for every species "
                         "and surface potentials") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    nonconst.setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
        {"H2O",
         "H+",
         "HCO3-",
         "O2(g)",
         "Fe+++",
         ">(s)FeOH",
         ">(w)FeOH",
         "(O-phth)--",
         "CH4(aq)",
         "CO2(aq)",
         "CO3--",
         "OH-",
         ">(s)FeO-",
         "Goethite",
         "O2(aq)",
         "Goethite_surface_potential_expr"},
        {1.1, 2.2, 3.3, 1.0, 5.5, 6.6, 7.7, 8.8, 9.9, 10, 11, 12, 14, 15, 16, 17},
        {false, true, true, false, true, true, true});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Molality for gas O2(g) cannot be 1: it must be zero") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    nonconst.setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
        {"H2O",
         "H+",
         "HCO3-",
         "O2(g)",
         "Fe+++",
         ">(s)FeOH",
         ">(w)FeOH",
         "(O-phth)--",
         "CH4(aq)",
         "CO2(aq)",
         "CO3--",
         "OH-",
         ">(s)FeO-",
         "Goethite",
         "O2(aq)",
         "Goethite_surface_potential_expr"},
        {1.1, 2.2, 3.3, 0.0, 5.5, 6.6, 7.7, 8.8, 9.9, 10, 11, 12, 14, 15, -1.0, 17},
        {false, true, true, false, true, true, true});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Molality for species O2(aq) cannot be -1: it must be non-negative") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    nonconst.setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
        {"H2O",
         "H+",
         "HCO3-",
         "O2(g)",
         "Fe+++",
         ">(s)FeOH",
         ">(w)FeOH",
         "(O-phth)--",
         "CH4(aq)",
         "CO2(aq)",
         "CO3--",
         "OH-",
         ">(s)FeO-",
         "Goethite",
         "O2(aq)",
         "Goethite"},
        {1.1, 2.2, 3.3, 0.0, 5.5, 6.6, 7.7, 8.8, 9.9, 10, 11, 12, 14, 15, 16, 17},
        {false, true, true, false, true, true, true});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Surface potential for mineral Goethite needs to be provided when setting "
                         "all molalities") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    nonconst.setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
        {"H2O",
         "H+",
         "HCO3-",
         "O2(g)",
         "Fe+++",
         ">(s)FeOH",
         ">(w)FeOH",
         "(O-phth)--",
         "CH4(aq)",
         "CO2(aq)",
         "CO3--",
         "OH-",
         ">(s)FeO-",
         "Goethite",
         "O2(aq)",
         "Goethite_surface_potential_expr"},
        {1.1, 2.2, 3.3, 0.0, 5.5, 6.6, 7.7, 8.8, 9.9, 10, 11, 12, 14, 15, 16, 0},
        {false, true, true, false, true, true, true});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Surface-potential expression for mineral Goethite cannot be 0: it must "
                         "be positive") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    nonconst.setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
        {"H2O",
         "H+",
         "HCO3-",
         "O2(g)",
         "Fe+++",
         ">(s)FeOH",
         ">(w)FeOH",
         "(O-phth)--",
         "CH4(aq)",
         "CO2(aq)",
         "CO3--",
         "OH-",
         ">(s)FeO-",
         "Goethite",
         "O2(aq)",
         "Goethite_surface_potential_expr"},
        {1.1, 2.2, 3.3, 0.0, 5.5, 6.6, 7.7, 8.8, 9.9, 10, 11, 12, 14, 15, 16, 17},
        {false, true, true, true, true, true, true});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Gas fugacity cannot be determined from molality: "
                         "constraints_from_molalities must be set false for all gases") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check exceptions for setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePots
TEST_F(GeochemicalSystemTest, setMolalitiesExcept3)
{
  GeochemicalSystem nonconst = _egs_kinetic_calcite;
  try
  {
    nonconst.setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
        {"H2O", "H+", "HCO3-", "Ca++", "CO2(aq)", "CO3--", "CaCO3", "CaOH+", "OH-", "bad"},
        {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.0},
        {false, true, true, true});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("Moles for species Calcite needs to be provided when setting all molalities") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    nonconst.setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
        {"H2O", "H+", "HCO3-", "Ca++", "CO2(aq)", "CO3--", "CaCO3", "CaOH+", "OH-", "Calcite"},
        {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 0.0},
        {false, true, true, true});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Mole number for kinetic species must be positive, not 0") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePots
TEST_F(GeochemicalSystemTest, setMolalities1)
{
  const PertinentGeochemicalSystem model_gas_and_sorption(
      _db_calcite,
      {"H2O", "H+", "HCO3-", "O2(aq)", "Fe+++", ">(s)FeOH", ">(w)FeOH"},
      {"Goethite"},
      {"O2(g)"},
      {},
      {},
      {},
      "O2(aq)",
      "e-");
  ModelGeochemicalDatabase mgd = model_gas_and_sorption.modelGeochemicalDatabase();
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
      GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FUGACITY,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::KG);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  GeochemicalSystem nonconst(mgd,
                             _ac0,
                             _is0,
                             _swapper7,
                             {"O2(aq)"},
                             {"O2(g)"},
                             "Fe+++",
                             {"H2O", "H+", "HCO3-", "O2(g)", "Fe+++", ">(s)FeOH", ">(w)FeOH"},
                             {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0},
                             cu,
                             cm,
                             25,
                             0,
                             1E-20,
                             {},
                             {},
                             {});
  const std::vector<Real> bulk_before_set = nonconst.getBulkMolesOld();
  const std::vector<Real> molal_before_set =
      nonconst.getSolventMassAndFreeMolalityAndMineralMoles();

  const std::vector<std::string> gold_name = {"H2O",
                                              "H+",
                                              "HCO3-",
                                              "O2(g)",
                                              "Fe+++",
                                              ">(s)FeOH",
                                              ">(w)FeOH",
                                              "(O-phth)--",
                                              "CH4(aq)",
                                              "CO2(aq)",
                                              "CO3--",
                                              "OH-",
                                              ">(s)FeO-",
                                              "Goethite",
                                              "O2(aq)",
                                              "Goethite_surface_potential_expr"};
  const std::vector<Real> set_molal = {
      1.1, 2.2, 3.3, 0.0, 5.5, 6.6, 7.7, 8.8, 9.9, 10, 11, 12, 14, 15, 16, 17};
  nonconst.setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
      gold_name, set_molal, {true, true, true, false, false, true, false});

  const unsigned num_basis = nonconst.getNumInBasis();
  const unsigned num_eqm = nonconst.getNumInEquilibrium();

  // check molalities:
  std::vector<Real> gold_molal = set_molal;
  gold_molal[6] = molal_before_set[6]; // >(w)FeOH has a FREE_MOLALITY constraint and the
                                       // setSolvent... method has false
  gold_molal[13] = 0.0; // the setSolvent... method explicitly sets secondary mineral molality zero
  const std::vector<Real> molal = nonconst.getSolventMassAndFreeMolalityAndMineralMoles();
  for (unsigned i = 0; i < num_basis; ++i)
    EXPECT_EQ(molal[mgd.basis_species_index[gold_name[i]]], gold_molal[i]);
  for (unsigned j = num_basis; j < num_basis + num_eqm; ++j)
    EXPECT_EQ(nonconst.getEquilibriumMolality(mgd.eqm_species_index[gold_name[j]]), gold_molal[j]);
  EXPECT_NEAR(nonconst.getSurfacePotential(0),
              -2.0 * GeochemistryConstants::GAS_CONSTANT *
                  (25.0 + GeochemistryConstants::CELSIUS_TO_KELVIN) /
                  GeochemistryConstants::FARADAY * std::log(17.0),
              1.0E-8);
  EXPECT_EQ(nonconst.getSolventWaterMass(), 1.1);

  // check activities
  const std::vector<bool> act_known_gold = {false, true, false, true, false, false, false};
  const std::vector<bool> act_known = nonconst.getBasisActivityKnown();
  for (unsigned i = 0; i < num_basis; ++i)
    EXPECT_EQ(act_known[i], act_known_gold[i]);
  EXPECT_EQ(nonconst.getBasisActivity(1),
            2.2); // H+: activity changed by setSolvent... (activity coeffs = 1)
  EXPECT_EQ(nonconst.getBasisActivity(3), 4.0); // O2(g): fugacity unchanged by setSolvent...

  // check sorbing surface area
  EXPECT_EQ(nonconst.getSorbingSurfaceArea()[0], 60.0); // Goethite is not in the basis

  // check bulk moles
  const std::vector<Real> bulk = nonconst.getBulkMolesOld();
  for (unsigned i = 0; i < num_basis; ++i)
  {
    Real b = 0.0;
    if (i == 0)
      b = GeochemistryConstants::MOLES_PER_KG_WATER;
    else
      b = gold_molal[i];
    for (unsigned j = 0; j < num_eqm; ++j)
      b += mgd.eqm_stoichiometry(j, i) * nonconst.getEquilibriumMolality(j);
    b *= gold_molal[0];
    if (i == 4)
      EXPECT_EQ(bulk[4], bulk_before_set[4]); // Fe+++ is set by original constraints
    else
      EXPECT_NEAR(bulk[mgd.basis_species_index[gold_name[i]]] / b, 1.0, 1.0E-8);
  }
}

/// Check setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePots
TEST_F(GeochemicalSystemTest, setMolalities2)
{
  const PertinentGeochemicalSystem model_gas_and_sorption(
      _db_calcite,
      {"H2O", "H+", "HCO3-", "O2(aq)", "Fe+++", ">(s)FeOH", ">(w)FeOH"},
      {"Goethite"},
      {"O2(g)"},
      {},
      {},
      {},
      "O2(aq)",
      "e-");
  ModelGeochemicalDatabase mgd = model_gas_and_sorption.modelGeochemicalDatabase();
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FUGACITY,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  GeochemicalSystem nonconst(mgd,
                             _ac3,
                             _is3,
                             _swapper7,
                             {"O2(aq)"},
                             {"O2(g)"},
                             "Fe+++",
                             {"H2O", "H+", "HCO3-", "O2(g)", "Fe+++", ">(s)FeOH", ">(w)FeOH"},
                             {11.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0},
                             cu,
                             cm,
                             25,
                             0,
                             1E-20,
                             {},
                             {},
                             {});
  const std::vector<Real> bulk_before_set = nonconst.getBulkMolesOld();
  const std::vector<Real> molal_before_set =
      nonconst.getSolventMassAndFreeMolalityAndMineralMoles();

  const std::vector<std::string> gold_name = {"H2O",
                                              "H+",
                                              "HCO3-",
                                              "O2(g)",
                                              "Fe+++",
                                              ">(s)FeOH",
                                              ">(w)FeOH",
                                              "(O-phth)--",
                                              "CH4(aq)",
                                              "CO2(aq)",
                                              "CO3--",
                                              "OH-",
                                              ">(s)FeO-",
                                              "Goethite",
                                              "O2(aq)",
                                              "Goethite_surface_potential_expr"};
  const std::vector<Real> set_molal = {
      1.1, 2.2, 3.3, 0.0, 5.5, 6.6, 7.7, 8.8, 9.9, 10, 11, 12, 14, 15, 16, 17};
  nonconst.setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
      gold_name, set_molal, {false, false, true, false, false, true, false});

  const unsigned num_basis = nonconst.getNumInBasis();
  const unsigned num_eqm = nonconst.getNumInEquilibrium();

  // check molalities:
  std::vector<Real> gold_molal = set_molal;
  gold_molal[1] =
      molal_before_set[1]; // H+ has an ACTIVITY constraint and the setSolvent... method has false
  gold_molal[6] = molal_before_set[6]; // >(w)FeOH has a FREE_MOLALITY constraint and the
                                       // setSolvent... method has false
  gold_molal[13] = 0.0; // the setSolvent... method explicitly sets secondary mineral molality zero
  const std::vector<Real> molal = nonconst.getSolventMassAndFreeMolalityAndMineralMoles();
  for (unsigned i = 0; i < num_basis; ++i)
    EXPECT_EQ(molal[mgd.basis_species_index[gold_name[i]]], gold_molal[i]);
  for (unsigned j = num_basis; j < num_basis + num_eqm; ++j)
    EXPECT_EQ(nonconst.getEquilibriumMolality(mgd.eqm_species_index[gold_name[j]]), gold_molal[j]);
  EXPECT_NEAR(nonconst.getSurfacePotential(0),
              -2.0 * GeochemistryConstants::GAS_CONSTANT *
                  (25.0 + GeochemistryConstants::CELSIUS_TO_KELVIN) /
                  GeochemistryConstants::FARADAY * std::log(17.0),
              1.0E-8);
  EXPECT_EQ(nonconst.getSolventWaterMass(), 1.1);

  // check activities
  const std::vector<bool> act_known_gold = {false, true, false, true, false, false, false};
  const std::vector<bool> act_known = nonconst.getBasisActivityKnown();
  for (unsigned i = 0; i < num_basis; ++i)
    EXPECT_EQ(act_known[i], act_known_gold[i]);
  EXPECT_EQ(nonconst.getBasisActivity(1),
            2.0); // H+: activity unchanged by setSolvent... (activity coeffs = 1)
  EXPECT_EQ(nonconst.getBasisActivity(3), 4.0); // O2(g): fugacity unchanged by setSolvent...

  // check sorbing surface area
  EXPECT_EQ(nonconst.getSorbingSurfaceArea()[0], 60.0); // Goethite is not in the basis

  // check bulk moles
  const std::vector<Real> bulk = nonconst.getBulkMolesOld();
  for (unsigned i = 0; i < num_basis; ++i)
  {
    Real b = 0.0;
    if (i == 0)
      b = GeochemistryConstants::MOLES_PER_KG_WATER;
    else
      b = gold_molal[i];
    for (unsigned j = 0; j < num_eqm; ++j)
      b += mgd.eqm_stoichiometry(j, i) * nonconst.getEquilibriumMolality(j);
    b *= gold_molal[0];
    if (i == 0 || i == 4)
      EXPECT_EQ(bulk[i], bulk_before_set[i]); // H2O and Fe+++ are set by original constraints
    else
      EXPECT_NEAR(bulk[mgd.basis_species_index[gold_name[i]]] / b, 1.0, 1.0E-8);
  }
}

/// Check getConstraintMeaning
TEST_F(GeochemicalSystemTest, getConstraintMeaning)
{
  ModelGeochemicalDatabase mgd = _model_calcite.modelGeochemicalDatabase();
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION};
  const std::vector<GeochemicalSystem::ConstraintMeaningEnum> cim = {
      GeochemicalSystem::ConstraintMeaningEnum::ACTIVITY,
      GeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      GeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY,
      GeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  const GeochemicalSystem egs(mgd,
                              _ac3,
                              _is3,
                              _swapper4,
                              {"Ca++"},
                              {"Calcite"},
                              "H+",
                              {"H2O", "H+", "HCO3-", "Calcite"},
                              {1.75, 3.0, 2.0, 1.0},
                              cu,
                              cm,
                              25,
                              0,
                              1E-20,
                              {},
                              {},
                              {});
  for (unsigned i = 0; i < 4; ++i)
    EXPECT_EQ(egs.getConstraintMeaning()[i], cim[i]);
}

/// Check changeConstraintToBulk exceptions
TEST_F(GeochemicalSystemTest, changeContraintToBulkExceptions)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-", "O2(aq)"}, {}, {"O2(g)"}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FUGACITY);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);

  GeochemicalSystem egs(mgd,
                        _ac3,
                        _is3,
                        _swapper4,
                        {"O2(aq)"},
                        {"O2(g)"},
                        "H+",
                        {"H2O", "H+", "O2(g)", "HCO3-"},
                        {1.0, 2.0, 3.0, 4.0},
                        cu,
                        cm,
                        25,
                        0,
                        1E-20,
                        {},
                        {},
                        {});

  try
  {
    egs.changeConstraintToBulk(4);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Cannot changeConstraintToBulk for species 4 because there are only 4 "
                         "basis species") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    egs.changeConstraintToBulk(4, 1.0);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Cannot changeConstraintToBulk for species 4 because there are only 4 "
                         "basis species") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    egs.changeConstraintToBulk(3, 1.0);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Attempting to changeConstraintToBulk for a gas species.  Since a swap is "
                         "involved, you cannot specify a value for the bulk number of moles.  You "
                         "must use changeConstraintToBulk(basis_ind) method instead of "
                         "changeConstraintToBulk(basis_ind, value)") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check closeSystem
TEST_F(GeochemicalSystemTest, closeSystem)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-", "O2(aq)"}, {}, {"O2(g)"}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FUGACITY);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::KG);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);

  GeochemicalSystem egs(mgd,
                        _ac3,
                        _is3,
                        _swapper4,
                        {"O2(aq)"},
                        {"O2(g)"},
                        "H+",
                        {"H2O", "H+", "HCO3-", "O2(g)"},
                        {1.0, 2.0, 3.0, 4.0},
                        cu,
                        cm,
                        25,
                        0,
                        1E-20,
                        {},
                        {},
                        {});
  egs.closeSystem();
  const std::vector<GeochemicalSystem::ConstraintMeaningEnum> cim = {
      GeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_WATER,
      GeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      GeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      GeochemicalSystem::ConstraintMeaningEnum::FUGACITY};
  for (unsigned i = 0; i < 4; ++i)
    EXPECT_EQ(egs.getConstraintMeaning()[i], cim[i]);
}

/// Check changeConstraintToBulk
TEST_F(GeochemicalSystemTest, changeConstraintToBulk)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-", "O2(aq)"}, {}, {"O2(g)"}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FUGACITY);
  std::vector<GeochemicalSystem::ConstraintMeaningEnum> cim;
  cim.push_back(GeochemicalSystem::ConstraintMeaningEnum::KG_SOLVENT_WATER);
  cim.push_back(GeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES);
  cim.push_back(GeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY);
  cim.push_back(GeochemicalSystem::ConstraintMeaningEnum::FUGACITY);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::KG);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);

  GeochemicalSystem egs(mgd,
                        _ac3,
                        _is3,
                        _swapper4,
                        {"O2(aq)"},
                        {"O2(g)"},
                        "H+",
                        {"H2O", "H+", "HCO3-", "O2(g)"},
                        {1.0, 2.0, 3.0, 4.0},
                        cu,
                        cm,
                        25,
                        0,
                        1E-20,
                        {},
                        {},
                        {});
  const std::vector<Real> orig_bulk = egs.getBulkMolesOld();

  egs.changeConstraintToBulk(0);
  cim[0] = GeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_WATER;
  for (unsigned i = 0; i < 4; ++i)
    EXPECT_EQ(egs.getConstraintMeaning()[i], cim[i]);
  for (unsigned i = 0; i < 4; ++i)
    EXPECT_EQ(egs.getBulkMolesOld()[i], orig_bulk[i]);

  std::vector<Real> new_bulk = orig_bulk;
  egs.changeConstraintToBulk(1, 1.1); // just resets the bulk constraint value
  new_bulk[1] = 1.1;
  for (unsigned i = 0; i < 4; ++i)
    EXPECT_EQ(egs.getConstraintMeaning()[i], cim[i]);
  for (unsigned i = 0; i < 4; ++i)
    EXPECT_EQ(egs.getBulkMolesOld()[i], new_bulk[i]);

  egs.changeConstraintToBulk(2); // this means charge-balance can be enforced simply: see next line
  new_bulk[1] = new_bulk[2];
  cim[2] = GeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES;
  for (unsigned i = 0; i < 4; ++i)
    EXPECT_EQ(egs.getConstraintMeaning()[i], cim[i]);
  for (unsigned i = 0; i < 4; ++i)
    EXPECT_EQ(egs.getBulkMolesOld()[i], new_bulk[i]);

  egs.changeConstraintToBulk(3);
  cim[3] = GeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES;
  for (unsigned i = 0; i < 4; ++i)
    EXPECT_EQ(egs.getConstraintMeaning()[i], cim[i]);
  EXPECT_EQ(mgd.basis_species_name[3], "(O-phth)--");
}

/// Check addToBulkMoles exceptions
TEST_F(GeochemicalSystemTest, addToBulkMolesExceptions)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-", "O2(aq)"}, {}, {"O2(g)"}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FUGACITY);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::KG);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);

  GeochemicalSystem egs(mgd,
                        _ac3,
                        _is3,
                        _swapper4,
                        {"O2(aq)"},
                        {"O2(g)"},
                        "H+",
                        {"H2O", "H+", "HCO3-", "O2(g)"},
                        {1.0, 2.0, 3.0, 4.0},
                        cu,
                        cm,
                        25,
                        0,
                        1E-20,
                        {},
                        {},
                        {});

  try
  {
    egs.addToBulkMoles(4, 1.0);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("Cannot addToBulkMoles for species 4 because there are only 4 basis species") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check addToBulkMoles
TEST_F(GeochemicalSystemTest, addToBulkMoles)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "O2(aq)", "HCO3-"}, {}, {"O2(g)"}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);

  std::vector<Real> bm = {1.0, 2.0, 3.0, 4.0};

  GeochemicalSystem egs(mgd,
                        _ac3,
                        _is3,
                        _swapper4,
                        {},
                        {},
                        "H+",
                        {"H2O", "H+", "O2(aq)", "HCO3-"},
                        bm,
                        cu,
                        cm,
                        25,
                        0,
                        1E-20,
                        {},
                        {},
                        {});

  egs.addToBulkMoles(0, 1.1);
  bm[0] += 1.1;
  for (unsigned i = 0; i < 3; ++i)
    EXPECT_EQ(egs.getBulkMolesOld()[i], bm[i]);
  egs.addToBulkMoles(1, 2.2);
  bm[1] += 2.2;
  for (unsigned i = 0; i < 3; ++i)
    EXPECT_EQ(egs.getBulkMolesOld()[i], bm[i]);
  egs.addToBulkMoles(2, 3.3);
  bm[2] += 3.3;
  for (unsigned i = 0; i < 3; ++i)
    EXPECT_EQ(egs.getBulkMolesOld()[i], bm[i]);
}

/// Check setConstraintValue exceptions
TEST_F(GeochemicalSystemTest, setConstraintValueExceptions)
{
  GeochemicalSystem nonconst = _egs_calcite;
  try
  {
    nonconst.setConstraintValue(4, 1.0);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find(
            "Cannot setConstraintValue for species 4 because there are only 4 basis species") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check setConstraintValue
TEST_F(GeochemicalSystemTest, setConstraintValue)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "O2(aq)", "HCO3-"}, {}, {"O2(g)"}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm;
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY);
  cm.push_back(GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);

  std::vector<Real> bm_in = {1.0, 2.0, 3.0, 4.0};

  GeochemicalSystem egs(mgd,
                        _ac3,
                        _is3,
                        _swapper4,
                        {},
                        {},
                        "H+",
                        {"H2O", "H+", "O2(aq)", "HCO3-"},
                        bm_in,
                        cu,
                        cm,
                        25,
                        0,
                        1E-20,
                        {},
                        {},
                        {});

  std::vector<Real> bm = egs.getBulkMolesOld();
  std::vector<Real> mol = egs.getSolventMassAndFreeMolalityAndMineralMoles();

  egs.setConstraintValue(0, 1.1);
  bm[0] = 1.1;
  for (unsigned i = 0; i < 4; ++i)
    EXPECT_EQ(egs.getBulkMolesOld()[i], bm[i]);
  for (unsigned i = 0; i < 4; ++i)
    EXPECT_EQ(egs.getSolventMassAndFreeMolalityAndMineralMoles()[i], mol[i]);

  egs.setConstraintValue(1, 2.2);
  bm[1] = 2.2;
  for (unsigned i = 0; i < 4; ++i)
    EXPECT_EQ(egs.getBulkMolesOld()[i], bm[i]);
  for (unsigned i = 0; i < 4; ++i)
    EXPECT_EQ(egs.getSolventMassAndFreeMolalityAndMineralMoles()[i], mol[i]);

  egs.setConstraintValue(3, 4.4);
  mol[3] = 4.4;
  bm = egs.getBulkMolesOld(); // setting the molality to 4.4 changes the bulk composition
  for (unsigned i = 0; i < 4; ++i)
    EXPECT_EQ(egs.getBulkMolesOld()[i], bm[i]);
  for (unsigned i = 0; i < 4; ++i)
    EXPECT_EQ(egs.getSolventMassAndFreeMolalityAndMineralMoles()[i], mol[i]);

  egs.setConstraintValue(2, 3.3);
  bm = egs.getBulkMolesOld(); // setting the activity to 3.3 changes the basis molality and hence
                              // the bulk composition
  for (unsigned i = 0; i < 4; ++i)
    EXPECT_EQ(egs.getBulkMolesOld()[i], bm[i]);
  EXPECT_EQ(egs.getBasisActivity(2), 3.3);

  // now set the constraint on HCO3- to bulk, which allows charge-neutrality to be enforced simply
  egs.changeConstraintToBulk(3);
  // set HCO3- and see it changes H+
  bm = egs.getBulkMolesOld();
  egs.setConstraintValue(3, 7.7);
  bm[1] = 7.7;
  bm[3] = 7.7;
  for (unsigned i = 0; i < 4; ++i)
    EXPECT_NEAR(egs.getBulkMolesOld()[i], bm[i], 1.0E-8);
  // set H+ and see it actually makes no difference because of charge-neutrality
  egs.setConstraintValue(1, 1.2345);
  for (unsigned i = 0; i < 4; ++i)
    EXPECT_NEAR(egs.getBulkMolesOld()[i], bm[i], 1.0E-8);
}

/// illustrates how to use getModelGeochemicalDatabase and setModelGeochemicalDatabase
TEST_F(GeochemicalSystemTest, getsetModelGeochemicalDatabase)
{
  // this test involves radically changing the basis to illustrate copying, etc, works: no coder
  // should actually perform these radical changes otherwise a crash is virtually guaranteed!

  const GeochemicalDatabaseReader db("database/moose_testdb.json", true, true, false);
  const PertinentGeochemicalSystem pgs(db, {"H2O", "H+"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = pgs.modelGeochemicalDatabase();
  GeochemistrySpeciesSwapper swapper(2, 1E-6);
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  GeochemicalSystem egs(mgd,
                        _ac3,
                        _is3,
                        swapper,
                        {},
                        {},
                        "H+",
                        {"H2O", "H+"},
                        {1, 1},
                        cu,
                        cm,
                        25,
                        0,
                        1E-20,
                        {},
                        {},
                        {});

  const ModelGeochemicalDatabase & mgd_ref = egs.getModelGeochemicalDatabase();
  EXPECT_EQ(mgd_ref.basis_species_name.size(), (std::size_t)2);
  ModelGeochemicalDatabase copy_of_mgd = egs.getModelGeochemicalDatabase();
  EXPECT_EQ(copy_of_mgd.basis_species_name.size(), (std::size_t)2);

  // the following copies all the data of egs into copy_of_egs, but only the references (such as
  // _mgd) get copied: the data they refer to doesn't get copied
  GeochemicalSystem copy_of_egs = egs;

  // create a new ModelGeochemicalDatabase, with 3 basis species
  const PertinentGeochemicalSystem pgs3(
      db, {"H2O", "H+", "Ca++"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd3 = pgs3.modelGeochemicalDatabase();

  // Copy mgd3 into the memory referred to by copy_of_egs._mgd, which is also referred to by
  // egs._mgd and mgd_ref
  copy_of_egs.setModelGeochemicalDatabase(mgd3);
  EXPECT_EQ(mgd_ref.basis_species_name.size(), (std::size_t)3);
  EXPECT_EQ(egs.getModelGeochemicalDatabase().basis_species_name.size(), (std::size_t)3);
  EXPECT_EQ(copy_of_egs.getModelGeochemicalDatabase().basis_species_name.size(), (std::size_t)3);
  EXPECT_EQ(copy_of_mgd.basis_species_name.size(),
            (std::size_t)2); // copy_of_mgd shouldn't be impacted

  // do a similar thing with 1 basis species
  const PertinentGeochemicalSystem pgs1(db, {"H2O"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd1 = pgs1.modelGeochemicalDatabase();
  egs.setModelGeochemicalDatabase(mgd1);
  EXPECT_EQ(mgd_ref.basis_species_name.size(), (std::size_t)1);
  EXPECT_EQ(egs.getModelGeochemicalDatabase().basis_species_name.size(), (std::size_t)1);
  EXPECT_EQ(copy_of_egs.getModelGeochemicalDatabase().basis_species_name.size(), (std::size_t)1);
  EXPECT_EQ(copy_of_mgd.basis_species_name.size(), (std::size_t)2);
}

/// check setMineralRelatedFreeMoles
TEST_F(GeochemicalSystemTest, getsetMineralRelatedFreeMoles)
{
  // first, create an GeochemicalSystem that has minerals and sorption-related things
  const PertinentGeochemicalSystem model_gas_and_sorption(
      _db_calcite,
      {"H2O", "H+", "HCO3-", "O2(aq)", "Fe+++", ">(s)FeOH", ">(w)FeOH", "Ca++"},
      {"Goethite", "Calcite"},
      {"O2(g)"},
      {"Calcite_asdf"},
      {},
      {},
      "O2(aq)",
      "e-");
  ModelGeochemicalDatabase mgd = model_gas_and_sorption.modelGeochemicalDatabase();
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
      GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FUGACITY,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::KG);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> ku;
  ku.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  GeochemicalSystem nonconst(
      mgd,
      _ac0,
      _is0,
      _swapper8,
      {"O2(aq)", "Ca++"},
      {"O2(g)", "Calcite"},
      "Fe+++",
      {"H2O", "H+", "HCO3-", "O2(g)", "Fe+++", ">(s)FeOH", ">(w)FeOH", "Calcite"},
      {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0},
      cu,
      cm,
      25,
      0,
      1E-20,
      {"Calcite_asdf"},
      {9.0},
      ku);
  const unsigned num_basis = nonconst.getNumInBasis();
  const unsigned num_eqm = nonconst.getNumInEquilibrium();
  const unsigned num_kin = nonconst.getNumKinetic();

  // preset some molalities
  const std::vector<std::string> gold_name = {
      "H2O",         "H+",         "HCO3-",
      "O2(g)",       "Fe+++",      ">(s)FeOH",
      ">(w)FeOH",    "Calcite",    "(O-phth)--",
      "CH4(aq)",     "CO2(aq)",    "CO3--",
      "CaCO3",       "CaOH+",      "OH-",
      ">(s)FeO-",    ">(s)FeOCa+", "Goethite",
      "Ca++",        "O2(aq)",     "Goethite_surface_potential_expr",
      "Calcite_asdf"};
  const std::vector<Real> set_molal = {1.1, 2.2, 3.3, 0.0, 5.5, 6.6, 7.7, 8.8, 9.9, 10, 11,
                                       12,  13,  14,  15,  17,  18,  19,  20,  21,  22, 23};
  nonconst.setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
      gold_name, set_molal, {true, true, true, false, true, true, true, true});

  // now set the moles of mineral-related things to 1234.5
  nonconst.setMineralRelatedFreeMoles(1234.5);

  std::vector<Real> gold_molal = set_molal;
  gold_molal[5] = 1234.5;  // >(s)FeOH
  gold_molal[6] = 1234.5;  // >(w)FeOH
  gold_molal[7] = 1234.5;  // Calcite
  gold_molal[15] = 1234.5; // >(s)FeO-
  gold_molal[16] = 1234.5; // >(s)FeOCa+
  gold_molal[17] = 0.0;    // Goethite is an equilibrium mineral
  gold_molal[21] = 1234.5; // Calcite_asdf

  const std::vector<Real> & basis_mol = nonconst.getSolventMassAndFreeMolalityAndMineralMoles();
  for (unsigned i = 0; i < num_basis; ++i)
    EXPECT_EQ(basis_mol[mgd.basis_species_index.at(gold_name[i])], gold_molal[i]);
  const std::vector<Real> & eqm_mol = nonconst.getEquilibriumMolality();
  for (unsigned j = num_basis; j < num_basis + num_eqm; ++j)
    EXPECT_EQ(eqm_mol[mgd.eqm_species_index.at(gold_name[j])], gold_molal[j]);
  const std::vector<Real> & kin_mol = nonconst.getKineticMoles();
  for (unsigned k = num_basis + num_eqm + 1; k < num_basis + num_eqm + num_kin + 1;
       ++k) // + 1 for surface_pot
    EXPECT_EQ(kin_mol[mgd.kin_species_index.at(gold_name[k])], gold_molal[k]);
}

/// check initialKineticMole exceptions
TEST_F(GeochemicalSystemTest, initialKinMoleExcept)
{
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> ku;
  ku.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  try
  {
    const GeochemicalSystem bad(_mgd_kinetic_calcite,
                                _ac3,
                                _is3,
                                _swapper4,
                                {},
                                {},
                                "H+",
                                {"H2O", "Ca++", "H+", "HCO3-"},
                                {1.75, 3.0, 2.0, 1.0},
                                _cu_calcite,
                                _cm_calcite,
                                25,
                                0,
                                1E-20,
                                {},
                                {},
                                {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Initial mole number (or mass or volume) and a unit must be provided for "
                         "each kinetic species") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    const GeochemicalSystem bad(_mgd_kinetic_calcite,
                                _ac3,
                                _is3,
                                _swapper4,
                                {},
                                {},
                                "H+",
                                {"H2O", "Ca++", "H+", "HCO3-"},
                                {1.75, 3.0, 2.0, 1.0},
                                _cu_calcite,
                                _cm_calcite,
                                25,
                                0,
                                1E-20,
                                {"Calcite"},
                                {},
                                {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Initial mole number (or mass or volume) and a unit must be provided for "
                         "each kinetic species") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    const GeochemicalSystem bad(_mgd_kinetic_calcite,
                                _ac3,
                                _is3,
                                _swapper4,
                                {},
                                {},
                                "H+",
                                {"H2O", "Ca++", "H+", "HCO3-"},
                                {1.75, 3.0, 2.0, 1.0},
                                _cu_calcite,
                                _cm_calcite,
                                25,
                                0,
                                1E-20,
                                {"Calcite"},
                                {1.0},
                                {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Initial mole number (or mass or volume) and a unit must be provided for "
                         "each kinetic species") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    const GeochemicalSystem bad(_mgd_kinetic_calcite,
                                _ac3,
                                _is3,
                                _swapper4,
                                {},
                                {},
                                "H+",
                                {"H2O", "Ca++", "H+", "HCO3-"},
                                {1.75, 3.0, 2.0, 1.0},
                                _cu_calcite,
                                _cm_calcite,
                                25,
                                0,
                                1E-20,
                                {},
                                {1.0},
                                ku);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Initial mole number (or mass or volume) and a unit must be provided for "
                         "each kinetic species") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    const GeochemicalSystem bad(_mgd_kinetic_calcite,
                                _ac3,
                                _is3,
                                _swapper4,
                                {},
                                {},
                                "H+",
                                {"H2O", "Ca++", "H+", "HCO3-"},
                                {1.75, 3.0, 2.0, 1.0},
                                _cu_calcite,
                                _cm_calcite,
                                25,
                                0,
                                1E-20,
                                {"Ca++"},
                                {1.0},
                                {});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Initial mole number (or mass or volume) and a unit must be provided for "
                         "each kinetic species") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  std::vector<GeochemistryUnitConverter::GeochemistryUnit> ku_bad;
  ku_bad.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  try
  {
    const GeochemicalSystem bad(_mgd_kinetic_calcite,
                                _ac3,
                                _is3,
                                _swapper4,
                                {},
                                {},
                                "H+",
                                {"H2O", "Ca++", "H+", "HCO3-"},
                                {1.75, 3.0, 2.0, 1.0},
                                _cu_calcite,
                                _cm_calcite,
                                25,
                                0,
                                1E-20,
                                {"Calcite"},
                                {1.0},
                                ku_bad);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Kinetic species Calcite: units must be moles or mass, or volume in the "
                         "case of minerals") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// check getKineticLog10K exceptions
TEST_F(GeochemicalSystemTest, getKineticLog10Kexcept)
{
  try
  {
    _egs_kinetic_calcite.getKineticLog10K(1);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Cannot retrieve log10K for kinetic species 1 since there are only 1 "
                         "kinetic species") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// check log10KineticActivityProduct exceptions
TEST_F(GeochemicalSystemTest, log10KineticActivityProductExcept)
{
  try
  {
    _egs_kinetic_calcite.log10KineticActivityProduct(1);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("Cannot retrieve activity product for kinetic species 1 since there are only 1 "
                 "kinetic species") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// check getKineticMoles exceptions
TEST_F(GeochemicalSystemTest, getKineticMolesExcept)
{
  try
  {
    _egs_kinetic_calcite.getKineticMoles(1);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Cannot retrieve moles for kinetic species 1 since there are only 1 "
                         "kinetic species") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// check setKineticMoles exceptions
TEST_F(GeochemicalSystemTest, setKineticMolesExcept)
{
  GeochemicalSystem nonconst = _egs_kinetic_calcite;

  try
  {
    nonconst.setKineticMoles(1, 1.0);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Cannot set moles for kinetic species 1 since there are only 1 "
                         "kinetic species") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    nonconst.setKineticMoles(0, 0.0);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Mole number for kinetic species must be positive, not 0") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// check getNumKinetic
TEST_F(GeochemicalSystemTest, getNumKinetic)
{
  EXPECT_EQ(_egs_kinetic_calcite.getNumKinetic(), (unsigned)1);
}

/// check getKineticLog10K
TEST_F(GeochemicalSystemTest, getKineticLog10K)
{
  EXPECT_EQ(_egs_kinetic_calcite.getKineticLog10K(0), 1.7130);
  EXPECT_EQ(_egs_kinetic_calcite.getKineticLog10K().size(), (std::size_t)1);
  EXPECT_EQ(_egs_kinetic_calcite.getKineticLog10K()[0], 1.7130);
}

/// check log10KineticActivityProduct
TEST_F(GeochemicalSystemTest, log10KineticActivityProduct)
{
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
      GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
      GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> ku;
  ku.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  const GeochemicalSystem gs(_mgd_kinetic_calcite,
                             _ac3,
                             _is3,
                             _swapper4,
                             {},
                             {},
                             "Ca++",
                             {"H2O", "H+", "HCO3-", "Ca++"},
                             {1.75, 3.0, 2.0, 1.0},
                             cu,
                             cm,
                             25,
                             0,
                             1E-20,
                             {"Calcite"},
                             {1.1},
                             ku);
  // Calcite = Ca++ + HCO3- - H+
  EXPECT_NEAR(
      gs.log10KineticActivityProduct(0), std::log10(gs.getBasisActivity(3) / 3.0 * 2.0), 1.0E-6);
}

/// check getKineticMoles
TEST_F(GeochemicalSystemTest, getKineticMoles)
{
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> ku;
  ku.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  GeochemicalSystem gs(_mgd_kinetic_calcite,
                       _ac3,
                       _is3,
                       _swapper4,
                       {},
                       {},
                       "H+",
                       {"H2O", "H+", "HCO3-", "Ca++"},
                       {1.75, 3.0, 2.0, 1.0},
                       _cu_calcite,
                       _cm_calcite,
                       25,
                       0,
                       1E-20,
                       {"Calcite"},
                       {1.1},
                       ku);
  EXPECT_EQ(gs.getKineticMoles(0), 1.1);
  EXPECT_EQ(gs.getKineticMoles().size(), (std::size_t)1);
  EXPECT_EQ(gs.getKineticMoles()[0], 1.1);
  gs.setKineticMoles(0, 2.2);
  EXPECT_EQ(gs.getKineticMoles(0), 2.2);
  EXPECT_EQ(gs.getKineticMoles().size(), (std::size_t)1);
  EXPECT_EQ(gs.getKineticMoles()[0], 2.2);

  std::vector<GeochemistryUnitConverter::GeochemistryUnit> kucm3;
  kucm3.push_back(GeochemistryUnitConverter::GeochemistryUnit::CM3);
  GeochemicalSystem gscm3(_mgd_kinetic_calcite,
                          _ac3,
                          _is3,
                          _swapper4,
                          {},
                          {},
                          "H+",
                          {"H2O", "H+", "HCO3-", "Ca++"},
                          {1.75, 3.0, 2.0, 1.0},
                          _cu_calcite,
                          _cm_calcite,
                          25,
                          0,
                          1E-20,
                          {"Calcite"},
                          {1.1 * 36.9340},
                          kucm3);
  EXPECT_NEAR(gscm3.getKineticMoles(0), 1.1, 1.0E-6);
  EXPECT_EQ(gscm3.getKineticMoles().size(), (std::size_t)1);
  EXPECT_NEAR(gscm3.getKineticMoles()[0], 1.1, 1.0E-6);
  gscm3.setKineticMoles(0, 2.2);
  EXPECT_EQ(gscm3.getKineticMoles(0), 2.2);
  EXPECT_EQ(gscm3.getKineticMoles().size(), (std::size_t)1);
  EXPECT_EQ(gscm3.getKineticMoles()[0], 2.2);
}

/// check getBulkMolesOld with kinetic contributions
TEST_F(GeochemicalSystemTest, getBulk_with_kinetic)
{
  const PertinentGeochemicalSystem model_without_calcite(
      _db_calcite, {"H2O", "H+", "HCO3-", "Ca++"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd_without_calcite = model_without_calcite.modelGeochemicalDatabase();
  const GeochemicalSystem egs_without_kinetic(
      mgd_without_calcite,
      _ac3,
      _is3,
      _swapper4,
      {},
      {},
      "H+",
      {"H2O", "Ca++", "H+", "HCO3-"},
      // in _egs_kinetic_calcite, the 1.1 moles of Calcite contributes 1.1 moles of Ca++ and -1.1
      // moles of H+ (along with 1.1 moles of HCO3-, but this species has a free_concentration
      // constraint)
      {1.75, 3.0 + 1.1, 2.0 - 1.1, 1.0},
      _cu_calcite,
      _cm_calcite,
      25,
      0,
      1E-20,
      {},
      {},
      {});
  EXPECT_EQ(_egs_kinetic_calcite.getBulkMolesOld()[0],
            egs_without_kinetic.getBulkMolesOld()[0]); // H2O
  EXPECT_EQ(_egs_kinetic_calcite.getBulkMolesOld()[1],
            egs_without_kinetic.getBulkMolesOld()[1]); // H+ has fixed bulk
  EXPECT_EQ(_egs_kinetic_calcite.getBulkMolesOld()[2],
            egs_without_kinetic.getBulkMolesOld()[2] + 1.1); // HCO3- has fixed free molality
  EXPECT_EQ(_egs_kinetic_calcite.getBulkMolesOld()[3],
            egs_without_kinetic.getBulkMolesOld()[3]); // Ca++ has fixed bulk
}

/// check setAlgebraicVariables with kinetic
TEST_F(GeochemicalSystemTest, setAlgebraicVars_kinetic)
{
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> ku;
  ku.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  GeochemicalSystem gs(_mgd_kinetic_calcite,
                       _ac3,
                       _is3,
                       _swapper4,
                       {},
                       {},
                       "H+",
                       {"H2O", "H+", "HCO3-", "Ca++"},
                       {1.75, 3.0, 2.0, 1.0},
                       _cu_calcite,
                       _cm_calcite,
                       25,
                       0,
                       1E-20,
                       {"Calcite"},
                       {1.1},
                       ku);
  const DenseVector<Real> alg_vars(3, 3.3);
  gs.setAlgebraicVariables(alg_vars);
  for (unsigned i = 0; i < 3; ++i)
    EXPECT_EQ(gs.getAlgebraicVariableValues()[i], 3.3);
  EXPECT_EQ(gs.getKineticMoles()[0], 3.3);
}

/// check mineral free molality calculation when there are kinetic species
TEST_F(GeochemicalSystemTest, free_molality_kinetic)
{
  const PertinentGeochemicalSystem model(_db_calcite,
                                         {"H2O", "H+", "HCO3-", "Ca++"},
                                         {"Calcite"},
                                         {},
                                         {"Calcite_asdf"},
                                         {},
                                         {},
                                         "O2(aq)",
                                         "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> ku;
  ku.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  GeochemicalSystem gs(mgd,
                       _ac0,
                       _is0,
                       _swapper4,
                       {"Ca++"},
                       {"Calcite"},
                       "H+",
                       {"H2O", "H+", "HCO3-", "Calcite"},
                       {1.75, 3.0, 2.0, 10.0},
                       cu,
                       cm,
                       25,
                       0,
                       1E-20,
                       {"Calcite_asdf"},
                       {0.25},
                       ku);
  const Real orig_calcite = gs.getSolventMassAndFreeMolalityAndMineralMoles()[3];
  gs.setKineticMoles(0, 1.25);
  gs.computeConsistentConfiguration();
  EXPECT_NEAR(orig_calcite - 2.0,
              gs.getSolventMassAndFreeMolalityAndMineralMoles()[3],
              1.0E-8); // Calcite_asdf = 2 Calcite + ...
}

/// check setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles when there are kinetic species
TEST_F(GeochemicalSystemTest, setSolventETC4)
{
  const PertinentGeochemicalSystem model(_db_calcite,
                                         {"H2O", "H+", "HCO3-", "Ca++"},
                                         {"Calcite"},
                                         {},
                                         {"Calcite_asdf"},
                                         {},
                                         {},
                                         "O2(aq)",
                                         "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::KG);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> ku;
  ku.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  GeochemicalSystem gs(mgd,
                       _ac0,
                       _is0,
                       _swapper4,
                       {"Ca++"},
                       {"Calcite"},
                       "H+",
                       {"H2O", "H+", "HCO3-", "Calcite"},
                       {1.75, 3.0, 2.0, 10.0},
                       cu,
                       cm,
                       25,
                       0,
                       1E-20,
                       {"Calcite_asdf"},
                       {0.25},
                       ku);
  const std::vector<std::string> gold_name = {"H2O",
                                              "H+",
                                              "HCO3-",
                                              "Calcite",
                                              "CO2(aq)",
                                              "CO3--",
                                              "CaCO3",
                                              "CaOH+",
                                              "OH-",
                                              "Ca++",
                                              "Calcite_asdf"};
  const std::vector<Real> set_molal = {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.0, 11.1};
  gs.setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
      gold_name, set_molal, {true, true, true, true});

  const unsigned num_basis = gs.getNumInBasis();
  const unsigned num_eqm = gs.getNumInEquilibrium();
  const unsigned num_kin = gs.getNumKinetic();
  const std::vector<Real> molal = gs.getSolventMassAndFreeMolalityAndMineralMoles();
  for (unsigned i = 0; i < num_basis; ++i)
    EXPECT_EQ(molal[mgd.basis_species_index.at(gold_name[i])], set_molal[i]);
  for (unsigned j = num_basis; j < num_basis + num_eqm; ++j)
    EXPECT_EQ(gs.getEquilibriumMolality(mgd.eqm_species_index.at(gold_name[j])), set_molal[j]);
  for (unsigned k = num_basis + num_eqm; k < num_basis + num_eqm + num_kin; ++k)
    EXPECT_EQ(gs.getKineticMoles()[mgd.kin_species_index.at(gold_name[k])], set_molal[k]);
}

/// check computeJacobian exception
TEST_F(GeochemicalSystemTest, computeJacobianExcept)
{
  DenseMatrix<Real> jac;

  try
  {
    const DenseVector<Real> res(4);
    const DenseVector<Real> mole_additions(5);
    const DenseMatrix<Real> dmole_additions(5, 5);
    _egs_kinetic_calcite.computeJacobian(res, jac, mole_additions, dmole_additions);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Jacobian: residual size must be 3 but it is 4") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    const DenseVector<Real> res(3);
    const DenseVector<Real> mole_additions(4);
    const DenseMatrix<Real> dmole_additions(5, 5);
    _egs_kinetic_calcite.computeJacobian(res, jac, mole_additions, dmole_additions);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Jacobian: the increment in mole numbers (mole_additions) needs to be of "
                         "size 4 + 1 but it is of size 4") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    const DenseVector<Real> res(3);
    const DenseVector<Real> mole_additions(5);
    const DenseMatrix<Real> dmole_additions(4, 5);
    _egs_kinetic_calcite.computeJacobian(res, jac, mole_additions, dmole_additions);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Jacobian: the derivative of mole additions (dmole_additions) needs to be "
                         "of size 5x5 but it is of size 4x5") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    const DenseVector<Real> res(3);
    const DenseVector<Real> mole_additions(5);
    const DenseMatrix<Real> dmole_additions(5, 4);
    _egs_kinetic_calcite.computeJacobian(res, jac, mole_additions, dmole_additions);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Jacobian: the derivative of mole additions (dmole_additions) needs to be "
                         "of size 5x5 but it is of size 5x4") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// check updateOldWithCurrent exception
TEST_F(GeochemicalSystemTest, updateOldWithCurrent)
{
  try
  {
    const DenseVector<Real> mole_additions(4);
    GeochemicalSystem nonconst = _egs_kinetic_calcite;
    nonconst.updateOldWithCurrent(mole_additions);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("The increment in mole numbers (mole_additions) needs to be of size 4 + 1 "
                         "but it is of size 4") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// check addKineticRates exceptions
TEST_F(GeochemicalSystemTest, addKineticRatesExcept)
{
  GeochemicalSystem nonconst = _egs_kinetic_calcite;
  DenseVector<Real> mole_additions(5);
  DenseMatrix<Real> dmole_additions(5, 5);
  try
  {
    DenseVector<Real> bad(4);
    nonconst.addKineticRates(1.0, bad, dmole_additions);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("addKineticRates: incorrectly sized additions: 4 5 5") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
  try
  {
    DenseMatrix<Real> bad(4, 5);
    nonconst.addKineticRates(1.0, mole_additions, bad);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("addKineticRates: incorrectly sized additions: 5 4 5") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
  try
  {
    DenseMatrix<Real> bad(5, 4);
    nonconst.addKineticRates(1.0, mole_additions, bad);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("addKineticRates: incorrectly sized additions: 5 5 4") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// check addKineticRates: note that this checks rates and derivatives are produced, while GeochemicalKineticRateCalculatorTest tests that the rates and derivatives are numerically correct
TEST_F(GeochemicalSystemTest, addKineticRates)
{
  PertinentGeochemicalSystem mod(_db_calcite,
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
                                      1.5,
                                      0.5,
                                      0.1,
                                      {"H+", "HCO3-"},
                                      {1.3, 1.2},
                                      {0.3, 0.2},
                                      {0.1, 0.2},
                                      0.8,
                                      2.5,
                                      66.0,
                                      0.003,
                                      DirectionChoiceEnum::BOTH,
                                      "H2O",
                                      0.0,
                                      -1.0,
                                      0.0);
  KineticRateUserDescription rate_cal("Calcite",
                                      7.0,
                                      6.0,
                                      false,
                                      0.0,
                                      0.0,
                                      0.0,
                                      {"H+"},
                                      {-3.0},
                                      {0.0},
                                      {0.0},
                                      2.5,
                                      0.8,
                                      55.0,
                                      0.00315,
                                      DirectionChoiceEnum::BOTH,
                                      "H2O",
                                      0.0,
                                      -1.0,
                                      0.0);
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::KG);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> ku;
  ku.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  ku.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  DenseVector<Real> mole_additions(7);
  DenseMatrix<Real> dmole_additions(7, 7);
  ModelGeochemicalDatabase mgd_kin = mod.modelGeochemicalDatabase();
  GeochemicalSystem egs(mgd_kin,
                        _ac3,
                        _is3,
                        _swapper5,
                        {},
                        {},
                        "H+",
                        {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++"},
                        {1.75, 3.0, 2.0, 1.0, 1.5},
                        cu,
                        cm,
                        25,
                        0,
                        1E-20,
                        {"Calcite", "CH4(aq)"},
                        {1.1, 2.2},
                        ku);

  egs.addKineticRates(1.0, mole_additions, dmole_additions);
  for (unsigned i = 0; i < 7; ++i)
  {
    EXPECT_EQ(mole_additions(i), 0.0);
    for (unsigned j = 0; j < 7; ++j)
      EXPECT_EQ(dmole_additions(i, j), 0.0);
  }

  // add a rate and re-construct the geochemical system
  mod.addKineticRate(rate_ch4); // this adds to mole_additions(6)
  mgd_kin = mod.modelGeochemicalDatabase();
  egs.setModelGeochemicalDatabase(mgd_kin);

  egs.addKineticRates(1.0, mole_additions, dmole_additions);
  for (unsigned i = 0; i < 6; ++i)
  {
    EXPECT_EQ(mole_additions(i), 0.0);
    for (unsigned j = 0; j < 7; ++j)
      EXPECT_EQ(dmole_additions(i, j), 0.0);
  }
  const Real ch4rate = mole_additions(6);
  std::vector<Real> ch4deriv(7);
  for (unsigned j = 0; j < 7; ++j)
    ch4deriv[j] = dmole_additions(6, j);

  // add another (identical) rate and re-construct the geochemical system
  mod.addKineticRate(rate_ch4);
  mgd_kin = mod.modelGeochemicalDatabase();
  egs.setModelGeochemicalDatabase(mgd_kin);
  mole_additions.zero();
  dmole_additions.zero();
  egs.addKineticRates(1.0, mole_additions, dmole_additions);
  for (unsigned i = 0; i < 6; ++i)
  {
    EXPECT_EQ(mole_additions(i), 0.0);
    for (unsigned j = 0; j < 7; ++j)
      EXPECT_EQ(dmole_additions(i, j), 0.0);
  }
  EXPECT_EQ(mole_additions(6), 2 * ch4rate);
  for (unsigned j = 0; j < 7; ++j)
    EXPECT_EQ(dmole_additions(6, j), 2 * ch4deriv[j]);

  // check timestep size is OK
  mole_additions.zero();
  dmole_additions.zero();
  egs.addKineticRates(0.5, mole_additions, dmole_additions);
  for (unsigned i = 0; i < 6; ++i)
  {
    EXPECT_EQ(mole_additions(i), 0.0);
    for (unsigned j = 0; j < 7; ++j)
      EXPECT_EQ(dmole_additions(i, j), 0.0);
  }
  EXPECT_EQ(mole_additions(6), ch4rate);
  for (unsigned j = 0; j < 7; ++j)
    EXPECT_EQ(dmole_additions(6, j), ch4deriv[j]);

  // add a rate for calcite (mole_additions(5)) and re-construct the geochemical system
  mod.addKineticRate(rate_cal);
  mgd_kin = mod.modelGeochemicalDatabase();
  egs.setModelGeochemicalDatabase(mgd_kin);
  mole_additions.zero();
  dmole_additions.zero();
  egs.addKineticRates(0.5, mole_additions, dmole_additions);
  for (unsigned i = 0; i < 5; ++i)
  {
    EXPECT_EQ(mole_additions(i), 0.0);
    for (unsigned j = 0; j < 7; ++j)
      EXPECT_EQ(dmole_additions(i, j), 0.0);
  }
  EXPECT_NE(mole_additions(5), 0.0);
  const Real calciterate = mole_additions(5);
  EXPECT_EQ(mole_additions(6), ch4rate);
  for (unsigned j = 0; j < 7; ++j)
    EXPECT_EQ(dmole_additions(6, j), ch4deriv[j]);
  std::vector<Real> calcitederiv(7);
  Real sum_deriv = 0.0;
  for (unsigned j = 0; j < 7; ++j)
  {
    calcitederiv[j] = dmole_additions(5, j);
    sum_deriv += std::abs(dmole_additions(5, j));
  }
  EXPECT_NE(sum_deriv, 0.0);

  // add more rates for calcite and re-construct the geochemical system
  mod.addKineticRate(rate_cal);
  mod.addKineticRate(rate_cal);
  mod.addKineticRate(rate_cal);
  mgd_kin = mod.modelGeochemicalDatabase();
  egs.setModelGeochemicalDatabase(mgd_kin);
  mole_additions.zero();
  dmole_additions.zero();
  egs.addKineticRates(1.0, mole_additions, dmole_additions);
  for (unsigned i = 0; i < 5; ++i)
  {
    EXPECT_EQ(mole_additions(i), 0.0);
    for (unsigned j = 0; j < 7; ++j)
      EXPECT_EQ(dmole_additions(i, j), 0.0);
  }
  EXPECT_EQ(mole_additions(5), 8 * calciterate);
  EXPECT_EQ(mole_additions(6), 2 * ch4rate);
  for (unsigned j = 0; j < 7; ++j)
  {
    EXPECT_EQ(dmole_additions(5, j), 8 * calcitederiv[j]);
    EXPECT_EQ(dmole_additions(6, j), 2 * ch4deriv[j]);
  }
}

/// check addKineticRates with kinetic_bio_efficiency
TEST_F(GeochemicalSystemTest, addKineticRates_bio_eff)
{
  PertinentGeochemicalSystem mod(_db_calcite,
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
                                      1.5,
                                      0.5,
                                      0.1,
                                      {"H+", "HCO3-"},
                                      {1.3, 1.2},
                                      {0.3, 0.2},
                                      {0.1, 0.2},
                                      0.8,
                                      2.5,
                                      66.0,
                                      0.003,
                                      DirectionChoiceEnum::BOTH,
                                      "H2O",
                                      0.0,
                                      -1.0,
                                      1.0);
  KineticRateUserDescription rate_ch4_bio("CH4(aq)",
                                          1.5,
                                          2.0,
                                          true,
                                          1.5,
                                          0.5,
                                          0.1,
                                          {"H+", "HCO3-"},
                                          {1.3, 1.2},
                                          {0.3, 0.2},
                                          {0.1, 0.2},
                                          0.8,
                                          2.5,
                                          66.0,
                                          0.003,
                                          DirectionChoiceEnum::BOTH,
                                          "H2O",
                                          0.0,
                                          4.0,
                                          1.0);
  KineticRateUserDescription rate_ch4_death("CH4(aq)",
                                            1.5,
                                            2.0,
                                            true,
                                            1.5,
                                            0.5,
                                            0.1,
                                            {"H+", "HCO3-"},
                                            {1.3, 1.2},
                                            {0.3, 0.2},
                                            {0.1, 0.2},
                                            0.8,
                                            2.5,
                                            66.0,
                                            0.003,
                                            DirectionChoiceEnum::DEATH,
                                            "H2O",
                                            0.0,
                                            7.0,
                                            1.0);
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::KG);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> ku;
  ku.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  ku.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  DenseVector<Real> mole_additions(7);
  DenseMatrix<Real> dmole_additions(7, 7);
  ModelGeochemicalDatabase mgd_kin = mod.modelGeochemicalDatabase();
  GeochemicalSystem egs(mgd_kin,
                        _ac3,
                        _is3,
                        _swapper5,
                        {},
                        {},
                        "H+",
                        {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++"},
                        {1.75, 3.0, 2.0, 1.0, 1.5},
                        cu,
                        cm,
                        25,
                        0,
                        1E-20,
                        {"Calcite", "CH4(aq)"},
                        {1.1, 2.2},
                        ku);

  egs.addKineticRates(1.0, mole_additions, dmole_additions);
  for (unsigned i = 0; i < 7; ++i)
  {
    EXPECT_EQ(mole_additions(i), 0.0);
    for (unsigned j = 0; j < 7; ++j)
      EXPECT_EQ(dmole_additions(i, j), 0.0);
  }

  // add a rate and re-construct the geochemical system
  mod.addKineticRate(rate_ch4); // this adds to mole_additions(6) (the CH4(aq))
  mgd_kin = mod.modelGeochemicalDatabase();
  egs.setModelGeochemicalDatabase(mgd_kin);

  // capture the rate of CH4 with kinetic_bio_efficiency = -1.  This is assumed to be correct
  egs.addKineticRates(2.0, mole_additions, dmole_additions);
  for (unsigned i = 0; i < 6; ++i)
  {
    EXPECT_EQ(mole_additions(i), 0.0);
    for (unsigned j = 0; j < 7; ++j)
      EXPECT_EQ(dmole_additions(i, j), 0.0);
  }
  const Real rate_dt = -mole_additions(6);
  const Real drate_dkin_dt = -dmole_additions(6, 6);
  std::vector<Real> drate_dmol_dt(6);
  for (unsigned j = 0; j < 6; ++j)
    drate_dmol_dt[j] = -dmole_additions(6, j);

  // add a rate that is identical except it has kinetic_bio_efficiency = 4, and re-construct the
  // geochemical system
  mod.addKineticRate(rate_ch4_bio);
  mgd_kin = mod.modelGeochemicalDatabase();
  egs.setModelGeochemicalDatabase(mgd_kin);
  mole_additions.zero();
  dmole_additions.zero();
  egs.addKineticRates(2.0, mole_additions, dmole_additions);
  // CH4(aq) = 1*H2O + 1*H+ + 1*HCO3- + -2*O2(aq)
  const std::vector<Real> stoi = {1, 1, 1, -2, 0, 0};
  for (unsigned i = 0; i < 6; ++i)
  {
    EXPECT_NEAR(mole_additions(i), stoi[i] * (4 + 1) * rate_dt, 1E-8);
    EXPECT_NEAR(dmole_additions(i, 6), stoi[i] * (4 + 1) * drate_dkin_dt, 1E-8);
    for (unsigned j = 0; j < 6; ++j)
      EXPECT_NEAR(dmole_additions(i, j), stoi[i] * (4 + 1) * drate_dmol_dt[j], 1E-8);
  }
  EXPECT_NEAR(mole_additions(6), (4 - 1) * rate_dt, 1E-8);
  EXPECT_NEAR(dmole_additions(6, 6), (4 - 1) * drate_dkin_dt, 1E-8);
  for (unsigned j = 0; j < 6; ++j)
    EXPECT_NEAR(dmole_additions(6, j), (4 - 1) * drate_dmol_dt[j], 1E-8);

  // add a rate that is identical except it has kinetic_bio_efficiency = 7 and direction = DEATH,
  // and re-construct the geochemical system
  mod.addKineticRate(rate_ch4_death);
  mgd_kin = mod.modelGeochemicalDatabase();
  egs.setModelGeochemicalDatabase(mgd_kin);
  mole_additions.zero();
  dmole_additions.zero();
  egs.addKineticRates(2.0, mole_additions, dmole_additions);
  for (unsigned i = 0; i < 6; ++i)
  {
    EXPECT_NEAR(mole_additions(i), stoi[i] * (7 + 4 + 1) * rate_dt, 1E-8);
    EXPECT_NEAR(dmole_additions(i, 6), stoi[i] * (7 + 4 + 1) * drate_dkin_dt, 1E-8);
    for (unsigned j = 0; j < 6; ++j)
      EXPECT_NEAR(dmole_additions(i, j), stoi[i] * (7 + 4 + 1) * drate_dmol_dt[j], 1E-8);
  }
  EXPECT_NEAR(mole_additions(6), (7 + 4 - 1) * rate_dt, 1E-8);
  EXPECT_NEAR(dmole_additions(6, 6), (7 + 4 - 1) * drate_dkin_dt, 1E-8);
  for (unsigned j = 0; j < 6; ++j)
    EXPECT_NEAR(dmole_additions(6, j), (7 + 4 - 1) * drate_dmol_dt[j], 1E-8);
}

/// check addKineticRates with progeny
TEST_F(GeochemicalSystemTest, addKineticRates_progeny)
{
  PertinentGeochemicalSystem mod(_db_calcite,
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
                                      1.5,
                                      0.5,
                                      0.1,
                                      {"H+", "HCO3-"},
                                      {1.3, 1.2},
                                      {0.3, 0.2},
                                      {0.1, 0.2},
                                      0.8,
                                      2.5,
                                      66.0,
                                      0.003,
                                      DirectionChoiceEnum::BOTH,
                                      "H2O",
                                      0.0,
                                      -1.0,
                                      0.0);
  KineticRateUserDescription rate_ch4_progH("CH4(aq)",
                                            1.5,
                                            2.0,
                                            true,
                                            1.5,
                                            0.5,
                                            0.1,
                                            {"H+", "HCO3-"},
                                            {1.3, 1.2},
                                            {0.3, 0.2},
                                            {0.1, 0.2},
                                            0.8,
                                            2.5,
                                            66.0,
                                            0.003,
                                            DirectionChoiceEnum::BOTH,
                                            "H+",
                                            2.0,
                                            -1.0,
                                            1.0);
  KineticRateUserDescription rate_ch4_progOH("CH4(aq)",
                                             1.5,
                                             2.0,
                                             true,
                                             1.5,
                                             0.5,
                                             0.1,
                                             {"H+", "HCO3-"},
                                             {1.3, 1.2},
                                             {0.3, 0.2},
                                             {0.1, 0.2},
                                             0.8,
                                             2.5,
                                             66.0,
                                             0.003,
                                             DirectionChoiceEnum::BOTH,
                                             "OH-",
                                             22.0,
                                             -1.0,
                                             1.0);
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::KG);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> ku;
  ku.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  ku.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  DenseVector<Real> mole_additions(7);
  DenseMatrix<Real> dmole_additions(7, 7);
  ModelGeochemicalDatabase mgd_kin = mod.modelGeochemicalDatabase();
  GeochemicalSystem egs(mgd_kin,
                        _ac3,
                        _is3,
                        _swapper5,
                        {},
                        {},
                        "H+",
                        {"H2O", "H+", "HCO3-", "O2(aq)", "Ca++"},
                        {1.75, 3.0, 2.0, 1.0, 1.5},
                        cu,
                        cm,
                        25,
                        0,
                        1E-20,
                        {"Calcite", "CH4(aq)"},
                        {1.1, 2.2},
                        ku);

  egs.addKineticRates(1.0, mole_additions, dmole_additions);
  for (unsigned i = 0; i < 7; ++i)
  {
    EXPECT_EQ(mole_additions(i), 0.0);
    for (unsigned j = 0; j < 7; ++j)
      EXPECT_EQ(dmole_additions(i, j), 0.0);
  }

  // add a rate and re-construct the geochemical system
  mod.addKineticRate(rate_ch4); // this adds to mole_additions(6) (the CH4(aq))
  mgd_kin = mod.modelGeochemicalDatabase();
  egs.setModelGeochemicalDatabase(mgd_kin);

  // capture the rate of CH4 and the derivatives.  These are assumed to be correct
  egs.addKineticRates(2.0, mole_additions, dmole_additions);
  for (unsigned i = 0; i < 6; ++i)
  {
    EXPECT_EQ(mole_additions(i), 0.0);
    for (unsigned j = 0; j < 7; ++j)
      EXPECT_EQ(dmole_additions(i, j), 0.0);
  }
  const Real rate_dt = -mole_additions(6);
  const Real drate_dkin_dt = -dmole_additions(6, 6);
  std::vector<Real> drate_dmol_dt(6);
  for (unsigned j = 0; j < 6; ++j)
    drate_dmol_dt[j] = -dmole_additions(6, j);

  // add a rate and re-construct the geochemical system
  mod.addKineticRate(rate_ch4_progH); // this adds 2 moles of H as 1 mole of CH4(aq) dissolves
  mgd_kin = mod.modelGeochemicalDatabase();
  egs.setModelGeochemicalDatabase(mgd_kin);
  mole_additions.zero();
  dmole_additions.zero();
  egs.addKineticRates(2.0, mole_additions, dmole_additions);
  EXPECT_NEAR(mole_additions(1), 2 * rate_dt, 1E-8);
  for (unsigned j = 0; j < 6; ++j)
    EXPECT_NEAR(dmole_additions(1, j), 2 * drate_dmol_dt[j], 1E-8);
  EXPECT_NEAR(dmole_additions(1, 6), 2 * drate_dkin_dt, 1E-8);
  for (unsigned i = 0; i < 6; ++i)
  {
    if (i == 1)
      continue;
    EXPECT_EQ(mole_additions(i), 0);
    for (unsigned j = 0; j < 6; ++j)
      EXPECT_EQ(dmole_additions(i, j), 0);
  }
  EXPECT_NEAR(mole_additions(6), -2 * rate_dt, 1E-8);
  EXPECT_NEAR(dmole_additions(6, 6), -2 * drate_dkin_dt, 1E-8);
  for (unsigned j = 0; j < 6; ++j)
    EXPECT_NEAR(dmole_additions(6, j), -2 * drate_dmol_dt[j], 1E-8);

  // add a rate and re-construct the geochemical system
  mod.addKineticRate(rate_ch4_progOH); // this adds 22 moles of OH-, which is 22 moles of H2O - 22
                                       // moles of H+, as 1 mole of CH4(aq) dissolves
  mgd_kin = mod.modelGeochemicalDatabase();
  egs.setModelGeochemicalDatabase(mgd_kin);
  mole_additions.zero();
  dmole_additions.zero();
  egs.addKineticRates(2.0, mole_additions, dmole_additions);
  EXPECT_NEAR(mole_additions(0), 22 * rate_dt, 1E-8);
  for (unsigned j = 0; j < 6; ++j)
    EXPECT_NEAR(dmole_additions(0, j), 22 * drate_dmol_dt[j], 1E-8);
  EXPECT_NEAR(dmole_additions(0, 6), 22 * drate_dkin_dt, 1E-8);
  EXPECT_NEAR(mole_additions(1), (-22 + 2) * rate_dt, 1E-8);
  for (unsigned j = 0; j < 6; ++j)
    EXPECT_NEAR(dmole_additions(1, j), (-22 + 2) * drate_dmol_dt[j], 1E-8);
  EXPECT_NEAR(dmole_additions(1, 6), (-22 + 2) * drate_dkin_dt, 1E-8);
  for (unsigned i = 2; i < 6; ++i)
  {
    EXPECT_EQ(mole_additions(i), 0);
    for (unsigned j = 0; j < 6; ++j)
      EXPECT_EQ(dmole_additions(i, j), 0);
  }
  EXPECT_NEAR(mole_additions(6), -3 * rate_dt, 1E-8);
  EXPECT_NEAR(dmole_additions(6, 6), -3 * drate_dkin_dt, 1E-8);
  for (unsigned j = 0; j < 6; ++j)
    EXPECT_NEAR(dmole_additions(6, j), -3 * drate_dmol_dt[j], 1E-8);
}

/// Check getBulkOldInOriginalBasis
TEST_F(GeochemicalSystemTest, getBulkOldInOriginalBasis)
{
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION_WITH_KINETIC};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> ku;
  ku.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);

  GeochemicalSystem egs_no_swaps(_mgd_kinetic_calcite,
                                 _ac3,
                                 _is3,
                                 _swapper4,
                                 {},
                                 {},
                                 "H+",
                                 {"H2O", "H+", "HCO3-", "Ca++"},
                                 {1.75, 1.0, 5.0, 2.0},
                                 cu,
                                 cm,
                                 25,
                                 0,
                                 1E-20,
                                 {"Calcite"},
                                 {1.1},
                                 ku);
  EXPECT_EQ(egs_no_swaps.getBulkOldInOriginalBasis()(0), 1.75);        // H2O
  EXPECT_NEAR(egs_no_swaps.getBulkOldInOriginalBasis()(1), 2.1, 1E-6); // H+ from charge neutrality
  EXPECT_EQ(egs_no_swaps.getBulkOldInOriginalBasis()(2), 5.0 + 1.1);   // HCO3-
  EXPECT_EQ(egs_no_swaps.getBulkOldInOriginalBasis()(3), 2.0);         // Ca++

  ModelGeochemicalDatabase my_mgd_calcite = _mgd_calcite;
  GeochemicalSystem egs(my_mgd_calcite,
                        _ac3,
                        _is3,
                        _swapper4,
                        {},
                        {},
                        "H+",
                        {"H2O", "H+", "HCO3-", "Calcite"},
                        {-0.5, 2.5, 1.0, 3.5},
                        cu,
                        cm,
                        25,
                        0,
                        1E-20,
                        {},
                        {},
                        {});
  // In current basis, bulk(H2O) = -0.5, bulk(H+) = 1.0 (charge neutrality), bulk(HCO3-) = 1.0,
  // bulk(Calcite) = 3.5.  So, in orig basis (Calcite = Ca++ + HCO3- - H+):
  EXPECT_EQ(egs.getBulkOldInOriginalBasis()(0), -0.5); // H2O
  EXPECT_EQ(egs.getBulkOldInOriginalBasis()(1), -2.5); // H+
  EXPECT_EQ(egs.getBulkOldInOriginalBasis()(2), 4.5);  // HCO3-
  EXPECT_EQ(egs.getBulkOldInOriginalBasis()(3), 3.5);  // Ca++

  // perform some swaps
  egs.performSwap(2, 0); // CO2(aq) and HCO3-
  egs.performSwap(3, 3); // Calcite and CaOH+

  // bulk in original basis should not have changed (apart from some roundoff error)
  EXPECT_NEAR(egs.getBulkOldInOriginalBasis()(0), -0.5, 1.0E-6); // H2O
  EXPECT_NEAR(egs.getBulkOldInOriginalBasis()(1), -2.5, 1.0E-6); // H+
  EXPECT_NEAR(egs.getBulkOldInOriginalBasis()(2), 4.5, 1.0E-6);  // HCO3-
  EXPECT_NEAR(egs.getBulkOldInOriginalBasis()(3), 3.5, 1.0E-6);  // Ca++
}

/// Check TransportedBulk and also in original basis
TEST_F(GeochemicalSystemTest, getTransportedBulkMoles)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  const std::vector<std::string> original_basis = {
      "H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe+++", "HCO3-"};
  PertinentGeochemicalSystem model(database,
                                   original_basis,
                                   {"Fe(OH)3(ppd)"},
                                   {},
                                   {"Something", "Fe(OH)3(ppd)fake"}, // non transporting
                                   {},
                                   {},
                                   "O2(aq)",
                                   "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION,
      GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::KG);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> ku;
  ku.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  ku.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  GeochemicalSystem egs(mgd,
                        _ac3,
                        _is3,
                        _swapper6,
                        {},
                        {},
                        "H+",
                        {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe+++", "HCO3-"},
                        {1.75, 1.0, 2.0, 3.0, 0.5, 1.0},
                        cu,
                        cm,
                        40,
                        0,
                        1E-20,
                        {"Something", "Fe(OH)3(ppd)fake"},
                        {4.4, 5.5},
                        ku);

  std::vector<Real> basis_mol = egs.getSolventMassAndFreeMolalityAndMineralMoles();
  // equilibrium species are (in this order):
  // [0] CO2(aq) = -H2O + H+ + HCO3-
  // [1] CO3-- = HCO3- - H+
  // [2] OH- = H2O - H+
  // [3] >(s)FeO- = >(s)FeOH - H+.  This is not transporting
  // [4] Fe(OH)3(ppd) = 3H2O - 3H+ + Fe+++.  This is not transporting
  std::vector<Real> eqm_mol = egs.getEquilibriumMolality();

  std::vector<Real> original_trans_bulk;
  egs.computeTransportedBulkFromMolalities(original_trans_bulk);

  std::vector<Real> gold_trans_bulk(6);
  // H2O
  gold_trans_bulk[0] = 1.75 * (GeochemistryConstants::MOLES_PER_KG_WATER - eqm_mol[0] + eqm_mol[2]);
  // "H+"
  gold_trans_bulk[1] = 1.75 * (basis_mol[1] + eqm_mol[0] - eqm_mol[1] - eqm_mol[2]);
  // ">(s)FeOH".  Non transporting and all equilibrium species involving it aren't transporting
  gold_trans_bulk[2] = 0.0;
  // ">(w)FeOH".  Non transporting, and there are no equilibrium species involving it
  gold_trans_bulk[3] = 0.0;
  // "Fe+++".  Equilibrium species Fe(OH)3(ppd) is not transporting
  gold_trans_bulk[4] = 1.75 * basis_mol[4];
  // "HCO3-"
  gold_trans_bulk[5] = 1.75 * (basis_mol[5] + eqm_mol[0] + eqm_mol[1]);

  const Real eps = 1E-6;

  for (unsigned i = 0; i < 6; ++i)
  {
    if (std::abs(gold_trans_bulk[i]) <= eps)
      EXPECT_LE(std::abs(original_trans_bulk[i]), eps);
    else
      EXPECT_NEAR(original_trans_bulk[i] / gold_trans_bulk[i], 1.0, eps);
  }

  EXPECT_EQ(model.originalBasisNames(), original_basis);

  // swap out Fe+++ and replace with Fe(OH)3(ppd)
  egs.performSwap(4, 4);

  // compare the gold and computed transporting bulk
  basis_mol = egs.getSolventMassAndFreeMolalityAndMineralMoles();
  // equilibrium species are (in this order):
  // [0] CO2(aq) = -H2O + H+ + HCO3-
  // [1] CO3-- = HCO3- - H+
  // [2] OH- = H2O - H+
  // [3] >(s)FeO- = >(s)FeOH - H+.  This is not transporting
  // [4] Fe+++ = Fe(OH)3(ppd) - 3H2O + 3H+
  eqm_mol = egs.getEquilibriumMolality();
  gold_trans_bulk[0] =
      1.75 * (GeochemistryConstants::MOLES_PER_KG_WATER - eqm_mol[0] + eqm_mol[2] - 3 * eqm_mol[4]);
  gold_trans_bulk[1] =
      1.75 * (basis_mol[1] + eqm_mol[0] - eqm_mol[1] - eqm_mol[2] + 3 * eqm_mol[4]);
  gold_trans_bulk[2] = 0.0;
  gold_trans_bulk[3] = 0.0;
  // "Fe(OH)3(ppd)".  Only contribution is from Fe+++
  gold_trans_bulk[4] = 1.75 * eqm_mol[4];
  gold_trans_bulk[5] = 1.75 * (basis_mol[5] + eqm_mol[0] + eqm_mol[1]);

  std::vector<Real> new_trans_bulk;
  egs.computeTransportedBulkFromMolalities(new_trans_bulk);
  for (unsigned i = 0; i < 6; ++i)
  {
    if (std::abs(gold_trans_bulk[i]) <= eps)
      EXPECT_LE(std::abs(new_trans_bulk[i]), eps);
    else
      EXPECT_NEAR(new_trans_bulk[i] / gold_trans_bulk[i], 1.0, eps);
  }

  // now in the original basis:
  std::vector<Real> gold_trans_bulk_orig(6);
  gold_trans_bulk_orig[0] =
      1.75 * (GeochemistryConstants::MOLES_PER_KG_WATER - eqm_mol[0] + eqm_mol[2]);
  gold_trans_bulk_orig[1] = 1.75 * (basis_mol[1] + eqm_mol[0] - eqm_mol[1] - eqm_mol[2]);
  gold_trans_bulk_orig[2] = 0.0;
  gold_trans_bulk_orig[3] = 0.0;
  gold_trans_bulk_orig[4] = 1.75 * eqm_mol[4];
  gold_trans_bulk_orig[5] = 1.75 * (basis_mol[5] + eqm_mol[0] + eqm_mol[1]);

  DenseVector<Real> trans_bulk_original_basis = egs.getTransportedBulkInOriginalBasis();
  for (unsigned i = 0; i < 6; ++i)
  {
    if (std::abs(gold_trans_bulk_orig[i]) <= eps)
      EXPECT_LE(std::abs(trans_bulk_original_basis(i)), eps);
    else
      EXPECT_NEAR(trans_bulk_original_basis(i) / gold_trans_bulk_orig[i], 1.0, eps);
  }

  EXPECT_EQ(model.originalBasisNames(), original_basis);

  // now set mineral-related moles and there should be no change since these are non-transporting
  egs.setMineralRelatedFreeMoles(123.0);
  egs.computeTransportedBulkFromMolalities(new_trans_bulk);
  for (unsigned i = 0; i < 6; ++i)
  {
    if (std::abs(gold_trans_bulk[i]) <= eps)
      EXPECT_LE(std::abs(new_trans_bulk[i]), eps);
    else
      EXPECT_NEAR(new_trans_bulk[i] / gold_trans_bulk[i], 1.0, eps);
  }
  trans_bulk_original_basis = egs.getTransportedBulkInOriginalBasis();
  for (unsigned i = 0; i < 6; ++i)
  {
    if (std::abs(gold_trans_bulk_orig[i]) <= eps)
      EXPECT_LE(std::abs(trans_bulk_original_basis(i)), eps);
    else
      EXPECT_NEAR(trans_bulk_original_basis(i) / gold_trans_bulk_orig[i], 1.0, eps);
  }
}

/// Check TransportedBulk for case including kinetic redox
TEST_F(GeochemicalSystemTest, getTransportedBulkMoles_kin_redox)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(
      database,
      {"H2O", "H+", "Fe++", "O2(aq)"},
      {},
      {},
      {},
      {"Fe+++"}, // is transporting, but should not be reported in transportedMoles
      {},
      "O2(aq)",
      "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::KG);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLAL);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> ku;
  ku.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  GeochemicalSystem egs(mgd,
                        _ac3,
                        _is3,
                        _swapper4,
                        {},
                        {},
                        "H+",
                        {"H2O", "H+", "Fe++", "O2(aq)"},
                        {1.75, -2.0, 1.0, 1.0},
                        cu,
                        cm,
                        40,
                        0,
                        1E-20,
                        {"Fe+++"},
                        {4.4},
                        ku);

  std::vector<Real> basis_mol = egs.getSolventMassAndFreeMolalityAndMineralMoles();
  // equilibrium species are only OH- = H2O - H+
  std::vector<Real> eqm_mol = egs.getEquilibriumMolality();

  std::vector<Real> trans_bulk;
  egs.computeTransportedBulkFromMolalities(trans_bulk);
  std::vector<Real> gold_trans_bulk(4);
  // H2O
  gold_trans_bulk[0] = 1.75 * (GeochemistryConstants::MOLES_PER_KG_WATER + eqm_mol[0]);
  // "H+"
  gold_trans_bulk[1] = 1.75 * (basis_mol[1] - eqm_mol[0]);
  // "Fe++"
  gold_trans_bulk[2] = 1.75 * (basis_mol[2]);
  // "O2(aq)"
  gold_trans_bulk[3] = 1.75 * (basis_mol[3]);

  const Real eps = 1E-6;
  for (unsigned i = 0; i < 4; ++i)
    EXPECT_NEAR(trans_bulk[i] / gold_trans_bulk[i], 1.0, eps);
}

/// Check copy assignment constructor exceptions: the copy assignment is further checked in GeochmicalSolverTest for a nontrivial situation
TEST_F(GeochemicalSystemTest, copyAssignmentExcept)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);
  PertinentGeochemicalSystem model(
      database, {"H2O", "H+", "HCO3-"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION};
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu;
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::KG);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  cu.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  GeochemicalSystem dest(mgd,
                         _ac3,
                         _is3,
                         _swapper3,
                         {},
                         {},
                         "H+",
                         {"H2O", "H+", "HCO3-"},
                         {1.75, 1.0, 2.0},
                         cu,
                         cm,
                         40,
                         0,
                         1E-20,
                         {},
                         {},
                         {});

  try
  {
    dest = _egs_calcite; // wrong number of basis species
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("GeochemicalSystem: copy assigment operator called with inconsistent "
                         "fundamental properties") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  GeochemicalSystem dest2(mgd,
                          _ac3,
                          _is3,
                          _swapper3,
                          {},
                          {},
                          "HCO3-",
                          {"H2O", "H+", "HCO3-"},
                          {1.75, 1.0, 2.0},
                          cu,
                          cm,
                          40,
                          0,
                          1E-20,
                          {},
                          {},
                          {});
  try
  {
    dest2 = dest; // incorrect original charge-balance species
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("GeochemicalSystem: copy assigment operator called with inconsistent "
                         "fundamental properties") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  const PertinentGeochemicalSystem model_no_kinetic_calcite(
      _db_calcite, {"H2O", "H+", "HCO3-", "Ca++"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd_no_kinetic_calcite =
      model_no_kinetic_calcite.modelGeochemicalDatabase();
  GeochemicalSystem egs_no_kinetic_calcite(mgd_no_kinetic_calcite,
                                           _ac3,
                                           _is3,
                                           _swapper4,
                                           {},
                                           {},
                                           "H+",
                                           {"H2O", "Ca++", "H+", "HCO3-"},
                                           {1.75, 3.0, 2.0, 1.0},
                                           _cu_calcite,
                                           _cm_calcite,
                                           25,
                                           0,
                                           1E-20,
                                           {},
                                           {},
                                           {});
  try
  {
    egs_no_kinetic_calcite = _egs_kinetic_calcite; // wrong number of kinetic species
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("GeochemicalSystem: copy assigment operator called with inconsistent "
                         "fundamental properties") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}
