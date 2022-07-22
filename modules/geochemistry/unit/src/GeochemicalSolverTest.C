//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "GeochemicalSolver.h"
#include "GeochemistryKineticRateCalculator.h"

const GeochemicalDatabaseReader db_solver("database/moose_testdb.json", true, true, false);
const GeochemicalDatabaseReader db_full("../database/moose_geochemdb.json", true, true, false);
const GeochemicalDatabaseReader
    db_ferric("../test/database/ferric_hydroxide_sorption.json", true, true);
// Following model only has OH- as an equilibrium species
const PertinentGeochemicalSystem
    model_simplest(db_solver, {"H2O", "H+"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
GeochemistrySpeciesSwapper swapper2(2, 1E-6);
GeochemistrySpeciesSwapper swapper_kin(4, 1E-6);
const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm2 = {
    GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
    GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION};
const std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu2 = {
    GeochemistryUnitConverter::GeochemistryUnit::KG,
    GeochemistryUnitConverter::GeochemistryUnit::MOLES};
const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm4 = {
    GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
    GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
    GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
    GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION_WITH_KINETIC};
const std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu4 = {
    GeochemistryUnitConverter::GeochemistryUnit::KG,
    GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS,
    GeochemistryUnitConverter::GeochemistryUnit::MOLES,
    GeochemistryUnitConverter::GeochemistryUnit::MOLES};
GeochemistryIonicStrength is_solver(3.0, 3.0, false, false);
GeochemistryActivityCoefficientsDebyeHuckel ac_solver(is_solver, db_solver);

/// Check exception
TEST(GeochemicalSolverTest, exception)
{
  ModelGeochemicalDatabase mgd = model_simplest.modelGeochemicalDatabase();
  GeochemicalSystem egs(mgd,
                        ac_solver,
                        is_solver,
                        swapper2,
                        {},
                        {},
                        "H+",
                        {"H2O", "H+"},
                        {1.75, 3.0},
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
    const GeochemicalSolver solver(mgd.basis_species_name.size(),
                                   mgd.kin_species_name.size(),
                                   is_solver,
                                   1.0,
                                   0.1,
                                   1,
                                   1E100,
                                   0.1,
                                   1,
                                   {},
                                   3.0,
                                   10,
                                   true);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("GeochemicalSolver: ramp_max_ionic_strength must be less than max_iter") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    const GeochemicalSolver solver(mgd.basis_species_name.size(),
                                   mgd.kin_species_name.size(),
                                   is_solver,
                                   1.0,
                                   0.1,
                                   100,
                                   0.0,
                                   0.1,
                                   1,
                                   {},
                                   3.0,
                                   10,
                                   true);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("GeochemicalSolver: max_initial_residual must be positive") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    const GeochemicalSolver solver(mgd.basis_species_name.size(),
                                   mgd.kin_species_name.size(),
                                   is_solver,
                                   1.0,
                                   0.1,
                                   100,
                                   1E100,
                                   0.1,
                                   1,
                                   {},
                                   -1.0,
                                   10,
                                   true);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("GeochemicalSolver: max_ionic_strength must not be negative") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    const GeochemicalSolver solver(mgd.basis_species_name.size(),
                                   mgd.kin_species_name.size(),
                                   is_solver,
                                   1.0,
                                   -0.1,
                                   100,
                                   1E100,
                                   0.1,
                                   1,
                                   {},
                                   1.0,
                                   10,
                                   true);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("GeochemicalSolver: rel_tol must not be negative") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    const GeochemicalSolver solver(mgd.basis_species_name.size(),
                                   mgd.kin_species_name.size(),
                                   is_solver,
                                   -1.0,
                                   0.1,
                                   100,
                                   1E100,
                                   0.1,
                                   1,
                                   {},
                                   1.0,
                                   10,
                                   true);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("GeochemicalSolver: abs_tol must not be negative") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    const GeochemicalSolver solver(mgd.basis_species_name.size(),
                                   mgd.kin_species_name.size(),
                                   is_solver,
                                   0.0,
                                   0.0,
                                   100,
                                   1E100,
                                   0.1,
                                   1,
                                   {},
                                   1.0,
                                   10,
                                   true);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("GeochemicalSolver: either rel_tol or abs_tol must be positive") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    GeochemicalSolver solver(mgd.basis_species_name.size(),
                             mgd.kin_species_name.size(),
                             is_solver,
                             1.0,
                             0.1,
                             100,
                             1E100,
                             0.1,
                             1,
                             {},
                             1.0,
                             10,
                             true);
    solver.setMaxInitialResidual(0.0);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("GeochemicalSolver: max_initial_residual must be positive") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check set/getMaxInitialResidual
TEST(GeochemicalSolverTest, setgetMaxInitialResidual)
{
  ModelGeochemicalDatabase mgd = model_simplest.modelGeochemicalDatabase();
  GeochemicalSystem egs(mgd,
                        ac_solver,
                        is_solver,
                        swapper2,
                        {},
                        {},
                        "H+",
                        {"H2O", "H+"},
                        {1.75, 3.0},
                        cu2,
                        cm2,
                        25,
                        0,
                        1E-20,
                        {},
                        {},
                        {});
  GeochemicalSolver solver(mgd.basis_species_name.size(),
                           mgd.kin_species_name.size(),
                           is_solver,
                           1.0,
                           0.1,
                           100,
                           99.0,
                           0.1,
                           1,
                           {},
                           1.0,
                           10,
                           true);
  EXPECT_EQ(solver.getMaxInitialResidual(), 99.0);
  solver.setMaxInitialResidual(123.0);
  EXPECT_EQ(solver.getMaxInitialResidual(), 123.0);
}

/// Solve super-simple case
TEST(GeochemicalSolverTest, solve1)
{
  ModelGeochemicalDatabase mgd = model_simplest.modelGeochemicalDatabase();
  GeochemicalSystem egs(mgd,
                        ac_solver,
                        is_solver,
                        swapper2,
                        {},
                        {},
                        "H+",
                        {"H2O", "H+"},
                        {1.75, 1},
                        cu2,
                        cm2,
                        25,
                        0,
                        1E-20,
                        {},
                        {},
                        {});
  GeochemicalSolver solver(mgd.basis_species_name.size(),
                           mgd.kin_species_name.size(),
                           is_solver,
                           1.0E-15,
                           1.0E-200,
                           100,
                           10.0,
                           0.1,
                           1,
                           {},
                           3.0,
                           0,
                           false);

  std::stringstream ss;
  unsigned tot_iter;
  Real abs_residual;
  DenseVector<Real> mole_additions(egs.getNumInBasis() + egs.getNumKinetic());
  DenseMatrix<Real> dmole_additions(egs.getNumInBasis() + egs.getNumKinetic(),
                                    egs.getNumInBasis() + egs.getNumKinetic());
  solver.solveSystem(egs, ss, tot_iter, abs_residual, 0.0, mole_additions, dmole_additions);

  // check Newton has converged
  EXPECT_LE(tot_iter, (unsigned)100);
  EXPECT_LE(abs_residual, 1.0E-15);

  // check that equilibrium molality is set correctly
  EXPECT_NEAR(egs.getEquilibriumMolality(0) /
                  (egs.getBasisActivity(0) / egs.getBasisActivity(1) /
                   std::pow(10.0, egs.getLog10K(0)) / egs.getEquilibriumActivityCoefficient(0)),
              1.0,
              1.0E-9);

  // check that basis molality is correct
  EXPECT_EQ(egs.getSolventWaterMass(), 1.75);
  EXPECT_NEAR(egs.getSolventMassAndFreeMolalityAndMineralMoles()[1] / egs.getEquilibriumMolality(0),
              1.0,
              1.0E-9);

  // check that basis bulk composition is correct
  EXPECT_NEAR(egs.getBulkMolesOld()[0],
              1.75 * (GeochemistryConstants::MOLES_PER_KG_WATER + egs.getEquilibriumMolality(0)),
              1.0E-9);
  EXPECT_NEAR(egs.getBulkMolesOld()[1], 0.0, 1.0E-9);
}

/// Solve realistic case with minerals and gases, but no precipitation, no redox, no sorption
TEST(GeochemicalSolverTest, solve2)
{
  // build the model
  const PertinentGeochemicalSystem model(
      db_full,
      {"H2O", "H+", "Cl-", "Na+", "SO4--", "Mg++", "Ca++", "K+", "HCO3-", "SiO2(aq)", "O2(aq)"},
      {"Antigorite",
       "Tremolite",
       "Talc",
       "Chrysotile",
       "Sepiolite",
       "Anthophyllite",
       "Dolomite",
       "Dolomite-ord",
       "Huntite",
       "Dolomite-dis",
       "Magnesite",
       "Calcite",
       "Aragonite",
       "Quartz"},
      {"O2(g)", "CO2(g)"},
      {},
      {},
      {},
      "O2(aq)",
      "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  // build the equilibrium system
  GeochemistrySpeciesSwapper swapper(11, 1E-6);
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
      GeochemicalSystem::ConstraintUserMeaningEnum::FUGACITY,
      GeochemicalSystem::ConstraintUserMeaningEnum::FUGACITY,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION};
  const std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu = {
      GeochemistryUnitConverter::GeochemistryUnit::KG,
      GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS,
      GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES};
  GeochemicalSystem egs(
      mgd,
      ac_solver,
      is_solver,
      swapper,
      {"H+", "O2(aq)"},
      {"CO2(g)", "O2(g)"},
      "Cl-",
      {"H2O", "CO2(g)", "O2(g)", "Cl-", "Na+", "SO4--", "Mg++", "Ca++", "K+", "HCO3-", "SiO2(aq)"},
      {1.0,
       0.0003162278,
       0.2,
       0.5656,
       0.4850,
       0.02924,
       0.05501,
       0.01063,
       0.010576055,
       0.002412,
       0.00010349},
      cu,
      cm,
      25,
      0,
      1E-20,
      {},
      {},
      {});

  // build solver
  GeochemicalSolver solver(mgd.basis_species_name.size(),
                           mgd.kin_species_name.size(),
                           is_solver,
                           1.0E-15,
                           1.0E-200,
                           100,
                           1E3,
                           0.1,
                           1,
                           {"Antigorite",
                            "Tremolite",
                            "Talc",
                            "Chrysotile",
                            "Sepiolite",
                            "Anthophyllite",
                            "Dolomite",
                            "Dolomite-ord",
                            "Huntite",
                            "Dolomite-dis",
                            "Magnesite",
                            "Calcite",
                            "Aragonite",
                            "Quartz"},
                           3.0,
                           10,
                           false);

  // solve
  std::stringstream ss;
  unsigned tot_iter;
  Real abs_residual;
  DenseVector<Real> mole_additions(egs.getNumInBasis() + egs.getNumKinetic());
  DenseMatrix<Real> dmole_additions(egs.getNumInBasis() + egs.getNumKinetic(),
                                    egs.getNumInBasis() + egs.getNumKinetic());
  solver.solveSystem(egs, ss, tot_iter, abs_residual, 0.0, mole_additions, dmole_additions);

  // check converged
  EXPECT_LE(tot_iter, (unsigned)100);
  EXPECT_LE(abs_residual, 1.0E-15);

  // check number in basis, number in redox disequilibrium and number of surface potentials
  EXPECT_EQ(egs.getNumInBasis(), (unsigned)11);
  EXPECT_EQ(egs.getNumRedox(), (unsigned)1);
  EXPECT_EQ(egs.getNumSurfacePotentials(), (unsigned)0);
  EXPECT_EQ(egs.getNumInEquilibrium(), mgd.eqm_species_name.size());
  EXPECT_EQ(egs.getNumKinetic(), (unsigned)0);

  // check that the constraints are satisfied
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
    if (mgd.basis_species_name[i] == "H2O")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getSolventWaterMass(), 1.0, 1.0E-15);
      EXPECT_NEAR(egs.getSolventMassAndFreeMolalityAndMineralMoles()[0], 1.0, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "CO2(g)")
    {
      EXPECT_TRUE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_TRUE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBasisActivity(i), 0.0003162278, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "O2(g)")
    {
      EXPECT_TRUE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_TRUE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBasisActivity(i), 0.2, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Cl-")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      // do not know the bulk composition as it is dictated by charge neutrality
      EXPECT_EQ(egs.getChargeBalanceBasisIndex(), i);
    }
    else if (mgd.basis_species_name[i] == "Na+")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 0.4850, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "SO4--")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 0.02924, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Mg++")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 0.05501, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Ca++")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 0.01063, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "K+")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 0.010576055, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "HCO3-")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 0.002412, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "SiO2(aq)")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 0.00010349, 1.0E-15);
    }
    else
      FAIL() << "Incorrect basis species";

  // check total charge
  EXPECT_NEAR(egs.getTotalChargeOld(), 0.0, 1E-15);
  // check total charge by summing up basis charges
  Real tot_charge = 0.0;
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
    tot_charge += egs.getBulkMolesOld()[i] * mgd.basis_species_charge[i];
  EXPECT_NEAR(tot_charge, 0.0, 1E-15);

  // surface charges
  for (unsigned sp = 0; sp < egs.getNumSurfacePotentials(); ++sp)
  {
    Real charge = 0.0;
    for (unsigned j = 0; j < mgd.eqm_species_name.size(); ++j)
      if (mgd.surface_sorption_related[j] && mgd.surface_sorption_number[j] == sp)
        charge += mgd.eqm_species_charge[j] * egs.getEquilibriumMolality(j);
    EXPECT_NEAR(charge,
                egs.getSorbingSurfaceArea()[sp] * egs.getSurfaceCharge(sp) /
                    GeochemistryConstants::FARADAY,
                1E-15);
  }

  // check basis activity = activity_coefficient * molality
  for (unsigned i = 1; i < egs.getNumInBasis(); ++i) // don't loop over water
    if (mgd.basis_species_gas[i] || mgd.basis_species_mineral[i] || egs.getBasisActivityKnown()[i])
      continue;
    else
      EXPECT_NEAR(egs.getBasisActivity(i),
                  egs.getBasisActivityCoefficient(i) *
                      egs.getSolventMassAndFreeMolalityAndMineralMoles()[i],
                  1E-15);

  // check residuals are zero
  for (unsigned a = 0; a < egs.getNumInAlgebraicSystem(); ++a)
    EXPECT_LE(std::abs(egs.getResidualComponent(a, mole_additions)), 1E-15);
  // check residuals are zero by summing molalities, bulk compositions, etc
  Real nw = egs.getSolventWaterMass();
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
  {
    Real res = -egs.getBulkMolesOld()[i];
    if (i == 0)
      res += nw * GeochemistryConstants::MOLES_PER_KG_WATER;
    else if (mgd.basis_species_mineral[i])
      res += egs.getSolventMassAndFreeMolalityAndMineralMoles()[i];
    else if (mgd.basis_species_gas[i])
      res += 0.0;
    else
      res += nw * egs.getSolventMassAndFreeMolalityAndMineralMoles()[i];
    for (unsigned j = 0; j < mgd.eqm_species_name.size(); ++j)
      res += nw * mgd.eqm_stoichiometry(j, i) * egs.getEquilibriumMolality(j);
    EXPECT_LE(std::abs(res), 1E-13);
  }
  // surface potentials
  const Real prefactor = std::sqrt(GeochemistryConstants::GAS_CONSTANT *
                                   (25.0 + GeochemistryConstants::CELSIUS_TO_KELVIN) *
                                   GeochemistryConstants::PERMITTIVITY_FREE_SPACE *
                                   GeochemistryConstants::DIELECTRIC_CONSTANT_WATER *
                                   GeochemistryConstants::DENSITY_WATER * egs.getIonicStrength()) /
                         nw;
  for (unsigned sp = 0; sp < egs.getNumSurfacePotentials(); ++sp)
  {
    EXPECT_NEAR(prefactor * std::sinh(egs.getSurfacePotential(sp) * GeochemistryConstants::FARADAY /
                                      2.0 / GeochemistryConstants::GAS_CONSTANT /
                                      (25.0 + GeochemistryConstants::CELSIUS_TO_KELVIN)),
                egs.getSurfaceCharge(sp),
                1.0E-13);
  }

  // check equilibrium mass balance
  for (unsigned j = 0; j < mgd.eqm_species_name.size(); ++j)
  {
    if (mgd.eqm_species_mineral[j] || mgd.eqm_species_gas[j])
      continue;
    Real log10ap = 0.0;
    for (unsigned i = 0; i < mgd.basis_species_name.size(); ++i)
      log10ap += mgd.eqm_stoichiometry(j, i) * std::log10(egs.getBasisActivity(i));
    log10ap -= egs.getLog10K(j);
    if (mgd.surface_sorption_related[j])
      log10ap -= std::log10(egs.getSurfacePotential(mgd.surface_sorption_number[j]));
    else
      log10ap -= std::log10(egs.getEquilibriumActivityCoefficient(j));
    if (log10ap < -300.0)
      EXPECT_LE(std::abs(egs.getEquilibriumMolality(j)), 1E-25);
    else
      EXPECT_LE(std::abs(egs.getEquilibriumMolality(j) - std::pow(10.0, log10ap)), 1E-15);
  }
}

/// Solve realistic case with minerals that precipitate, no redox, no sorption
TEST(GeochemicalSolverTest, solve3)
{
  // build the model
  const PertinentGeochemicalSystem model(
      db_full,
      {"H2O", "H+", "Cl-", "Na+", "SO4--", "Mg++", "Ca++", "K+", "HCO3-", "SiO2(aq)", "O2(aq)"},
      {"Antigorite",
       "Tremolite",
       "Talc",
       "Chrysotile",
       "Sepiolite",
       "Anthophyllite",
       "Dolomite",
       "Dolomite-ord",
       "Huntite",
       "Dolomite-dis",
       "Magnesite",
       "Calcite",
       "Aragonite",
       "Quartz"},
      {},
      {},
      {},
      {},
      "O2(aq)",
      "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  // build the equilibrium system
  GeochemistrySpeciesSwapper swapper(11, 1E-6);
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION};
  const std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu = {
      GeochemistryUnitConverter::GeochemistryUnit::KG,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLAL,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES};
  GeochemicalSystem egs(
      mgd,
      ac_solver,
      is_solver,
      swapper,
      {"H+"},
      {"MgCO3"},
      "Cl-",
      {"H2O", "MgCO3", "O2(aq)", "Cl-", "Na+", "SO4--", "Mg++", "Ca++", "K+", "HCO3-", "SiO2(aq)"},
      {1.0,
       0.196E-3,
       0.2151E-3,
       0.5656,
       0.4850,
       0.02924,
       0.05501,
       0.01063,
       0.010576055,
       0.002412,
       0.00010349},
      cu,
      cm,
      25,
      0,
      1E-20,
      {},
      {},
      {});

  // build solver (4 swaps are needed to solve this system)
  GeochemicalSolver solver(mgd.basis_species_name.size(),
                           mgd.kin_species_name.size(),
                           is_solver,
                           1.0E-15,
                           1.0E-200,
                           100,
                           1E3,
                           0.1,
                           4,
                           {"Dolomite-ord", "Dolomite-dis"},
                           3.0,
                           10,
                           false);

  // solve
  std::stringstream ss;
  unsigned tot_iter;
  Real abs_residual;
  DenseVector<Real> mole_additions(egs.getNumInBasis() + egs.getNumKinetic());
  DenseMatrix<Real> dmole_additions(egs.getNumInBasis() + egs.getNumKinetic(),
                                    egs.getNumInBasis() + egs.getNumKinetic());
  solver.solveSystem(egs, ss, tot_iter, abs_residual, 0.0, mole_additions, dmole_additions);

  // check converged
  EXPECT_LE(tot_iter, (unsigned)100);
  EXPECT_LE(abs_residual, 1.0E-15);

  // check number in basis, number in redox disequilibrium and number of surface potentials
  EXPECT_EQ(egs.getNumInBasis(), (unsigned)11);
  EXPECT_EQ(egs.getNumRedox(), (unsigned)1);
  EXPECT_EQ(egs.getNumSurfacePotentials(), (unsigned)0);
  EXPECT_EQ(egs.getNumInEquilibrium(), mgd.eqm_species_name.size());

  // check that the constraints are satisfied
  Real bulk_dolo = 0.0;
  Real bulk_ca = 0.0;
  Real bulk_mg = 0.0;
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
    if (mgd.basis_species_name[i] == "H2O")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getSolventWaterMass(), 1.0, 1.0E-15);
      EXPECT_NEAR(egs.getSolventMassAndFreeMolalityAndMineralMoles()[0], 1.0, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "O2(aq)")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getSolventMassAndFreeMolalityAndMineralMoles()[i], 0.2151E-3, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Cl-")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      // do not know the bulk composition as it is dictated by charge neutrality
      EXPECT_EQ(egs.getChargeBalanceBasisIndex(), i);
    }
    else if (mgd.basis_species_name[i] == "Na+")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 0.4850, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "SO4--")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 0.02924, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Mg++")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      bulk_mg = egs.getBulkMolesOld()[i];
    }
    else if (mgd.basis_species_name[i] == "Ca++")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      bulk_ca = egs.getBulkMolesOld()[i];
    }
    else if (mgd.basis_species_name[i] == "K+")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 0.010576055, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Quartz")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_TRUE(mgd.basis_species_mineral[i]);
      EXPECT_TRUE(egs.getBasisActivityKnown()[i]);
      EXPECT_GE(egs.getSolventMassAndFreeMolalityAndMineralMoles()[i], 0.0);
      // Quartz = SiO2(aq)
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 0.00010349, 1.0);
    }
    else if (mgd.basis_species_name[i] == "Dolomite")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_TRUE(mgd.basis_species_mineral[i]);
      EXPECT_TRUE(egs.getBasisActivityKnown()[i]);
      EXPECT_GE(egs.getSolventMassAndFreeMolalityAndMineralMoles()[i], 0.0);
      bulk_dolo = egs.getBulkMolesOld()[i];
    }
    else if (mgd.basis_species_name[i] == "HCO3-")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 0.002412, 1.0E-13);
    }
    else
      FAIL() << "Incorrect basis species";
  // dolomite = 2MgCO3 - Mg + Ca
  EXPECT_NEAR(bulk_dolo + bulk_ca, 0.01063, 1E-13);
  EXPECT_NEAR(-bulk_dolo + bulk_mg, 0.05501, 1E-13);
  EXPECT_NEAR(2.0 * bulk_dolo, 0.196E-3, 1E-13);

  // check total charge
  EXPECT_NEAR(egs.getTotalChargeOld(), 0.0, 1E-15);
  // check total charge by summing up basis charges
  Real tot_charge = 0.0;
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
    tot_charge += egs.getBulkMolesOld()[i] * mgd.basis_species_charge[i];
  EXPECT_NEAR(tot_charge, 0.0, 1E-15);

  // surface charges
  for (unsigned sp = 0; sp < egs.getNumSurfacePotentials(); ++sp)
  {
    Real charge = 0.0;
    for (unsigned j = 0; j < mgd.eqm_species_name.size(); ++j)
      if (mgd.surface_sorption_related[j] && mgd.surface_sorption_number[j] == sp)
        charge += mgd.eqm_species_charge[j] * egs.getEquilibriumMolality(j);
    EXPECT_NEAR(charge,
                egs.getSorbingSurfaceArea()[sp] * egs.getSurfaceCharge(sp) /
                    GeochemistryConstants::FARADAY,
                1E-15);
  }

  // check basis activity = activity_coefficient * molality
  for (unsigned i = 1; i < egs.getNumInBasis(); ++i) // don't loop over water
    if (mgd.basis_species_gas[i] || mgd.basis_species_mineral[i] || egs.getBasisActivityKnown()[i])
      continue;
    else
      EXPECT_NEAR(egs.getBasisActivity(i),
                  egs.getBasisActivityCoefficient(i) *
                      egs.getSolventMassAndFreeMolalityAndMineralMoles()[i],
                  1E-15);

  // check residuals are zero
  for (unsigned a = 0; a < egs.getNumInAlgebraicSystem(); ++a)
    EXPECT_LE(std::abs(egs.getResidualComponent(a, mole_additions)), 1E-15);
  // check residuals are zero by summing molalities, bulk compositions, etc
  Real nw = egs.getSolventWaterMass();
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
  {
    Real res = -egs.getBulkMolesOld()[i];
    if (i == 0)
      res += nw * GeochemistryConstants::MOLES_PER_KG_WATER;
    else if (mgd.basis_species_mineral[i])
      res += egs.getSolventMassAndFreeMolalityAndMineralMoles()[i];
    else if (mgd.basis_species_gas[i])
      res += 0.0;
    else
      res += nw * egs.getSolventMassAndFreeMolalityAndMineralMoles()[i];
    for (unsigned j = 0; j < mgd.eqm_species_name.size(); ++j)
      res += nw * mgd.eqm_stoichiometry(j, i) * egs.getEquilibriumMolality(j);
    EXPECT_LE(std::abs(res), 1E-13);
  }
  // surface potentials
  const Real prefactor = std::sqrt(GeochemistryConstants::GAS_CONSTANT *
                                   (25.0 + GeochemistryConstants::CELSIUS_TO_KELVIN) *
                                   GeochemistryConstants::PERMITTIVITY_FREE_SPACE *
                                   GeochemistryConstants::DIELECTRIC_CONSTANT_WATER *
                                   GeochemistryConstants::DENSITY_WATER * egs.getIonicStrength()) /
                         nw;
  for (unsigned sp = 0; sp < egs.getNumSurfacePotentials(); ++sp)
  {
    EXPECT_NEAR(prefactor * std::sinh(egs.getSurfacePotential(sp) * GeochemistryConstants::FARADAY /
                                      2.0 / GeochemistryConstants::GAS_CONSTANT /
                                      (25.0 + GeochemistryConstants::CELSIUS_TO_KELVIN)),
                egs.getSurfaceCharge(sp),
                1.0E-13);
  }

  // check equilibrium mass balance
  for (unsigned j = 0; j < mgd.eqm_species_name.size(); ++j)
  {
    if (mgd.eqm_species_mineral[j] || mgd.eqm_species_gas[j])
      continue;
    Real log10ap = 0.0;
    for (unsigned i = 0; i < mgd.basis_species_name.size(); ++i)
      log10ap += mgd.eqm_stoichiometry(j, i) * std::log10(egs.getBasisActivity(i));
    log10ap -= egs.getLog10K(j);
    if (mgd.surface_sorption_related[j])
      log10ap -= std::log10(egs.getSurfacePotential(mgd.surface_sorption_number[j]));
    else
      log10ap -= std::log10(egs.getEquilibriumActivityCoefficient(j));
    if (log10ap < -300.0)
      EXPECT_LE(std::abs(egs.getEquilibriumMolality(j)), 1E-25);
    else
      EXPECT_LE(std::abs(egs.getEquilibriumMolality(j) - std::pow(10.0, log10ap)), 1E-15);
  }

  // check equilibrium species have negative saturation indices, except for prevent_precipitation
  // minerals
  for (unsigned j = 0; j < mgd.eqm_species_name.size(); ++j)
  {
    if (!mgd.eqm_species_mineral[j])
      continue;
    if (mgd.eqm_species_name[j] == "Dolomite-ord" || mgd.eqm_species_name[j] == "Dolomite-dis")
      continue;
    Real log10ap = 0.0;
    for (unsigned i = 0; i < mgd.basis_species_name.size(); ++i)
      log10ap += mgd.eqm_stoichiometry(j, i) * std::log10(egs.getBasisActivity(i));
    EXPECT_LE(log10ap, egs.getLog10K(j));
  }
}

/// Solve realistic case with minerals that precipitate, no redox, no sorption, and then "restore" to check it works correctly, and then check copy-assignment of GeochemicalSystem works properly
TEST(GeochemicalSolverTest, solve3_restore)
{
  // build the model
  const PertinentGeochemicalSystem model(
      db_full,
      {"H2O", "H+", "Cl-", "Na+", "SO4--", "Mg++", "Ca++", "K+", "HCO3-", "SiO2(aq)", "O2(aq)"},
      {"Antigorite",
       "Tremolite",
       "Talc",
       "Chrysotile",
       "Sepiolite",
       "Anthophyllite",
       "Dolomite",
       "Dolomite-ord",
       "Huntite",
       "Dolomite-dis",
       "Magnesite",
       "Calcite",
       "Aragonite",
       "Quartz"},
      {},
      {},
      {},
      {},
      "O2(aq)",
      "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  // build the equilibrium system
  GeochemistrySpeciesSwapper swapper(11, 1E-6);
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION};
  const std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu = {
      GeochemistryUnitConverter::GeochemistryUnit::KG,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLAL,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES};
  GeochemicalSystem egs(
      mgd,
      ac_solver,
      is_solver,
      swapper,
      {"H+"},
      {"MgCO3"},
      "Cl-",
      {"H2O", "MgCO3", "O2(aq)", "Cl-", "Na+", "SO4--", "Mg++", "Ca++", "K+", "HCO3-", "SiO2(aq)"},
      {1.0,
       0.196E-3,
       0.2151E-3,
       0.5656,
       0.4850,
       0.02924,
       0.05501,
       0.01063,
       0.010576055,
       0.002412,
       0.00010349},
      cu,
      cm,
      25,
      0,
      1E-20,
      {},
      {},
      {});

  // build solver
  GeochemicalSolver solver(mgd.basis_species_name.size(),
                           mgd.kin_species_name.size(),
                           is_solver,
                           1.0E-15,
                           1.0E-200,
                           100,
                           1E3,
                           0.1,
                           5,
                           {"Dolomite-ord", "Dolomite-dis"},
                           3.0,
                           10,
                           false);

  // solve
  std::stringstream ss;
  unsigned tot_iter;
  Real abs_residual;
  DenseVector<Real> mole_additions(egs.getNumInBasis() + egs.getNumKinetic());
  DenseMatrix<Real> dmole_additions(egs.getNumInBasis() + egs.getNumKinetic(),
                                    egs.getNumInBasis() + egs.getNumKinetic());
  solver.solveSystem(egs, ss, tot_iter, abs_residual, 0.0, mole_additions, dmole_additions);
  const Real old_residual = abs_residual;

  // check converged
  EXPECT_LE(tot_iter, (unsigned)100);
  EXPECT_LE(abs_residual, 1.0E-15);

  // retrieve the molalities, and set them into egs: this should result in no change if the
  // "restore" is working correctly
  std::vector<std::string> names = mgd.basis_species_name;
  names.insert(names.end(), mgd.eqm_species_name.begin(), mgd.eqm_species_name.end());
  std::vector<Real> molal = egs.getSolventMassAndFreeMolalityAndMineralMoles();
  for (unsigned j = 0; j < egs.getNumInEquilibrium(); ++j)
    molal.push_back(egs.getEquilibriumMolality(j));
  // form constraints_from_molalities, which are all false
  const std::vector<bool> com_false(egs.getNumInBasis(), false);
  egs.setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
      names, molal, com_false);

  // build solver with no ramping of the maximum ionic strength
  GeochemicalSolver solver0(mgd.basis_species_name.size(),
                            mgd.kin_species_name.size(),
                            is_solver,
                            1.0E-15,
                            1.0E-200,
                            100,
                            1E3,
                            0.1,
                            0,
                            {"Dolomite-ord", "Dolomite-dis"},
                            3.0,
                            0,
                            false);

  // solve this: no swaps should be necessary
  solver0.solveSystem(egs, ss, tot_iter, abs_residual, 0.0, mole_additions, dmole_additions);

  // check that the soler thinks this is truly a solution and the residual has not changed
  EXPECT_EQ(tot_iter, (unsigned)0);
  EXPECT_EQ(abs_residual, old_residual);

  // Now use constraints_from_molalities = true
  const std::vector<bool> com_true(egs.getNumInBasis(), true);
  egs.setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
      names, molal, com_true);

  solver0.solveSystem(egs, ss, tot_iter, abs_residual, 0.0, mole_additions, dmole_additions);

  // check that the soler thinks this is truly a solution and the residual has not increased
  EXPECT_EQ(tot_iter, (unsigned)0);
  EXPECT_LE(abs_residual, old_residual);

  // now check the copy-assignment of GeochemicalSystem
  const PertinentGeochemicalSystem model_dest(
      db_full,
      {"H2O", "H+", "Cl-", "Na+", "SO4--", "Mg++", "Ca++", "K+", "HCO3-", "SiO2(aq)", "O2(aq)"},
      {"Antigorite",
       "Tremolite",
       "Talc",
       "Chrysotile",
       "Sepiolite",
       "Anthophyllite",
       "Dolomite",
       "Dolomite-ord",
       "Huntite",
       "Dolomite-dis",
       "Magnesite",
       "Calcite",
       "Aragonite",
       "Quartz"},
      {},
      {},
      {},
      {},
      "O2(aq)",
      "e-");
  ModelGeochemicalDatabase mgd_dest = model_dest.modelGeochemicalDatabase();
  GeochemicalSystem egs_dest(
      mgd_dest,
      ac_solver,
      is_solver,
      swapper,
      {"H+"},
      {"MgCO3"},
      "Cl-",
      {"H2O", "MgCO3", "O2(aq)", "Cl-", "Na+", "SO4--", "Mg++", "Ca++", "K+", "HCO3-", "SiO2(aq)"},
      {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
      cu,
      cm,
      25,
      0,
      1E-20,
      {},
      {},
      {});

  // change constraint meanings in egs to provide a more thorough check of the copy assignment
  egs.closeSystem();
  // copy assignment
  egs_dest = egs;
  EXPECT_EQ(egs_dest, egs);
}

/// Solve realistic case with redox disequilibrium, no minerals, no sorption
TEST(GeochemicalSolverTest, solve4)
{
  // build the model
  const PertinentGeochemicalSystem model(db_full,
                                         {"H2O",
                                          "H+",
                                          "Cl-",
                                          "O2(aq)",
                                          "HCO3-",
                                          "Ca++",
                                          "Mg++",
                                          "Na+",
                                          "K+",
                                          "Fe++",
                                          "Fe+++",
                                          "Mn++",
                                          "Zn++",
                                          "SO4--"},
                                         {},
                                         {},
                                         {},
                                         {},
                                         {},
                                         "O2(aq)",
                                         "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  // build the equilibrium system
  GeochemistrySpeciesSwapper swapper(14, 1E-6);
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
      GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION};
  const std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu = {
      GeochemistryUnitConverter::GeochemistryUnit::KG,
      GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS,
      GeochemistryUnitConverter::GeochemistryUnit::MOLAL,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES};
  GeochemicalSystem egs(mgd,
                        ac_solver,
                        is_solver,
                        swapper,
                        {},
                        {},
                        "Cl-",
                        {"H2O",
                         "H+",
                         "O2(aq)",
                         "Cl-",
                         "HCO3-",
                         "Ca++",
                         "Mg++",
                         "Na+",
                         "K+",
                         "Fe++",
                         "Fe+++",
                         "Mn++",
                         "Zn++",
                         "SO4--"},
                        {1.0,
                         0.8913E-6,
                         0.13438E-3,
                         3.041E-5,
                         0.0295E-3,
                         0.005938E-3,
                         0.01448E-3,
                         0.0018704E-3,
                         0.005115E-3,
                         0.012534E-3,
                         0.0005372E-3,
                         0.005042E-3,
                         0.001897E-3,
                         0.01562E-3},
                        cu,
                        cm,
                        25,
                        0,
                        1E-20,
                        {},
                        {},
                        {});

  // build solver
  GeochemicalSolver solver(mgd.basis_species_name.size(),
                           mgd.kin_species_name.size(),
                           is_solver,
                           1.0E-15,
                           1.0E-200,
                           100,
                           1E-2,
                           0.1,
                           1,
                           {},
                           3.0,
                           10,
                           false);

  // solve
  std::stringstream ss;
  unsigned tot_iter;
  Real abs_residual;
  DenseVector<Real> mole_additions(egs.getNumInBasis() + egs.getNumKinetic());
  DenseMatrix<Real> dmole_additions(egs.getNumInBasis() + egs.getNumKinetic(),
                                    egs.getNumInBasis() + egs.getNumKinetic());
  solver.solveSystem(egs, ss, tot_iter, abs_residual, 0.0, mole_additions, dmole_additions);

  // check converged
  EXPECT_LE(tot_iter, (unsigned)100);
  EXPECT_LE(abs_residual, 1.0E-15);

  // check number in basis, number in redox disequilibrium and number of surface potentials
  EXPECT_EQ(egs.getNumInBasis(), (unsigned)14);
  EXPECT_EQ(egs.getNumRedox(), (unsigned)2);
  EXPECT_EQ(egs.getNumSurfacePotentials(), (unsigned)0);
  EXPECT_EQ(egs.getNumInEquilibrium(), mgd.eqm_species_name.size());

  // check that the constraints are satisfied
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
    if (mgd.basis_species_name[i] == "H2O")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getSolventWaterMass(), 1.0, 1.0E-15);
      EXPECT_NEAR(egs.getSolventMassAndFreeMolalityAndMineralMoles()[0], 1.0, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "H+")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_TRUE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBasisActivity(i), 0.8913E-6, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "O2(aq)")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getSolventMassAndFreeMolalityAndMineralMoles()[i], 0.13438E-3, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Cl-")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      // do not know the bulk composition as it is dictated by charge neutrality
      EXPECT_EQ(egs.getChargeBalanceBasisIndex(), i);
    }
    else if (mgd.basis_species_name[i] == "HCO3-")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 0.0295E-3, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Ca++")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 0.005938E-3, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Mg++")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 0.01448E-3, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Na+")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 0.0018704E-3, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "K+")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 0.005115E-3, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Fe++")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 0.012534E-3, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Fe+++")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 0.0005372E-3, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Mn++")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 0.005042E-3, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Zn++")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 0.001897E-3, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "SO4--")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 0.01562E-3, 1.0E-15);
    }
    else
      FAIL() << "Incorrect basis species";

  // check total charge
  EXPECT_NEAR(egs.getTotalChargeOld(), 0.0, 1E-15);
  // check total charge by summing up basis charges
  Real tot_charge = 0.0;
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
    tot_charge += egs.getBulkMolesOld()[i] * mgd.basis_species_charge[i];
  EXPECT_NEAR(tot_charge, 0.0, 1E-15);

  // surface charges
  for (unsigned sp = 0; sp < egs.getNumSurfacePotentials(); ++sp)
  {
    Real charge = 0.0;
    for (unsigned j = 0; j < mgd.eqm_species_name.size(); ++j)
      if (mgd.surface_sorption_related[j] && mgd.surface_sorption_number[j] == sp)
        charge += mgd.eqm_species_charge[j] * egs.getEquilibriumMolality(j);
    EXPECT_NEAR(charge,
                egs.getSorbingSurfaceArea()[sp] * egs.getSurfaceCharge(sp) /
                    GeochemistryConstants::FARADAY,
                1E-15);
  }

  // check basis activity = activity_coefficient * molality
  for (unsigned i = 1; i < egs.getNumInBasis(); ++i) // don't loop over water
    if (mgd.basis_species_gas[i] || mgd.basis_species_mineral[i] || egs.getBasisActivityKnown()[i])
      continue;
    else
      EXPECT_NEAR(egs.getBasisActivity(i),
                  egs.getBasisActivityCoefficient(i) *
                      egs.getSolventMassAndFreeMolalityAndMineralMoles()[i],
                  1E-15);

  // check residuals are zero
  for (unsigned a = 0; a < egs.getNumInAlgebraicSystem(); ++a)
    EXPECT_LE(std::abs(egs.getResidualComponent(a, mole_additions)), 1E-15);
  // check residuals are zero by summing molalities, bulk compositions, etc
  Real nw = egs.getSolventWaterMass();
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
  {
    Real res = -egs.getBulkMolesOld()[i];
    if (i == 0)
      res += nw * GeochemistryConstants::MOLES_PER_KG_WATER;
    else if (mgd.basis_species_mineral[i])
      res += egs.getSolventMassAndFreeMolalityAndMineralMoles()[i];
    else if (mgd.basis_species_gas[i])
      res += 0.0;
    else
      res += nw * egs.getSolventMassAndFreeMolalityAndMineralMoles()[i];
    for (unsigned j = 0; j < mgd.eqm_species_name.size(); ++j)
      res += nw * mgd.eqm_stoichiometry(j, i) * egs.getEquilibriumMolality(j);
    EXPECT_LE(std::abs(res), 1E-14);
  }
  // surface potentials
  const Real prefactor = std::sqrt(GeochemistryConstants::GAS_CONSTANT *
                                   (25.0 + GeochemistryConstants::CELSIUS_TO_KELVIN) *
                                   GeochemistryConstants::PERMITTIVITY_FREE_SPACE *
                                   GeochemistryConstants::DIELECTRIC_CONSTANT_WATER *
                                   GeochemistryConstants::DENSITY_WATER * egs.getIonicStrength()) /
                         nw;
  for (unsigned sp = 0; sp < egs.getNumSurfacePotentials(); ++sp)
  {
    EXPECT_NEAR(prefactor * std::sinh(egs.getSurfacePotential(sp) * GeochemistryConstants::FARADAY /
                                      2.0 / GeochemistryConstants::GAS_CONSTANT /
                                      (25.0 + GeochemistryConstants::CELSIUS_TO_KELVIN)),
                egs.getSurfaceCharge(sp),
                1.0E-13);
  }

  // check equilibrium mass balance
  for (unsigned j = 0; j < mgd.eqm_species_name.size(); ++j)
  {
    if (mgd.eqm_species_mineral[j] || mgd.eqm_species_gas[j])
      continue;
    Real log10ap = 0.0;
    for (unsigned i = 0; i < mgd.basis_species_name.size(); ++i)
      log10ap += mgd.eqm_stoichiometry(j, i) * std::log10(egs.getBasisActivity(i));
    log10ap -= egs.getLog10K(j);
    if (mgd.surface_sorption_related[j])
      log10ap -= std::log10(egs.getSurfacePotential(mgd.surface_sorption_number[j]));
    else
      log10ap -= std::log10(egs.getEquilibriumActivityCoefficient(j));
    if (log10ap < -300.0)
      EXPECT_LE(std::abs(egs.getEquilibriumMolality(j)), 1E-25);
    else
      EXPECT_LE(std::abs(egs.getEquilibriumMolality(j) - std::pow(10.0, log10ap)), 1E-15);
  }

  // check equilibrium species have negative saturation indices, except for prevent_precipitation
  // minerals
  for (unsigned j = 0; j < mgd.eqm_species_name.size(); ++j)
  {
    if (!mgd.eqm_species_mineral[j])
      continue;
    Real log10ap = 0.0;
    for (unsigned i = 0; i < mgd.basis_species_name.size(); ++i)
      log10ap += mgd.eqm_stoichiometry(j, i) * std::log10(egs.getBasisActivity(i));
    EXPECT_LE(log10ap, egs.getLog10K(j));
  }
}

/// Solve realistic case with redox disequilibrium, no minerals, no sorption, then "restore" the solution and check
TEST(GeochemicalSolverTest, solve4_restore)
{
  // build the model
  const PertinentGeochemicalSystem model(db_full,
                                         {"H2O",
                                          "H+",
                                          "Cl-",
                                          "O2(aq)",
                                          "HCO3-",
                                          "Ca++",
                                          "Mg++",
                                          "Na+",
                                          "K+",
                                          "Fe++",
                                          "Fe+++",
                                          "Mn++",
                                          "Zn++",
                                          "SO4--"},
                                         {},
                                         {},
                                         {},
                                         {},
                                         {},
                                         "O2(aq)",
                                         "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  // build the equilibrium system
  GeochemistrySpeciesSwapper swapper(14, 1E-6);
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
      GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION};
  const std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu = {
      GeochemistryUnitConverter::GeochemistryUnit::KG,
      GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS,
      GeochemistryUnitConverter::GeochemistryUnit::MOLAL,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES};
  GeochemicalSystem egs(mgd,
                        ac_solver,
                        is_solver,
                        swapper,
                        {},
                        {},
                        "Cl-",
                        {"H2O",
                         "H+",
                         "O2(aq)",
                         "Cl-",
                         "HCO3-",
                         "Ca++",
                         "Mg++",
                         "Na+",
                         "K+",
                         "Fe++",
                         "Fe+++",
                         "Mn++",
                         "Zn++",
                         "SO4--"},
                        {1.0,
                         0.8913E-6,
                         0.13438E-3,
                         3.041E-5,
                         0.0295E-3,
                         0.005938E-3,
                         0.01448E-3,
                         0.0018704E-3,
                         0.005115E-3,
                         0.012534E-3,
                         0.0005372E-3,
                         0.005042E-3,
                         0.001897E-3,
                         0.01562E-3},
                        cu,
                        cm,
                        25,
                        0,
                        1E-20,
                        {},
                        {},
                        {});

  // build solver
  GeochemicalSolver solver(mgd.basis_species_name.size(),
                           mgd.kin_species_name.size(),
                           is_solver,
                           1.0E-15,
                           1.0E-200,
                           100,
                           1E-2,
                           0.1,
                           1,
                           {},
                           3.0,
                           10,
                           false);

  // solve
  std::stringstream ss;
  unsigned tot_iter;
  Real abs_residual;
  DenseVector<Real> mole_additions(egs.getNumInBasis() + egs.getNumKinetic());
  DenseMatrix<Real> dmole_additions(egs.getNumInBasis() + egs.getNumKinetic(),
                                    egs.getNumInBasis() + egs.getNumKinetic());
  solver.solveSystem(egs, ss, tot_iter, abs_residual, 0.0, mole_additions, dmole_additions);
  const Real old_residual = abs_residual;

  // check converged
  EXPECT_LE(tot_iter, (unsigned)100);
  EXPECT_LE(abs_residual, 1.0E-15);

  // retrieve the molalities, and set them into egs: this should result in no change if the
  // "restore" is working correctly
  std::vector<std::string> names = mgd.basis_species_name;
  names.insert(names.end(), mgd.eqm_species_name.begin(), mgd.eqm_species_name.end());
  std::vector<Real> molal = egs.getSolventMassAndFreeMolalityAndMineralMoles();
  for (unsigned j = 0; j < egs.getNumInEquilibrium(); ++j)
    molal.push_back(egs.getEquilibriumMolality(j));
  // form constraints_from_molalities, which are all false
  const std::vector<bool> com_false(egs.getNumInBasis(), false);
  egs.setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
      names, molal, com_false);

  // build solver with no ramping of the maximum ionic strength
  GeochemicalSolver solver0(mgd.basis_species_name.size(),
                            mgd.kin_species_name.size(),
                            is_solver,
                            1.0E-15,
                            1.0E-200,
                            100,
                            1E-2,
                            0.1,
                            1,
                            {},
                            3.0,
                            0,
                            false);

  solver0.solveSystem(egs, ss, tot_iter, abs_residual, 0.0, mole_additions, dmole_additions);

  // check that the soler thinks this is truly a solution and the residual has not changed (up to
  // precision-loss)
  EXPECT_EQ(tot_iter, (unsigned)0);
  EXPECT_LE(abs_residual, old_residual);

  // Now use constraints_from_molalities = true
  const std::vector<bool> com_true(egs.getNumInBasis(), true);
  egs.setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
      names, molal, com_true);

  solver0.solveSystem(egs, ss, tot_iter, abs_residual, 0.0, mole_additions, dmole_additions);

  // check that the soler thinks this is truly a solution and the residual has not increased
  EXPECT_EQ(tot_iter, (unsigned)0);
  EXPECT_LE(abs_residual, old_residual);
}

/// Solve realistic case with sorption and minerals, no redox
TEST(GeochemicalSolverTest, solve5)
{
  // build the model
  const PertinentGeochemicalSystem model(
      db_ferric,
      {"H2O", "H+", "Na+", "Cl-", "Hg++", "Pb++", "SO4--", "Fe+++", ">(s)FeOH", ">(w)FeOH"},
      {"Fe(OH)3(ppd)"},
      {},
      {},
      {},
      {},
      "O2(aq)",
      "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  // build the equilibrium system
  GeochemistrySpeciesSwapper swapper(10, 1E-6);
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
      GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_MINERAL,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION};
  const std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu = {
      GeochemistryUnitConverter::GeochemistryUnit::KG,
      GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES};
  GeochemicalSystem egs(
      mgd,
      ac_solver,
      is_solver,
      swapper,
      {"Fe+++"},
      {"Fe(OH)3(ppd)"},
      "Cl-",
      {"H2O", "H+", "Na+", "Cl-", "Hg++", "Pb++", "SO4--", "Fe(OH)3(ppd)", ">(s)FeOH", ">(w)FeOH"},
      {1.0, 1.0E-4, 10E-3, 10E-3, 0.1E-3, 0.1E-3, 0.2E-3, 9.3573E-3, 4.6786E-5, 1.87145E-3},
      cu,
      cm,
      25,
      0,
      1E-20,
      {},
      {},
      {});

  // build solver
  GeochemicalSolver solver(mgd.basis_species_name.size(),
                           mgd.kin_species_name.size(),
                           is_solver,
                           1.0E-15,
                           1.0E-200,
                           100,
                           1.0,
                           0.1,
                           1,
                           {},
                           3.0,
                           10,
                           false);

  // solve
  std::stringstream ss;
  unsigned tot_iter;
  Real abs_residual;
  DenseVector<Real> mole_additions(egs.getNumInBasis() + egs.getNumKinetic());
  DenseMatrix<Real> dmole_additions(egs.getNumInBasis() + egs.getNumKinetic(),
                                    egs.getNumInBasis() + egs.getNumKinetic());
  solver.solveSystem(egs, ss, tot_iter, abs_residual, 0.0, mole_additions, dmole_additions);

  // check converged
  EXPECT_LE(tot_iter, (unsigned)100);
  EXPECT_LE(abs_residual, 1.0E-15);

  // check number in basis, number in redox disequilibrium and number of surface potentials
  EXPECT_EQ(egs.getNumInBasis(), (unsigned)10);
  EXPECT_EQ(egs.getNumRedox(), (unsigned)0);
  EXPECT_EQ(egs.getNumSurfacePotentials(), (unsigned)1);
  EXPECT_EQ(egs.getNumInEquilibrium(), mgd.eqm_species_name.size());

  // check that the constraints are satisfied
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
    if (mgd.basis_species_name[i] == "H2O")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getSolventWaterMass(), 1.0, 1.0E-15);
      EXPECT_NEAR(egs.getSolventMassAndFreeMolalityAndMineralMoles()[0], 1.0, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "H+")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_TRUE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBasisActivity(i), 1.0E-4, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Na+")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 10E-3, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Cl-")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      // do not know the bulk composition as it is dictated by charge neutrality
      EXPECT_EQ(egs.getChargeBalanceBasisIndex(), i);
    }
    else if (mgd.basis_species_name[i] == "Hg++")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 0.1E-3, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Pb++")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 0.1E-3, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "SO4--")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 0.2E-3, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Fe(OH)3(ppd)")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_TRUE(mgd.basis_species_mineral[i]);
      EXPECT_TRUE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getSolventMassAndFreeMolalityAndMineralMoles()[i], 9.3573E-3, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == ">(s)FeOH")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 4.6786E-5, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == ">(w)FeOH")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 1.87145E-3, 1.0E-15);
    }
    else
      FAIL() << "Incorrect basis species";

  // check total charge
  EXPECT_NEAR(egs.getTotalChargeOld(), 0.0, 1E-15);
  // check total charge by summing up basis charges
  Real tot_charge = 0.0;
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
    tot_charge += egs.getBulkMolesOld()[i] * mgd.basis_species_charge[i];
  EXPECT_NEAR(tot_charge, 0.0, 1E-15);

  // surface charges
  for (unsigned sp = 0; sp < egs.getNumSurfacePotentials(); ++sp)
  {
    Real charge = 0.0;
    for (unsigned j = 0; j < mgd.eqm_species_name.size(); ++j)
      if (mgd.surface_sorption_related[j] && mgd.surface_sorption_number[j] == sp)
        charge += mgd.eqm_species_charge[j] * egs.getEquilibriumMolality(j);
    EXPECT_NEAR(charge,
                egs.getSorbingSurfaceArea()[sp] * egs.getSurfaceCharge(sp) /
                    GeochemistryConstants::FARADAY,
                1E-15);
  }

  // check basis activity = activity_coefficient * molality
  for (unsigned i = 1; i < egs.getNumInBasis(); ++i) // don't loop over water
    if (mgd.basis_species_gas[i] || mgd.basis_species_mineral[i] || egs.getBasisActivityKnown()[i])
      continue;
    else
      EXPECT_NEAR(egs.getBasisActivity(i),
                  egs.getBasisActivityCoefficient(i) *
                      egs.getSolventMassAndFreeMolalityAndMineralMoles()[i],
                  1E-15);

  // check residuals are zero
  for (unsigned a = 0; a < egs.getNumInAlgebraicSystem(); ++a)
    EXPECT_LE(std::abs(egs.getResidualComponent(a, mole_additions)), 1E-15);
  // check residuals are zero by summing molalities, bulk compositions, etc
  Real nw = egs.getSolventWaterMass();
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
  {
    Real res = -egs.getBulkMolesOld()[i];
    if (i == 0)
      res += nw * GeochemistryConstants::MOLES_PER_KG_WATER;
    else if (mgd.basis_species_mineral[i])
      res += egs.getSolventMassAndFreeMolalityAndMineralMoles()[i];
    else if (mgd.basis_species_gas[i])
      res += 0.0;
    else
      res += nw * egs.getSolventMassAndFreeMolalityAndMineralMoles()[i];
    for (unsigned j = 0; j < mgd.eqm_species_name.size(); ++j)
      res += nw * mgd.eqm_stoichiometry(j, i) * egs.getEquilibriumMolality(j);
    EXPECT_LE(std::abs(res), 1E-14);
  }
  // surface potentials
  const Real prefactor = std::sqrt(8.0 * GeochemistryConstants::GAS_CONSTANT *
                                   (25.0 + GeochemistryConstants::CELSIUS_TO_KELVIN) *
                                   GeochemistryConstants::PERMITTIVITY_FREE_SPACE *
                                   GeochemistryConstants::DIELECTRIC_CONSTANT_WATER *
                                   GeochemistryConstants::DENSITY_WATER * egs.getIonicStrength()) /
                         nw;
  for (unsigned sp = 0; sp < egs.getNumSurfacePotentials(); ++sp)
  {
    EXPECT_NEAR(prefactor * std::sinh(egs.getSurfacePotential(sp) * GeochemistryConstants::FARADAY /
                                      2.0 / GeochemistryConstants::GAS_CONSTANT /
                                      (25.0 + GeochemistryConstants::CELSIUS_TO_KELVIN)),
                egs.getSurfaceCharge(sp),
                1.0E-13);
  }

  // check equilibrium mass balance
  for (unsigned j = 0; j < mgd.eqm_species_name.size(); ++j)
  {
    if (mgd.eqm_species_mineral[j] || mgd.eqm_species_gas[j])
      continue;
    Real log10ap = 0.0;
    for (unsigned i = 0; i < mgd.basis_species_name.size(); ++i)
      log10ap += mgd.eqm_stoichiometry(j, i) * std::log10(egs.getBasisActivity(i));
    log10ap -= egs.getLog10K(j);
    if (mgd.surface_sorption_related[j])
      log10ap -= std::log10(std::exp(mgd.eqm_species_charge[j] * GeochemistryConstants::FARADAY *
                                     egs.getSurfacePotential(mgd.surface_sorption_number[j]) /
                                     GeochemistryConstants::GAS_CONSTANT /
                                     (25.0 + GeochemistryConstants::CELSIUS_TO_KELVIN)));
    else
      log10ap -= std::log10(egs.getEquilibriumActivityCoefficient(j));
    if (log10ap < -300.0)
      EXPECT_LE(std::abs(egs.getEquilibriumMolality(j)), 1E-25);
    else
      EXPECT_LE(std::abs(egs.getEquilibriumMolality(j) - std::pow(10.0, log10ap)), 1E-15);
  }

  // check equilibrium species have negative saturation indices, except for prevent_precipitation
  // minerals
  for (unsigned j = 0; j < mgd.eqm_species_name.size(); ++j)
  {
    if (!mgd.eqm_species_mineral[j])
      continue;
    Real log10ap = 0.0;
    for (unsigned i = 0; i < mgd.basis_species_name.size(); ++i)
      log10ap += mgd.eqm_stoichiometry(j, i) * std::log10(egs.getBasisActivity(i));
    EXPECT_LE(log10ap, egs.getLog10K(j));
  }
}

/// Repeat the realistic case with sorption and minerals, no redox, this time with a "restore", and then use the copy assignment operator of GeochemicalSystem to create a new geochemical system
TEST(GeochemicalSolverTest, solve5_restore)
{
  // build the model
  const PertinentGeochemicalSystem model(
      db_ferric,
      {"H2O", "H+", "Na+", "Cl-", "Hg++", "Pb++", "SO4--", "Fe+++", ">(s)FeOH", ">(w)FeOH"},
      {"Fe(OH)3(ppd)"},
      {},
      {},
      {},
      {},
      "O2(aq)",
      "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  // build the equilibrium system
  GeochemistrySpeciesSwapper swapper(10, 1E-6);
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
      GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_MINERAL,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION};
  const std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu = {
      GeochemistryUnitConverter::GeochemistryUnit::KG,
      GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES};
  GeochemicalSystem egs(
      mgd,
      ac_solver,
      is_solver,
      swapper,
      {"Fe+++"},
      {"Fe(OH)3(ppd)"},
      "Cl-",
      {"H2O", "H+", "Na+", "Cl-", "Hg++", "Pb++", "SO4--", "Fe(OH)3(ppd)", ">(s)FeOH", ">(w)FeOH"},
      {1.0, 1.0E-4, 10E-3, 10E-3, 0.1E-3, 0.1E-3, 0.2E-3, 9.3573E-3, 4.6786E-5, 1.87145E-3},
      cu,
      cm,
      25,
      0,
      1E-20,
      {},
      {},
      {});

  // build solver
  GeochemicalSolver solver(mgd.basis_species_name.size(),
                           mgd.kin_species_name.size(),
                           is_solver,
                           1.0E-15,
                           1.0E-200,
                           100,
                           1.0,
                           0.1,
                           1,
                           {},
                           3.0,
                           0,
                           false);

  // solve
  std::stringstream ss;
  unsigned tot_iter;
  Real abs_residual;
  DenseVector<Real> mole_additions(egs.getNumInBasis() + egs.getNumKinetic());
  DenseMatrix<Real> dmole_additions(egs.getNumInBasis() + egs.getNumKinetic(),
                                    egs.getNumInBasis() + egs.getNumKinetic());
  solver.solveSystem(egs, ss, tot_iter, abs_residual, 0.0, mole_additions, dmole_additions);
  const Real old_residual = abs_residual;

  // check converged
  EXPECT_LE(tot_iter, (unsigned)100);
  EXPECT_LE(abs_residual, 1.0E-15);

  // now setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles to the solution
  // obtained This should actually do nothing: we are just checking the "restore" doesn't muck
  // something up!  Of course, activity coefficients will slightly change due to making the
  // configuration slightly more consistent (activities and molalities match) so there will be very
  // minor changes
  std::vector<std::string> names = mgd.basis_species_name;
  names.insert(names.end(), mgd.eqm_species_name.begin(), mgd.eqm_species_name.end());
  names.push_back("Fe(OH)3(ppd)_surface_potential_expr");
  std::vector<Real> molal = egs.getSolventMassAndFreeMolalityAndMineralMoles();
  for (unsigned j = 0; j < egs.getNumInEquilibrium(); ++j)
    molal.push_back(egs.getEquilibriumMolality(j));
  molal.push_back(std::exp(egs.getSurfacePotential(0) * GeochemistryConstants::FARADAY /
                           (-2.0 * GeochemistryConstants::GAS_CONSTANT *
                            (25.0 + GeochemistryConstants::CELSIUS_TO_KELVIN))));
  // form constraints_from_molalities, which are all false
  const std::vector<bool> com_false(egs.getNumInBasis(), false);
  egs.setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
      names, molal, com_false);

  solver.solveSystem(egs, ss, tot_iter, abs_residual, 0.0, mole_additions, dmole_additions);
  EXPECT_EQ(tot_iter, (unsigned)0);
  EXPECT_LE(abs_residual, 2.0 * old_residual);

  // Now use constraints_from_molalities = true
  const std::vector<bool> com_true(egs.getNumInBasis(), true);
  egs.setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
      names, molal, com_true);

  solver.solveSystem(egs, ss, tot_iter, abs_residual, 0.0, mole_additions, dmole_additions);

  EXPECT_EQ(tot_iter, (unsigned)0);
  EXPECT_LE(abs_residual, 2.0 * old_residual);

  // now check the copy-assignment operator of GeochemicalSystem

  // build the destination model
  const PertinentGeochemicalSystem dest_model(
      db_ferric,
      {"H2O", "H+", "Na+", "Cl-", "Hg++", "Pb++", "SO4--", "Fe+++", ">(s)FeOH", ">(w)FeOH"},
      {"Fe(OH)3(ppd)"},
      {},
      {},
      {},
      {},
      "O2(aq)",
      "e-");
  ModelGeochemicalDatabase dest_mgd = dest_model.modelGeochemicalDatabase();

  // build the destination equilibrium system
  GeochemicalSystem dest_egs(
      dest_mgd,
      ac_solver,
      is_solver,
      swapper,
      {"Fe+++"},
      {"Fe(OH)3(ppd)"},
      "Cl-",
      {"H2O", "H+", "Na+", "Cl-", "Hg++", "Pb++", "SO4--", "Fe(OH)3(ppd)", ">(s)FeOH", ">(w)FeOH"},
      {2.0, 2.0E-4, 20E-3, 20E-3, 0.2E-3, 0.2E-3, 0.4E-3, 19.3573E-3, 14.6786E-5, 2.87145E-3},
      cu,
      cm,
      250,
      0,
      1E-20,
      {},
      {},
      {});

  // change constraint meanings in egs to provide a more thorough check of the copy assignment
  egs.closeSystem();
  // copy assignment
  dest_egs = egs;
  EXPECT_EQ(dest_egs, egs);
}

/// Test progressively adding H+ to a system
TEST(GeochemicalSolverTest, solve_addH)
{
  ModelGeochemicalDatabase mgd = model_simplest.modelGeochemicalDatabase();
  GeochemicalSystem egs(mgd,
                        ac_solver,
                        is_solver,
                        swapper2,
                        {},
                        {},
                        "H+",
                        {"H2O", "H+"},
                        {1.75, 1},
                        cu2,
                        cm2,
                        25,
                        0,
                        1E-20,
                        {},
                        {},
                        {});
  GeochemicalSolver solver(mgd.basis_species_name.size(),
                           mgd.kin_species_name.size(),
                           is_solver,
                           1.0E-12,
                           1.0E-200,
                           100,
                           10.0,
                           0.1,
                           1,
                           {},
                           3.0,
                           0,
                           false);

  std::stringstream ss;
  unsigned tot_iter;
  Real abs_residual;
  DenseVector<Real> mole_additions(egs.getNumInBasis() + egs.getNumKinetic());
  DenseMatrix<Real> dmole_additions(egs.getNumInBasis() + egs.getNumKinetic(),
                                    egs.getNumInBasis() + egs.getNumKinetic());
  solver.solveSystem(egs, ss, tot_iter, abs_residual, 0.0, mole_additions, dmole_additions);

  // check Newton has converged
  EXPECT_LE(tot_iter, (unsigned)100);
  EXPECT_LE(abs_residual, 1.0E-12);

  // check Bulk is as expected
  const std::vector<Real> bulk = egs.getBulkMolesOld();
  EXPECT_NEAR(bulk[0],
              1.75 * (GeochemistryConstants::MOLES_PER_KG_WATER + egs.getEquilibriumMolality(0)),
              1.0E-9);
  EXPECT_NEAR(bulk[1], 0.0, 1.0E-9);

  // close the system by setting to fixed number of moles
  egs.changeConstraintToBulk(0);

  // check Bulk is as expected
  std::vector<Real> bulk_new = egs.getBulkMolesOld();
  for (unsigned i = 0; i < 2; ++i)
    EXPECT_EQ(bulk_new[i], bulk[i]);

  // add 1 mol of water
  mole_additions(0) = 1.0;
  solver.solveSystem(egs, ss, tot_iter, abs_residual, 0.0, mole_additions, dmole_additions);

  // check Newton has converged
  EXPECT_LE(tot_iter, (unsigned)100);
  EXPECT_LE(abs_residual, 1.0E-12);

  // check Bulk is as expected
  bulk_new = egs.getBulkMolesOld();
  EXPECT_EQ(bulk_new[0], bulk[0] + 1.0);
  EXPECT_EQ(bulk_new[1], bulk[1]);

  // add 1 mol of H+ : this shouldn't make any difference because charge-balance instantly removes
  // it!
  mole_additions(0) = 0.0;
  mole_additions(1) = 1.0;
  solver.solveSystem(egs, ss, tot_iter, abs_residual, 0.0, mole_additions, dmole_additions);

  // check Newton has converged
  EXPECT_LE(tot_iter, (unsigned)100);
  EXPECT_LE(abs_residual, 1.0E-12);

  // check Bulk is as expected
  bulk_new = egs.getBulkMolesOld();
  EXPECT_EQ(bulk_new[0], bulk[0] + 1.0);
  EXPECT_EQ(bulk_new[1], bulk[1]);
}

/// Test that the max swaps allowed works OK
TEST(GeochemicalSolverTest, maxSwapsException)
{
  // build the model
  const PertinentGeochemicalSystem model(
      db_full,
      {"H2O", "H+", "Cl-", "Na+", "SO4--", "Mg++", "Ca++", "K+", "HCO3-", "SiO2(aq)", "O2(aq)"},
      {"Antigorite",
       "Tremolite",
       "Talc",
       "Chrysotile",
       "Sepiolite",
       "Anthophyllite",
       "Dolomite",
       "Dolomite-ord",
       "Huntite",
       "Dolomite-dis",
       "Magnesite",
       "Calcite",
       "Aragonite",
       "Quartz"},
      {},
      {},
      {},
      {},
      "O2(aq)",
      "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();

  // build the equilibrium system
  GeochemistrySpeciesSwapper swapper(11, 1E-6);
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> cm = {
      GeochemicalSystem::ConstraintUserMeaningEnum::KG_SOLVENT_WATER,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
      GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION};
  const std::vector<GeochemistryUnitConverter::GeochemistryUnit> cu = {
      GeochemistryUnitConverter::GeochemistryUnit::KG,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLAL,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES,
      GeochemistryUnitConverter::GeochemistryUnit::MOLES};
  GeochemicalSystem egs(
      mgd,
      ac_solver,
      is_solver,
      swapper,
      {"H+"},
      {"MgCO3"},
      "Cl-",
      {"H2O", "MgCO3", "O2(aq)", "Cl-", "Na+", "SO4--", "Mg++", "Ca++", "K+", "HCO3-", "SiO2(aq)"},
      {1.0,
       0.196E-3,
       0.2151E-3,
       0.5656,
       0.4850,
       0.02924,
       0.05501,
       0.01063,
       0.010576055,
       0.002412,
       0.00010349},
      cu,
      cm,
      25,
      0,
      1E-20,
      {},
      {},
      {});

  // build solver (4 swaps are needed to solve this system)
  GeochemicalSolver solver(mgd.basis_species_name.size(),
                           mgd.kin_species_name.size(),
                           is_solver,
                           1.0E-15,
                           1.0E-200,
                           100,
                           1E3,
                           0.1,
                           3,
                           {"Dolomite-ord", "Dolomite-dis"},
                           3.0,
                           10,
                           false);

  // solve
  std::stringstream ss;
  unsigned tot_iter;
  Real abs_residual;
  try
  {
    DenseVector<Real> mole_additions(egs.getNumInBasis() + egs.getNumKinetic());
    DenseMatrix<Real> dmole_additions(egs.getNumInBasis() + egs.getNumKinetic(),
                                      egs.getNumInBasis() + egs.getNumKinetic());
    solver.solveSystem(egs, ss, tot_iter, abs_residual, 0.0, mole_additions, dmole_additions);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Maximum number of swaps performed during solve") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check setRampMaxIonicStrength
TEST(GeochemicalSolverTest, setRampMaxIonicStrength)
{
  ModelGeochemicalDatabase mgd = model_simplest.modelGeochemicalDatabase();
  GeochemicalSystem egs(mgd,
                        ac_solver,
                        is_solver,
                        swapper2,
                        {},
                        {},
                        "H+",
                        {"H2O", "H+"},
                        {1.75, 3.0},
                        cu2,
                        cm2,
                        25,
                        0,
                        1E-20,
                        {},
                        {},
                        {});
  GeochemicalSolver solver(mgd.basis_species_name.size(),
                           mgd.kin_species_name.size(),
                           is_solver,
                           1.0,
                           0.1,
                           100,
                           1E100,
                           0.1,
                           1,
                           {},
                           3.0,
                           10,
                           true);

  ASSERT_EQ(solver.getRampMaxIonicStrength(), (unsigned)10);
  try
  {
    solver.setRampMaxIonicStrength(101);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("GeochemicalSolver: ramp_max_ionic_strength must be less than max_iter") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
  solver.setRampMaxIonicStrength(21);
  ASSERT_EQ(solver.getRampMaxIonicStrength(), (unsigned)21);
}

/// Solve case that involves kinetic species with zero rates (so kinetic species should have no impact except to modify the bulk composition)
TEST(GeochemicalSolverTest, solve_kinetic1)
{
  const PertinentGeochemicalSystem model(db_solver,
                                         {"H2O", "H+", "Fe+++", "HCO3-"},
                                         {},
                                         {},
                                         {"Something", "Fe(OH)3(ppd)fake"},
                                         {},
                                         {},
                                         "O2(aq)",
                                         "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> ku;
  ku.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  ku.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  GeochemicalSystem egs(mgd,
                        ac_solver,
                        is_solver,
                        swapper_kin,
                        {},
                        {},
                        "HCO3-",
                        {"H2O", "H+", "Fe+++", "HCO3-"},
                        {1.75, 1E-5, 1E-5, 4E-5},
                        cu4,
                        cm4,
                        25,
                        0,
                        1E-20,
                        {"Something", "Fe(OH)3(ppd)fake"},
                        {1.0E-6, 2.0E-6},
                        ku);
  GeochemicalSolver solver(mgd.basis_species_name.size(),
                           mgd.kin_species_name.size(),
                           is_solver,
                           1.0E-15,
                           1.0E-200,
                           100,
                           1E-5,
                           1E-16,
                           1,
                           {},
                           3.0,
                           0,
                           true);

  std::stringstream ss;
  unsigned tot_iter;
  Real abs_residual;
  DenseVector<Real> mole_additions(egs.getNumInBasis() + egs.getNumKinetic());
  DenseMatrix<Real> dmole_additions(egs.getNumInBasis() + egs.getNumKinetic(),
                                    egs.getNumInBasis() + egs.getNumKinetic());
  solver.solveSystem(egs, ss, tot_iter, abs_residual, 0.0, mole_additions, dmole_additions);

  // check Newton has converged
  EXPECT_LE(tot_iter, (unsigned)100);
  EXPECT_LE(abs_residual, 1.0E-15);

  // check numbers are correct
  EXPECT_EQ(egs.getNumInBasis(), (unsigned)4);
  EXPECT_EQ(egs.getNumRedox(), (unsigned)0);
  EXPECT_EQ(egs.getNumSurfacePotentials(), (unsigned)0);
  EXPECT_EQ(egs.getNumInEquilibrium(), mgd.eqm_species_name.size());
  EXPECT_EQ(egs.getNumKinetic(), (unsigned)2);

  // check that kinetic moles have not changed (kinetic rates are zero)
  for (unsigned k = 0; k < egs.getNumKinetic(); ++k)
    if (mgd.kin_species_name[k] == "Something")
      EXPECT_EQ(egs.getKineticMoles(k), 1.0E-6);
    else if (mgd.kin_species_name[k] == "Fe(OH)3(ppd)fake")
      EXPECT_EQ(egs.getKineticMoles(k), 2.0E-6);
    else
      FAIL() << "Incorrect kinetic species";

  // check that the constraints are satisfied
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
    if (mgd.basis_species_name[i] == "H2O")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getSolventWaterMass(), 1.75, 1.0E-15);
      EXPECT_NEAR(egs.getSolventMassAndFreeMolalityAndMineralMoles()[0], 1.75, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "H+")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_TRUE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBasisActivity(i), 1E-5, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Fe+++")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 1E-5 + 1E-6 + 2 * 2E-6, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "HCO3-")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      // do not know the bulk composition as it is dictated by charge neutrality
      EXPECT_EQ(egs.getChargeBalanceBasisIndex(), i);
    }
    else
      FAIL() << "Incorrect basis species";

  // check total charge
  EXPECT_NEAR(egs.getTotalChargeOld(), 0.0, 1E-15);
  // check total charge by summing up basis charges
  Real tot_charge = 0.0;
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
    tot_charge += egs.getBulkMolesOld()[i] * mgd.basis_species_charge[i];
  EXPECT_NEAR(tot_charge, 0.0, 1E-15);

  // check basis activity = activity_coefficient * molality
  for (unsigned i = 1; i < egs.getNumInBasis(); ++i) // don't loop over water
    if (mgd.basis_species_gas[i] || mgd.basis_species_mineral[i] || egs.getBasisActivityKnown()[i])
      continue;
    else
      EXPECT_NEAR(egs.getBasisActivity(i),
                  egs.getBasisActivityCoefficient(i) *
                      egs.getSolventMassAndFreeMolalityAndMineralMoles()[i],
                  1E-15);

  // check residuals are zero
  for (unsigned a = 0; a < egs.getNumInAlgebraicSystem(); ++a)
    EXPECT_LE(std::abs(egs.getResidualComponent(a, mole_additions)), 1E-15);
  // check residuals are zero by summing molalities, bulk compositions, etc
  Real nw = egs.getSolventWaterMass();
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
  {
    Real res = -egs.getBulkMolesOld()[i];
    if (i == 0)
      res += nw * GeochemistryConstants::MOLES_PER_KG_WATER;
    else if (mgd.basis_species_mineral[i])
      res += egs.getSolventMassAndFreeMolalityAndMineralMoles()[i];
    else if (mgd.basis_species_gas[i])
      res += 0.0;
    else
      res += nw * egs.getSolventMassAndFreeMolalityAndMineralMoles()[i];
    for (unsigned j = 0; j < mgd.eqm_species_name.size(); ++j)
      res += nw * mgd.eqm_stoichiometry(j, i) * egs.getEquilibriumMolality(j);
    for (unsigned k = 0; k < mgd.kin_species_name.size(); ++k)
      res += mgd.kin_stoichiometry(k, i) *
             egs.getKineticMoles(
                 k); // this should be the only important kinetic contribution to this test!
    EXPECT_LE(std::abs(res), 1E-13);
  }

  // check equilibrium mass balance
  for (unsigned j = 0; j < mgd.eqm_species_name.size(); ++j)
  {
    if (mgd.eqm_species_mineral[j] || mgd.eqm_species_gas[j])
      continue;
    Real log10ap = 0.0;
    for (unsigned i = 0; i < mgd.basis_species_name.size(); ++i)
      log10ap += mgd.eqm_stoichiometry(j, i) * std::log10(egs.getBasisActivity(i));
    log10ap -= egs.getLog10K(j);
    if (mgd.surface_sorption_related[j])
      log10ap -= std::log10(egs.getSurfacePotential(mgd.surface_sorption_number[j]));
    else
      log10ap -= std::log10(egs.getEquilibriumActivityCoefficient(j));
    if (log10ap < -300.0)
      EXPECT_LE(std::abs(egs.getEquilibriumMolality(j)), 1E-25);
    else
      EXPECT_LE(std::abs(egs.getEquilibriumMolality(j) - std::pow(10.0, log10ap)), 1E-15);
  }
}

/// Solve case that involves kinetic species with constant rates
TEST(GeochemicalSolverTest, solve_kinetic2)
{
  PertinentGeochemicalSystem model(db_solver,
                                   {"H2O", "H+", "Fe+++", "HCO3-"},
                                   {},
                                   {},
                                   {"Something", "Fe(OH)3(ppd)fake"},
                                   {},
                                   {},
                                   "O2(aq)",
                                   "e-");
  // Define constant rates
  // Something = -3H+ + Fe+++ + 2H2O + 1.5HCO3-, so Q=1E15*1E-5*1E-7=1E3 (approx) >> K, so
  // rate_Something produces a negative rate, so mole_additions > 0, so Something will increase in
  // mole number
  KineticRateUserDescription rate_Something("Something",
                                            1.0E-7,
                                            1.0,
                                            false,
                                            0.0,
                                            0.0,
                                            0.0,
                                            {},
                                            {},
                                            {},
                                            {},
                                            1.0,
                                            0.0,
                                            0.0,
                                            0.0,
                                            DirectionChoiceEnum::BOTH,
                                            "H2O",
                                            0.0,
                                            -1.0,
                                            0.0);
  // Fe(OH)3(ppd)fake = -3H+ + 2Fe+++ + 3H2O, so Q=1E15*1E-10=1E5 (approx) < K, so rate_Fe produces
  // a positive rate, so mole_additions < 0, so Something will decrease in mole number
  KineticRateUserDescription rate_Fe("Fe(OH)3(ppd)fake",
                                     1.0E-7,
                                     1.0,
                                     false,
                                     0.0,
                                     0.0,
                                     0.0,
                                     {},
                                     {},
                                     {},
                                     {},
                                     1.0,
                                     0.0,
                                     0.0,
                                     0.0,
                                     DirectionChoiceEnum::BOTH,
                                     "H2O",
                                     0.0,
                                     -1.0,
                                     0.0);
  model.addKineticRate(rate_Something);
  model.addKineticRate(rate_Fe);

  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> ku;
  ku.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  ku.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  GeochemicalSystem egs(mgd,
                        ac_solver,
                        is_solver,
                        swapper_kin,
                        {},
                        {},
                        "HCO3-",
                        {"H2O", "H+", "Fe+++", "HCO3-"},
                        {1.75, 1E-5, 1E-5, 4E-5},
                        cu4,
                        cm4,
                        25,
                        0,
                        1E-20,
                        {"Something", "Fe(OH)3(ppd)fake"},
                        {1.0E-6, 2.0E-6},
                        ku);
  GeochemicalSolver solver(mgd.basis_species_name.size(),
                           mgd.kin_species_name.size(),
                           is_solver,
                           1.0E-15,
                           1.0E-200,
                           100,
                           1E-5,
                           1E-16,
                           1,
                           {},
                           3.0,
                           0,
                           true);

  const unsigned index_something = mgd.kin_species_index.at("Something");
  const unsigned index_fe = mgd.kin_species_index.at("Fe(OH)3(ppd)fake");

  std::stringstream ss;
  unsigned tot_iter;
  Real abs_residual;
  DenseVector<Real> mole_additions(egs.getNumInBasis() + egs.getNumKinetic());
  mole_additions(egs.getNumInBasis() + index_something) = 1.5E-6; // external input to "Something"
  DenseMatrix<Real> dmole_additions(egs.getNumInBasis() + egs.getNumKinetic(),
                                    egs.getNumInBasis() + egs.getNumKinetic());
  // solve with a time-step size of 10.0
  solver.solveSystem(egs, ss, tot_iter, abs_residual, 10.0, mole_additions, dmole_additions);

  // check Newton has converged
  EXPECT_LE(tot_iter, (unsigned)100);
  EXPECT_LE(abs_residual, 1.0E-15);

  // check numbers are correct
  EXPECT_EQ(egs.getNumInBasis(), (unsigned)4);
  EXPECT_EQ(egs.getNumRedox(), (unsigned)0);
  EXPECT_EQ(egs.getNumSurfacePotentials(), (unsigned)0);
  EXPECT_EQ(egs.getNumInEquilibrium(), mgd.eqm_species_name.size());
  EXPECT_EQ(egs.getNumKinetic(), (unsigned)2);

  // check activity products and log10K are ordered as calculated above
  EXPECT_GT(egs.log10KineticActivityProduct(index_something),
            egs.getKineticLog10K(index_something));
  EXPECT_LT(egs.log10KineticActivityProduct(index_fe), egs.getKineticLog10K(index_fe));

  // check that mole additions is as calculated above
  EXPECT_EQ(mole_additions(egs.getNumInBasis() + index_something), 1.5E-6 + 1.0E-6);
  EXPECT_EQ(mole_additions(egs.getNumInBasis() + index_fe), -1.0E-6);

  // check that kinetic moles have changed according to the constant rates
  for (unsigned k = 0; k < egs.getNumKinetic(); ++k)
    if (mgd.kin_species_name[k] == "Something")
      EXPECT_NEAR(egs.getKineticMoles(k), 2.0E-6 + 1.5E-6, 1.0E-12);
    else if (mgd.kin_species_name[k] == "Fe(OH)3(ppd)fake")
      EXPECT_EQ(egs.getKineticMoles(k), 1.0E-6);
    else
      FAIL() << "Incorrect kinetic species";

  // check that the constraints are satisfied
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
    if (mgd.basis_species_name[i] == "H2O")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getSolventWaterMass(), 1.75, 1.0E-15);
      EXPECT_NEAR(egs.getSolventMassAndFreeMolalityAndMineralMoles()[0], 1.75, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "H+")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_TRUE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBasisActivity(i), 1E-5, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Fe+++")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 1E-5 + 1E-6 + 2 * 2E-6, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "HCO3-")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      // do not know the bulk composition as it is dictated by charge neutrality
      EXPECT_EQ(egs.getChargeBalanceBasisIndex(), i);
    }
    else
      FAIL() << "Incorrect basis species";

  // check total charge
  EXPECT_NEAR(egs.getTotalChargeOld(), 0.0, 1E-15);
  // check total charge by summing up basis charges
  Real tot_charge = 0.0;
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
    tot_charge += egs.getBulkMolesOld()[i] * mgd.basis_species_charge[i];
  EXPECT_NEAR(tot_charge, 0.0, 1E-15);

  // check basis activity = activity_coefficient * molality
  for (unsigned i = 1; i < egs.getNumInBasis(); ++i) // don't loop over water
    if (mgd.basis_species_gas[i] || mgd.basis_species_mineral[i] || egs.getBasisActivityKnown()[i])
      continue;
    else
      EXPECT_NEAR(egs.getBasisActivity(i),
                  egs.getBasisActivityCoefficient(i) *
                      egs.getSolventMassAndFreeMolalityAndMineralMoles()[i],
                  1E-15);

  // check residuals are zero
  mole_additions.zero(); // to remove the kinetic rate contributions
  for (unsigned a = 0; a < egs.getNumInAlgebraicSystem(); ++a)
    EXPECT_LE(std::abs(egs.getResidualComponent(a, mole_additions)), 1E-15);
  // check residuals are zero by summing molalities, bulk compositions, etc
  Real nw = egs.getSolventWaterMass();
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
  {
    Real res = -egs.getBulkMolesOld()[i];
    if (i == 0)
      res += nw * GeochemistryConstants::MOLES_PER_KG_WATER;
    else if (mgd.basis_species_mineral[i])
      res += egs.getSolventMassAndFreeMolalityAndMineralMoles()[i];
    else if (mgd.basis_species_gas[i])
      res += 0.0;
    else
      res += nw * egs.getSolventMassAndFreeMolalityAndMineralMoles()[i];
    for (unsigned j = 0; j < mgd.eqm_species_name.size(); ++j)
      res += nw * mgd.eqm_stoichiometry(j, i) * egs.getEquilibriumMolality(j);
    for (unsigned k = 0; k < mgd.kin_species_name.size(); ++k)
      res += mgd.kin_stoichiometry(k, i) * egs.getKineticMoles(k);
    EXPECT_LE(std::abs(res), 1E-13);
  }

  // check equilibrium mass balance
  for (unsigned j = 0; j < mgd.eqm_species_name.size(); ++j)
  {
    if (mgd.eqm_species_mineral[j] || mgd.eqm_species_gas[j])
      continue;
    Real log10ap = 0.0;
    for (unsigned i = 0; i < mgd.basis_species_name.size(); ++i)
      log10ap += mgd.eqm_stoichiometry(j, i) * std::log10(egs.getBasisActivity(i));
    log10ap -= egs.getLog10K(j);
    if (mgd.surface_sorption_related[j])
      log10ap -= std::log10(egs.getSurfacePotential(mgd.surface_sorption_number[j]));
    else
      log10ap -= std::log10(egs.getEquilibriumActivityCoefficient(j));
    if (log10ap < -300.0)
      EXPECT_LE(std::abs(egs.getEquilibriumMolality(j)), 1E-25);
    else
      EXPECT_LE(std::abs(egs.getEquilibriumMolality(j) - std::pow(10.0, log10ap)), 1E-15);
  }
}

/// Solve case that involves kinetic species with promoting indices and implicit solve
TEST(GeochemicalSolverTest, solve_kinetic3)
{
  PertinentGeochemicalSystem model(
      db_solver, {"H2O", "H+", "Fe+++", "HCO3-"}, {}, {}, {"Something"}, {}, {}, "O2(aq)", "e-");
  // Define rate.  The following produces
  // rate = 1.0E-2 * moles_something * 88.8537 * a_{H+} * exp(1E5 / R * (1/303.15 - 1/T))
  //      = 4.56796E-06 * moles_something
  // Something = -3H+ + Fe+++ + 2H2O + 1.5HCO3-, so Q=1E15*1E-5*1E-7=1E3 (approx) >> K, so
  // rate_Something produces a negative rate, so mole_additions > 0, so Something will increase in
  // mole number
  KineticRateUserDescription rate_Something("Something",
                                            1.0E-2,
                                            1.0,
                                            true,
                                            0.0,
                                            0.0,
                                            0.0,
                                            {"H+"},
                                            {1.0},
                                            {0.0},
                                            {0.0},
                                            1.0,
                                            0.0,
                                            1E5,
                                            1.0 / 303.15,
                                            DirectionChoiceEnum::BOTH,
                                            "H2O",
                                            0.0,
                                            -1.0,
                                            0.0);
  model.addKineticRate(rate_Something);

  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> ku;
  ku.push_back(GeochemistryUnitConverter::GeochemistryUnit::MOLES);
  GeochemicalSystem egs(mgd,
                        ac_solver,
                        is_solver,
                        swapper_kin,
                        {},
                        {},
                        "HCO3-",
                        {"H2O", "H+", "Fe+++", "HCO3-"},
                        {1.75, 1E-5, 1E-5, 4E-5},
                        cu4,
                        cm4,
                        25,
                        0,
                        1E-20,
                        {"Something"},
                        {1.0E-6},
                        ku);
  GeochemicalSolver solver(mgd.basis_species_name.size(),
                           mgd.kin_species_name.size(),
                           is_solver,
                           1.0E-15,
                           1.0E-200,
                           100,
                           1E-5,
                           1E-16,
                           1,
                           {},
                           3.0,
                           0,
                           true);

  std::stringstream ss;
  unsigned tot_iter;
  Real abs_residual;
  DenseVector<Real> mole_additions(egs.getNumInBasis() + egs.getNumKinetic());
  DenseMatrix<Real> dmole_additions(egs.getNumInBasis() + egs.getNumKinetic(),
                                    egs.getNumInBasis() + egs.getNumKinetic());
  // solve with a time-step size of 1E4
  solver.solveSystem(egs, ss, tot_iter, abs_residual, 1.0E4, mole_additions, dmole_additions);

  // check Newton has converged
  EXPECT_LE(tot_iter, (unsigned)100);
  EXPECT_LE(abs_residual, 1.0E-15);

  // check numbers are correct
  EXPECT_EQ(egs.getNumInBasis(), (unsigned)4);
  EXPECT_EQ(egs.getNumRedox(), (unsigned)0);
  EXPECT_EQ(egs.getNumSurfacePotentials(), (unsigned)0);
  EXPECT_EQ(egs.getNumInEquilibrium(), mgd.eqm_species_name.size());
  EXPECT_EQ(egs.getNumKinetic(), (unsigned)1);

  // check activity products and log10K are ordered as calculated above
  const unsigned index_something = 0;
  EXPECT_GT(egs.log10KineticActivityProduct(index_something),
            egs.getKineticLog10K(index_something));

  // check that mole additions is as calculated above
  EXPECT_NEAR(mole_additions(egs.getNumInBasis() + index_something) /
                  (1.0E4 * 4.56796204026978E-06 * egs.getKineticMoles(0)),
              1.0,
              1.0E-8);

  // check that kinetic moles have changed according to the constant rates
  for (unsigned k = 0; k < egs.getNumKinetic(); ++k)
    if (mgd.kin_species_name[k] == "Something")
      EXPECT_NEAR(egs.getKineticMoles(k),
                  1.0E-6 + mole_additions(egs.getNumInBasis() + index_something),
                  1.0E-12);
    else
      FAIL() << "Incorrect kinetic species";

  // check that the constraints are satisfied
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
    if (mgd.basis_species_name[i] == "H2O")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getSolventWaterMass(), 1.75, 1.0E-15);
      EXPECT_NEAR(egs.getSolventMassAndFreeMolalityAndMineralMoles()[0], 1.75, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "H+")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_TRUE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBasisActivity(i), 1E-5, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Fe+++")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMolesOld()[i], 1E-5 + 1E-6, 1E-15); // 1E-6 from Something
    }
    else if (mgd.basis_species_name[i] == "HCO3-")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      // do not know the bulk composition as it is dictated by charge neutrality
      EXPECT_EQ(egs.getChargeBalanceBasisIndex(), i);
    }
    else
      FAIL() << "Incorrect basis species";

  // check total charge
  EXPECT_NEAR(egs.getTotalChargeOld(), 0.0, 1E-15);
  // check total charge by summing up basis charges
  Real tot_charge = 0.0;
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
    tot_charge += egs.getBulkMolesOld()[i] * mgd.basis_species_charge[i];
  EXPECT_NEAR(tot_charge, 0.0, 1E-15);

  // check basis activity = activity_coefficient * molality
  for (unsigned i = 1; i < egs.getNumInBasis(); ++i) // don't loop over water
    if (mgd.basis_species_gas[i] || mgd.basis_species_mineral[i] || egs.getBasisActivityKnown()[i])
      continue;
    else
      EXPECT_NEAR(egs.getBasisActivity(i),
                  egs.getBasisActivityCoefficient(i) *
                      egs.getSolventMassAndFreeMolalityAndMineralMoles()[i],
                  1E-15);

  // check residuals are zero
  mole_additions.zero(); // to remove the kinetic rate contributions
  for (unsigned a = 0; a < egs.getNumInAlgebraicSystem(); ++a)
    EXPECT_LE(std::abs(egs.getResidualComponent(a, mole_additions)), 1E-15);
  // check residuals are zero by summing molalities, bulk compositions, etc
  Real nw = egs.getSolventWaterMass();
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
  {
    Real res = -egs.getBulkMolesOld()[i];
    if (i == 0)
      res += nw * GeochemistryConstants::MOLES_PER_KG_WATER;
    else if (mgd.basis_species_mineral[i])
      res += egs.getSolventMassAndFreeMolalityAndMineralMoles()[i];
    else if (mgd.basis_species_gas[i])
      res += 0.0;
    else
      res += nw * egs.getSolventMassAndFreeMolalityAndMineralMoles()[i];
    for (unsigned j = 0; j < mgd.eqm_species_name.size(); ++j)
      res += nw * mgd.eqm_stoichiometry(j, i) * egs.getEquilibriumMolality(j);
    for (unsigned k = 0; k < mgd.kin_species_name.size(); ++k)
      res += mgd.kin_stoichiometry(k, i) * egs.getKineticMoles(k);
    EXPECT_LE(std::abs(res), 1E-13);
  }

  // check equilibrium mass balance
  for (unsigned j = 0; j < mgd.eqm_species_name.size(); ++j)
  {
    if (mgd.eqm_species_mineral[j] || mgd.eqm_species_gas[j])
      continue;
    Real log10ap = 0.0;
    for (unsigned i = 0; i < mgd.basis_species_name.size(); ++i)
      log10ap += mgd.eqm_stoichiometry(j, i) * std::log10(egs.getBasisActivity(i));
    log10ap -= egs.getLog10K(j);
    if (mgd.surface_sorption_related[j])
      log10ap -= std::log10(egs.getSurfacePotential(mgd.surface_sorption_number[j]));
    else
      log10ap -= std::log10(egs.getEquilibriumActivityCoefficient(j));
    if (log10ap < -300.0)
      EXPECT_LE(std::abs(egs.getEquilibriumMolality(j)), 1E-25);
    else
      EXPECT_LE(std::abs(egs.getEquilibriumMolality(j) - std::pow(10.0, log10ap)), 1E-15);
  }
}
