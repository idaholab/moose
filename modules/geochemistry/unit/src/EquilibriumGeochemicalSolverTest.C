//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "EquilibriumGeochemicalSolver.h"

const GeochemicalDatabaseReader db_solver("database/moose_testdb.json");
// Following model only has OH- as an equilibrium species
const PertinentGeochemicalSystem
    model_simplest(db_solver, {"H2O", "H+"}, {}, {}, {}, {}, {}, "O2(aq)", "e-");
GeochemistrySpeciesSwapper swapper2(2, 1E-6);
const std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm2 = {
    EquilibriumGeochemicalSystem::ConstraintMeaningEnum::KG_SOLVENT_WATER,
    EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES};
GeochemistryIonicStrength is_solver(3.0, 3.0, false);
GeochemistryActivityCoefficientsDebyeHuckel ac_solver(is_solver);

/// Check exception
TEST(EquilibiumGeochemicalSolverTest, exception)
{
  ModelGeochemicalDatabase mgd = model_simplest.modelGeochemicalDatabase();
  EquilibriumGeochemicalSystem egs(mgd,
                                   ac_solver,
                                   is_solver,
                                   swapper2,
                                   {},
                                   {},
                                   "H+",
                                   {"H2O", "H+"},
                                   {1.75, 3.0},
                                   cm2,
                                   25,
                                   0,
                                   1E-20);
  try
  {
    const EquilibriumGeochemicalSolver solver(
        mgd, egs, is_solver, 1.0, 0.1, 1, 1E100, 1.0, {""}, 3.0, 10);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find(
            "EquilibriumGeochemicalSolver: ramp_max_ionic_strength must be less than max_iter") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Solve super-simple case
TEST(EquilibiumGeochemicalSolverTest, solve1)
{
  ModelGeochemicalDatabase mgd = model_simplest.modelGeochemicalDatabase();
  EquilibriumGeochemicalSystem egs(mgd,
                                   ac_solver,
                                   is_solver,
                                   swapper2,
                                   {},
                                   {},
                                   "H+",
                                   {"H2O", "H+"},
                                   {1.75, 1},
                                   cm2,
                                   25,
                                   0,
                                   1E-20);
  EquilibriumGeochemicalSolver solver(
      mgd, egs, is_solver, 1.0E-15, 1.0E-200, 100, 10.0, 1.0, {""}, 3.0, 0);
  std::stringstream ss;
  unsigned tot_iter;
  Real abs_residual;

  solver.solveSystem(ss, tot_iter, abs_residual);

  // check Newton has converged
  EXPECT_LE(tot_iter, 100);
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
  EXPECT_NEAR(egs.getBulkMoles()[0],
              1.75 * (GeochemistryConstants::MOLES_PER_KG_WATER + egs.getEquilibriumMolality(0)),
              1.0E-9);
  EXPECT_NEAR(egs.getBulkMoles()[1], 0.0, 1.0E-9);
}
