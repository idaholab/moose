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
const GeochemicalDatabaseReader db_full("../database/moose_geochemdb.json");
const GeochemicalDatabaseReader
    db_ferric("../test/tests/sorption_and_surface_complexation/ferric_hydroxide_sorption.json");
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
        mgd, egs, is_solver, 1.0, 0.1, 1, 1E100, 0.1, {}, 3.0, 10);
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

  try
  {
    const EquilibriumGeochemicalSolver solver(
        mgd, egs, is_solver, 1.0, 0.1, 100, 0.0, 0.1, {}, 3.0, 10);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("EquilibriumGeochemicalSolver: max_initial_residual must be positive") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    const EquilibriumGeochemicalSolver solver(
        mgd, egs, is_solver, 1.0, 0.1, 100, 1E100, 0.1, {}, -1.0, 10);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("EquilibriumGeochemicalSolver: max_ionic_strength must not be negative") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    const EquilibriumGeochemicalSolver solver(
        mgd, egs, is_solver, 1.0, -0.1, 100, 1E100, 0.1, {}, 1.0, 10);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("EquilibriumGeochemicalSolver: rel_tol must not be negative") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    const EquilibriumGeochemicalSolver solver(
        mgd, egs, is_solver, -1.0, 0.1, 100, 1E100, 0.1, {}, 1.0, 10);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("EquilibriumGeochemicalSolver: abs_tol must not be negative") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    const EquilibriumGeochemicalSolver solver(
        mgd, egs, is_solver, 0.0, 0.0, 100, 1E100, 0.1, {}, 1.0, 10);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("EquilibriumGeochemicalSolver: either rel_tol or abs_tol must be positive") !=
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
      mgd, egs, is_solver, 1.0E-15, 1.0E-200, 100, 10.0, 0.1, {}, 3.0, 0);

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

/// Solve realistic case with minerals and gases, but no precipitation, no redox, no sorption
TEST(EquilibiumGeochemicalSolverTest, solve2)
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
  const std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm = {
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::KG_SOLVENT_WATER,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FUGACITY,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FUGACITY,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES};
  EquilibriumGeochemicalSystem egs(
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
      cm,
      25,
      0,
      1E-20);

  // build solver
  EquilibriumGeochemicalSolver solver(mgd,
                                      egs,
                                      is_solver,
                                      1.0E-15,
                                      1.0E-200,
                                      100,
                                      1E3,
                                      0.1,
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
                                      10);

  // solve
  std::stringstream ss;
  unsigned tot_iter;
  Real abs_residual;
  solver.solveSystem(ss, tot_iter, abs_residual);

  // check converged
  EXPECT_LE(tot_iter, 100);
  EXPECT_LE(abs_residual, 1.0E-15);

  // check number in basis, number in redox disequilibrium and number of surface potentials
  EXPECT_EQ(egs.getNumInBasis(), 11);
  EXPECT_EQ(egs.getNumRedox(), 0);
  EXPECT_EQ(egs.getNumSurfacePotentials(), 0);
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
      EXPECT_NEAR(egs.getBulkMoles()[i], 0.4850, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "SO4--")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMoles()[i], 0.02924, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Mg++")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMoles()[i], 0.05501, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Ca++")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMoles()[i], 0.01063, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "K+")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMoles()[i], 0.010576055, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "HCO3-")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMoles()[i], 0.002412, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "SiO2(aq)")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMoles()[i], 0.00010349, 1.0E-15);
    }
    else
      FAIL() << "Incorrect basis species";

  // check total charge
  EXPECT_NEAR(egs.getTotalCharge(), 0.0, 1E-15);
  // check total charge by summing up basis charges
  Real tot_charge = 0.0;
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
    tot_charge += egs.getBulkMoles()[i] * mgd.basis_species_charge[i];
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
    EXPECT_LE(std::abs(egs.getResidualComponent(a)), 1E-15);
  // check residuals are zero by summing molalities, bulk compositions, etc
  Real nw = egs.getSolventWaterMass();
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
  {
    Real res = -egs.getBulkMoles()[i];
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
    EXPECT_LE(std::abs(res), 1E-15);
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
TEST(EquilibiumGeochemicalSolverTest, solve3)
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
  const std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm = {
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::KG_SOLVENT_WATER,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES};
  EquilibriumGeochemicalSystem egs(
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
      cm,
      25,
      0,
      1E-20);

  // build solver
  EquilibriumGeochemicalSolver solver(mgd,
                                      egs,
                                      is_solver,
                                      1.0E-15,
                                      1.0E-200,
                                      100,
                                      1E3,
                                      0.1,
                                      {"Dolomite-ord", "Dolomite-dis"},
                                      3.0,
                                      10);

  // solve
  std::stringstream ss;
  unsigned tot_iter;
  Real abs_residual;
  solver.solveSystem(ss, tot_iter, abs_residual);

  // check converged
  EXPECT_LE(tot_iter, 100);
  EXPECT_LE(abs_residual, 1.0E-15);

  // check number in basis, number in redox disequilibrium and number of surface potentials
  EXPECT_EQ(egs.getNumInBasis(), 11);
  EXPECT_EQ(egs.getNumRedox(), 0);
  EXPECT_EQ(egs.getNumSurfacePotentials(), 0);
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
      EXPECT_NEAR(egs.getBulkMoles()[i], 0.4850, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "SO4--")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMoles()[i], 0.02924, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Mg++")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      bulk_mg = egs.getBulkMoles()[i];
    }
    else if (mgd.basis_species_name[i] == "Ca++")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      bulk_ca = egs.getBulkMoles()[i];
    }
    else if (mgd.basis_species_name[i] == "K+")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMoles()[i], 0.010576055, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Quartz")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_TRUE(mgd.basis_species_mineral[i]);
      EXPECT_TRUE(egs.getBasisActivityKnown()[i]);
      EXPECT_GE(egs.getSolventMassAndFreeMolalityAndMineralMoles()[i], 0.0);
      // Quartz = SiO2(aq)
      EXPECT_NEAR(egs.getBulkMoles()[i], 0.00010349, 1.0);
    }
    else if (mgd.basis_species_name[i] == "Dolomite")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_TRUE(mgd.basis_species_mineral[i]);
      EXPECT_TRUE(egs.getBasisActivityKnown()[i]);
      EXPECT_GE(egs.getSolventMassAndFreeMolalityAndMineralMoles()[i], 0.0);
      bulk_dolo = egs.getBulkMoles()[i];
    }
    else if (mgd.basis_species_name[i] == "HCO3-")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMoles()[i], 0.002412, 1.0E-13);
    }
    else
      FAIL() << "Incorrect basis species";
  // dolomite = 2MgCO3 - Mg + Ca
  EXPECT_NEAR(bulk_dolo + bulk_ca, 0.01063, 1E-13);
  EXPECT_NEAR(-bulk_dolo + bulk_mg, 0.05501, 1E-13);
  EXPECT_NEAR(2.0 * bulk_dolo, 0.196E-3, 1E-13);

  // check total charge
  EXPECT_NEAR(egs.getTotalCharge(), 0.0, 1E-15);
  // check total charge by summing up basis charges
  Real tot_charge = 0.0;
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
    tot_charge += egs.getBulkMoles()[i] * mgd.basis_species_charge[i];
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
    EXPECT_LE(std::abs(egs.getResidualComponent(a)), 1E-15);
  // check residuals are zero by summing molalities, bulk compositions, etc
  Real nw = egs.getSolventWaterMass();
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
  {
    Real res = -egs.getBulkMoles()[i];
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
    if (mgd.eqm_species_name[j] == "Dolomite-ord" || mgd.eqm_species_name[j] == "Dolomite-dis")
      continue;
    Real log10ap = 0.0;
    for (unsigned i = 0; i < mgd.basis_species_name.size(); ++i)
      log10ap += mgd.eqm_stoichiometry(j, i) * std::log10(egs.getBasisActivity(i));
    EXPECT_LE(log10ap, egs.getLog10K(j));
  }
}

/// Solve realistic case with redox disequilibrium, no minerals, no sorption
TEST(EquilibiumGeochemicalSolverTest, solve4)
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
  const std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm = {
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::KG_SOLVENT_WATER,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLALITY,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES};
  EquilibriumGeochemicalSystem egs(mgd,
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
                                   cm,
                                   25,
                                   0,
                                   1E-20);

  // build solver
  EquilibriumGeochemicalSolver solver(
      mgd, egs, is_solver, 1.0E-15, 1.0E-200, 100, 1E-2, 0.1, {}, 3.0, 10);

  // solve
  std::stringstream ss;
  unsigned tot_iter;
  Real abs_residual;
  solver.solveSystem(ss, tot_iter, abs_residual);

  // check converged
  EXPECT_LE(tot_iter, 100);
  EXPECT_LE(abs_residual, 1.0E-15);

  // check number in basis, number in redox disequilibrium and number of surface potentials
  EXPECT_EQ(egs.getNumInBasis(), 14);
  EXPECT_EQ(egs.getNumRedox(), 1);
  EXPECT_EQ(egs.getNumSurfacePotentials(), 0);
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
      EXPECT_NEAR(egs.getBulkMoles()[i], 0.0295E-3, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Ca++")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMoles()[i], 0.005938E-3, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Mg++")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMoles()[i], 0.01448E-3, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Na+")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMoles()[i], 0.0018704E-3, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "K+")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMoles()[i], 0.005115E-3, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Fe++")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMoles()[i], 0.012534E-3, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Fe+++")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMoles()[i], 0.0005372E-3, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Mn++")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMoles()[i], 0.005042E-3, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Zn++")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMoles()[i], 0.001897E-3, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "SO4--")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMoles()[i], 0.01562E-3, 1.0E-15);
    }
    else
      FAIL() << "Incorrect basis species";

  // check total charge
  EXPECT_NEAR(egs.getTotalCharge(), 0.0, 1E-15);
  // check total charge by summing up basis charges
  Real tot_charge = 0.0;
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
    tot_charge += egs.getBulkMoles()[i] * mgd.basis_species_charge[i];
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
    EXPECT_LE(std::abs(egs.getResidualComponent(a)), 1E-15);
  // check residuals are zero by summing molalities, bulk compositions, etc
  Real nw = egs.getSolventWaterMass();
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
  {
    Real res = -egs.getBulkMoles()[i];
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

/// Solve realistic case with sorption and minerals, no redox
TEST(EquilibiumGeochemicalSolverTest, solve5)
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
  const std::vector<EquilibriumGeochemicalSystem::ConstraintMeaningEnum> cm = {
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::KG_SOLVENT_WATER,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::ACTIVITY,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::FREE_MOLES_MINERAL_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES,
      EquilibriumGeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES};
  EquilibriumGeochemicalSystem egs(
      mgd,
      ac_solver,
      is_solver,
      swapper,
      {"Fe+++"},
      {"Fe(OH)3(ppd)"},
      "Cl-",
      {"H2O", "H+", "Na+", "Cl-", "Hg++", "Pb++", "SO4--", "Fe(OH)3(ppd)", ">(s)FeOH", ">(w)FeOH"},
      {1.0, 1.0E-4, 10E-3, 10E-3, 0.1E-3, 0.1E-3, 0.2E-3, 9.3573E-3, 4.6786E-5, 1.87145E-3},
      cm,
      25,
      0,
      1E-20);

  // build solver
  EquilibriumGeochemicalSolver solver(
      mgd, egs, is_solver, 1.0E-15, 1.0E-200, 100, 1.0, 0.1, {}, 3.0, 10);

  // solve
  std::stringstream ss;
  unsigned tot_iter;
  Real abs_residual;
  solver.solveSystem(ss, tot_iter, abs_residual);

  // check converged
  EXPECT_LE(tot_iter, 100);
  EXPECT_LE(abs_residual, 1.0E-15);

  // check number in basis, number in redox disequilibrium and number of surface potentials
  EXPECT_EQ(egs.getNumInBasis(), 10);
  EXPECT_EQ(egs.getNumRedox(), 0);
  EXPECT_EQ(egs.getNumSurfacePotentials(), 1);
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
      EXPECT_NEAR(egs.getBulkMoles()[i], 10E-3, 1.0E-15);
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
      EXPECT_NEAR(egs.getBulkMoles()[i], 0.1E-3, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "Pb++")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMoles()[i], 0.1E-3, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == "SO4--")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMoles()[i], 0.2E-3, 1.0E-15);
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
      EXPECT_NEAR(egs.getBulkMoles()[i], 4.6786E-5, 1.0E-15);
    }
    else if (mgd.basis_species_name[i] == ">(w)FeOH")
    {
      EXPECT_FALSE(mgd.basis_species_gas[i]);
      EXPECT_FALSE(mgd.basis_species_mineral[i]);
      EXPECT_FALSE(egs.getBasisActivityKnown()[i]);
      EXPECT_NEAR(egs.getBulkMoles()[i], 1.87145E-3, 1.0E-15);
    }
    else
      FAIL() << "Incorrect basis species";

  // check total charge
  EXPECT_NEAR(egs.getTotalCharge(), 0.0, 1E-15);
  // check total charge by summing up basis charges
  Real tot_charge = 0.0;
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
    tot_charge += egs.getBulkMoles()[i] * mgd.basis_species_charge[i];
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
    EXPECT_LE(std::abs(egs.getResidualComponent(a)), 1E-15);
  // check residuals are zero by summing molalities, bulk compositions, etc
  Real nw = egs.getSolventWaterMass();
  for (unsigned i = 0; i < egs.getNumInBasis(); ++i)
  {
    Real res = -egs.getBulkMoles()[i];
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
