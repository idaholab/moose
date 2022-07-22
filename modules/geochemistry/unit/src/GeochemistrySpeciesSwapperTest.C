//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "GeochemistrySpeciesSwapper.h"

const Real eps =
    1E-12; // accounts for precision loss when substituting reactions and inverting the swap matrix

/// Check bulk_composition exception
TEST(GeochemistrySpeciesSwapperTest, bulkCompositionException)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);

  // The following system has secondary species: CO2(aq), CO3--, OH-,
  // (O-phth)--, CH4(aq), Fe+++,
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
  GeochemistrySpeciesSwapper swapper(mgd.basis_species_index.size(), 1E-6);

  try
  {
    DenseVector<Real> bulk_composition(8);
    swapper.performSwap(mgd, bulk_composition, "H+", "(O-phth)--");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("GeochemistrySpeciesSwapper: bulk_composition has size 8 which differs "
                         "from the basis size") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check all sorts of illegal swaps throw mooseError or mooseException
TEST(GeochemistrySpeciesSwapperTest, swapExceptions)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);

  // The following system has secondary species: CO2(aq), CO3--, OH-,
  // (O-phth)--, CH4(aq), Fe+++,
  // >(s)FeO-
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
  GeochemistrySpeciesSwapper swapper(mgd.basis_species_index.size(), 1E-6);

  try
  {
    swapper.checkSwap(mgd, "CO2(aq)", "CO2(aq)");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("CO2(aq) is not in the basis, so cannot be removed "
                         "from the basis") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    swapper.checkSwap(mgd, "CH4(g)fake", "CO2(aq)");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("CH4(g)fake is not in the basis, so cannot be removed "
                         "from the basis") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    swapper.checkSwap(mgd, "H+", ">(s)FeOH");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find(">(s)FeOH is not an equilibrium species, so cannot be "
                         "removed from the "
                         "equilibrium species list") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    swapper.checkSwap(mgd, "H2O", "CO2(aq)");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Cannot remove H2O from the basis") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    swapper.checkSwap(mgd, 123, 0);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("123 exceeds the number of basis species in the problem") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    swapper.checkSwap(mgd, 0, 0);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Cannot remove H2O from the basis") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    swapper.checkSwap(mgd, 1, 123);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("123 exceeds the number of equilibrium species in the problem") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    swapper.performSwap(mgd, ">(s)FeOH", "CO2(aq)");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Matrix is not invertible, which signals an invalid basis swap") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    swapper = GeochemistrySpeciesSwapper(8, 1E-6);
    swapper.checkSwap(mgd, "Fe++", "Fe+++");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("GeochemistrySpeciesSwapper constructed with incorrect basis_species size") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    swapper.checkSwap(mgd, ">(s)FeOH", ">(s)FeO-");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("Equilibrium species >(s)FeO- is involved in surface sorption so cannot be "
                 "swapped into the basis.  If this is truly necessary, code enhancements will need "
                 "to be made including: recording whether basis species are involved in surface "
                 "sorption, including them in the surface-potential calculations, and carefully "
                 "swapping surface-potential-modified equilibrium constants") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check a simple swap that does not involve redox or other complicated things
TEST(GeochemistrySpeciesSwapperTest, swap1)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);

  // eqm species are: CO2(aq), CO3--, CaCO3, CaOH+, OH-, Calcite
  PertinentGeochemicalSystem model(
      database, {"H2O", "Ca++", "HCO3-", "H+"}, {"Calcite"}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();
  GeochemistrySpeciesSwapper swapper(mgd.basis_species_index.size(), 1E-6);
  DenseVector<Real> bulk_composition(4);
  bulk_composition(0) = 0.5;
  bulk_composition(1) = 1.0;
  bulk_composition(2) = 2.5;
  bulk_composition(3) = 3.0;

  ASSERT_EQ(mgd.basis_species_index.size(), (std::size_t)4);
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_name[sp.second], sp.first);

  ASSERT_EQ(mgd.eqm_species_index.size(), (std::size_t)6);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_name[sp.second], sp.first);

  ASSERT_EQ(mgd.swap_to_original_basis.n(), (unsigned)0);
  ASSERT_EQ(mgd.have_swapped_out_of_basis.size(), (std::size_t)0);
  ASSERT_EQ(mgd.have_swapped_into_basis.size(), (std::size_t)0);

  for (const auto & species : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_mineral[species.second], false);
  for (const auto & species : mgd.eqm_species_index)
    if (species.first == "Calcite")
      ASSERT_EQ(mgd.eqm_species_mineral[species.second], true);
    else
      ASSERT_EQ(mgd.eqm_species_mineral[species.second], false);

  for (const auto & species : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_gas[species.second], false);
  for (const auto & species : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_gas[species.second], false);

  for (const auto & species : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_transported[species.second], true);
  for (const auto & species : mgd.eqm_species_index)
    if (species.first == "Calcite")
      ASSERT_EQ(mgd.eqm_species_transported[species.second], false);
    else
      ASSERT_EQ(mgd.eqm_species_transported[species.second], true);

  ASSERT_EQ(mgd.eqm_species_index.size(), (std::size_t)6);
  for (const auto & sp : {"CO2(aq)", "CO3--", "CaCO3", "CaOH+", "OH-", "Calcite"})
    ASSERT_EQ(mgd.eqm_species_index.count(sp), (std::size_t)1);

  std::map<std::string, Real> charge_gold;
  charge_gold["H2O"] = 0.0;
  charge_gold["H+"] = 1.0;
  charge_gold["HCO3-"] = -1.0;
  charge_gold["Ca++"] = 2.0;
  charge_gold["CO2(aq)"] = 0.0;
  charge_gold["CO3--"] = -2.0;
  charge_gold["CaCO3"] = 0.0;
  charge_gold["CaOH+"] = 1.0;
  charge_gold["OH-"] = -1.0;
  charge_gold["Calcite"] = 0.0;
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_charge[sp.second], charge_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_charge[sp.second], charge_gold[sp.first]);

  std::map<std::string, Real> radius_gold;
  radius_gold["H2O"] = 0.0;
  radius_gold["H+"] = 9.0;
  radius_gold["HCO3-"] = 4.5;
  radius_gold["Ca++"] = 6.0;
  radius_gold["CO2(aq)"] = 4.0;
  radius_gold["CO3--"] = 4.5;
  radius_gold["CaCO3"] = 4.0;
  radius_gold["CaOH+"] = 4.0;
  radius_gold["OH-"] = 3.5;
  radius_gold["Calcite"] = 0.0;
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_radius[sp.second], radius_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_radius[sp.second], radius_gold[sp.first]);

  std::map<std::string, Real> molecular_weight_gold;
  molecular_weight_gold["H2O"] = 18.0152;
  molecular_weight_gold["H+"] = 1.0079;
  molecular_weight_gold["HCO3-"] = 61.0171;
  molecular_weight_gold["Ca++"] = 40.0800;
  molecular_weight_gold["CO2(aq)"] = 44.0098;
  molecular_weight_gold["CO3--"] = 60.0092;
  molecular_weight_gold["CaCO3"] = 100.0892;
  molecular_weight_gold["CaOH+"] = 57.0873;
  molecular_weight_gold["OH-"] = 17.0073;
  molecular_weight_gold["Calcite"] = 100.0892;
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_molecular_weight[sp.second], molecular_weight_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_molecular_weight[sp.second], molecular_weight_gold[sp.first]);

  std::map<std::string, Real> molecular_volume_gold;
  molecular_volume_gold["H2O"] = 0.0;
  molecular_volume_gold["H+"] = 0.0;
  molecular_volume_gold["HCO3-"] = 0.0;
  molecular_volume_gold["O2(aq)"] = 0.0;
  molecular_volume_gold["Ca++"] = 0.0;
  molecular_volume_gold["CO2(aq)"] = 0.0;
  molecular_volume_gold["CO3--"] = 0.0;
  molecular_volume_gold["CaCO3"] = 0.0;
  molecular_volume_gold["CaOH+"] = 0.0;
  molecular_volume_gold["OH-"] = 0.0;
  molecular_volume_gold["Calcite"] = 36.9340;
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_molecular_volume[sp.second], molecular_volume_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_molecular_volume[sp.second], molecular_volume_gold[sp.first]);

  std::map<std::string, DenseMatrix<Real>> stoi_gold;
  for (const auto & sp : {"CO2(aq)", "CO3--", "CaCO3", "CaOH+", "OH-", "Calcite"})
    stoi_gold[sp] = DenseMatrix<Real>(1, 4);
  // remember the order of primaries: {"H2O", "Ca++", "HCO3-", "H+"}
  stoi_gold["CO2(aq)"](0, 0) = -1;
  stoi_gold["CO2(aq)"](0, 3) = 1;
  stoi_gold["CO2(aq)"](0, 2) = 1;
  stoi_gold["CO3--"](0, 2) = 1;
  stoi_gold["CO3--"](0, 3) = -1;
  stoi_gold["CaCO3"](0, 1) = 1;
  stoi_gold["CaCO3"](0, 2) = 1;
  stoi_gold["CaCO3"](0, 3) = -1;
  stoi_gold["CaOH+"](0, 1) = 1;
  stoi_gold["CaOH+"](0, 0) = 1;
  stoi_gold["CaOH+"](0, 3) = -1;
  stoi_gold["OH-"](0, 0) = 1;
  stoi_gold["OH-"](0, 3) = -1;
  stoi_gold["Calcite"](0, 1) = 1;
  stoi_gold["Calcite"](0, 2) = 1;
  stoi_gold["Calcite"](0, 3) = -1;

  for (const auto & sp : {"CO2(aq)", "CO3--", "CaCO3", "CaOH+", "OH-", "Calcite"})
  {
    const unsigned row = mgd.eqm_species_index[sp];
    ASSERT_EQ(mgd.eqm_stoichiometry.sub_matrix(row, 1, 0, 4), stoi_gold[sp]);
  }

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
  ASSERT_EQ(mgd.eqm_log10K(mgd.eqm_species_index["Calcite"], 0), 2.0683);

  const unsigned ca_posn = mgd.basis_species_index["Ca++"];
  const unsigned calcite_posn = mgd.eqm_species_index["Calcite"];

  swapper.performSwap(mgd, bulk_composition, "Ca++", "Calcite");

  // check names are swapped correctly
  ASSERT_EQ(mgd.basis_species_index.size(), (std::size_t)4);
  for (const auto & sp : {"Calcite", "H2O", "HCO3-", "H+"})
    ASSERT_EQ(mgd.basis_species_index.count(sp), (std::size_t)1);
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_name[sp.second], sp.first);
  ASSERT_EQ(mgd.basis_species_index["Calcite"], ca_posn);

  // check names are swapped correctly
  ASSERT_EQ(mgd.eqm_species_index.size(), (std::size_t)6);
  for (const auto & sp : {"CO2(aq)", "CO3--", "CaCO3", "CaOH+", "OH-", "Ca++"})
    ASSERT_EQ(mgd.eqm_species_index.count(sp), (std::size_t)1);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_name[sp.second], sp.first);
  ASSERT_EQ(mgd.eqm_species_index["Ca++"], calcite_posn);

  // check the swap is recorded correctly
  ASSERT_EQ(mgd.have_swapped_out_of_basis.size(), (std::size_t)1);
  ASSERT_EQ(mgd.have_swapped_into_basis.size(), (std::size_t)1);
  ASSERT_EQ(mgd.have_swapped_out_of_basis[0], ca_posn);
  ASSERT_EQ(mgd.have_swapped_into_basis[0], calcite_posn);
  ASSERT_EQ(mgd.swap_to_original_basis.n(), (unsigned)4);
  DenseMatrix<Real> gold_swap_matrix(4, 4);
  for (unsigned i = 0; i < 4; ++i)
    gold_swap_matrix(i, i) = 1.0;
  gold_swap_matrix(ca_posn, 0) = 0.0;  // H2O
  gold_swap_matrix(ca_posn, 1) = 1.0;  // Ca++
  gold_swap_matrix(ca_posn, 2) = 1.0;  // HCO3-
  gold_swap_matrix(ca_posn, 3) = -1.0; // H+
  for (unsigned i = 0; i < 4; ++i)
    for (unsigned j = 0; j < 4; ++j)
      EXPECT_EQ(mgd.swap_to_original_basis(i, j), gold_swap_matrix(i, j));

  // check charges swapped correctly
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_charge[sp.second], charge_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_charge[sp.second], charge_gold[sp.first]);

  // check radii swapped correctly
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_radius[sp.second], radius_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_radius[sp.second], radius_gold[sp.first]);

  // check molecular weights swapped correctly
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_molecular_weight[sp.second], molecular_weight_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_molecular_weight[sp.second], molecular_weight_gold[sp.first]);

  // check molecular volumes swapped correctly
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_molecular_volume[sp.second], molecular_volume_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_molecular_volume[sp.second], molecular_volume_gold[sp.first]);

  // check isMineral information has swapped correctly
  for (const auto & species : mgd.basis_species_index)
    if (species.first == "Calcite")
      ASSERT_EQ(mgd.basis_species_mineral[species.second], true);
    else
      ASSERT_EQ(mgd.basis_species_mineral[species.second], false);
  for (const auto & species : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_mineral[species.second], false);

  // check isGas information has swapped correctly
  for (const auto & species : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_gas[species.second], false);
  for (const auto & species : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_gas[species.second], false);

  // check isTransported information has swapped correctly
  for (const auto & species : mgd.basis_species_index)
    if (species.first == "Calcite")
      ASSERT_EQ(mgd.basis_species_transported[species.second], false);
    else
      ASSERT_EQ(mgd.basis_species_transported[species.second], true);
  for (const auto & species : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_transported[species.second], true);

  // check stoichiometry
  stoi_gold = std::map<std::string, DenseMatrix<Real>>();
  for (const auto & sp : {"CO2(aq)", "CO3--", "CaCO3", "CaOH+", "OH-", "Ca++"})
    stoi_gold[sp] = DenseMatrix<Real>(1, 4);
  // remember the order of primaries: {"H2O", "Calcite", "HCO3-", "H+"}
  stoi_gold["CO2(aq)"](0, 0) = -1;
  stoi_gold["CO2(aq)"](0, 3) = 1;
  stoi_gold["CO2(aq)"](0, 2) = 1;
  stoi_gold["CO3--"](0, 2) = 1;
  stoi_gold["CO3--"](0, 3) = -1;
  stoi_gold["CaCO3"](0, 1) = 1;
  stoi_gold["CaOH+"](0, 1) = 1;
  stoi_gold["CaOH+"](0, 0) = 1;
  stoi_gold["CaOH+"](0, 2) = -1;
  stoi_gold["OH-"](0, 0) = 1;
  stoi_gold["OH-"](0, 3) = -1;
  stoi_gold["Ca++"](0, 1) = 1;
  stoi_gold["Ca++"](0, 2) = -1;
  stoi_gold["Ca++"](0, 3) = 1;

  for (const auto & sp : {"CO2(aq)", "CO3--", "CaCO3", "CaOH+", "OH-", "Ca++"})
  {
    const unsigned row = mgd.eqm_species_index[sp];
    for (unsigned i = 0; i < 4; ++i)
      ASSERT_NEAR(mgd.eqm_stoichiometry(row, i), stoi_gold[sp](0, i), eps);
  }

  // check eqm constants
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO2(aq)"], 0), -6.5570, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO2(aq)"], 1), -6.3660, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO2(aq)"], 2), -6.3325, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO2(aq)"], 3), -6.4330, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO2(aq)"], 4), -6.7420, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO2(aq)"], 5), -7.1880, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO2(aq)"], 6), -7.7630, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO2(aq)"], 7), -8.4650, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO3--"], 0), 10.6169 - 0 * 2.0683, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO3--"], 1), 10.3439 - 0 * 1.7130, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO3--"], 2), 10.2092 - 0 * 1.2133, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO3--"], 3), 10.2793 - 0 * 0.6871, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO3--"], 4), 10.5131 - 0 * 0.0762, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO3--"], 5), 10.8637 - 0 * (-0.5349), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO3--"], 6), 11.2860 - 0 * (-1.2301), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO3--"], 7), 11.6319 - 0 * (-2.2107), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CaCO3"], 0), 7.5520 - 1 * 2.0683, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CaCO3"], 1), 7.1280 - 1 * 1.7130, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CaCO3"], 2), 6.7340 - 1 * 1.2133, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CaCO3"], 3), 6.4350 - 1 * 0.6871, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CaCO3"], 4), 6.1810 - 1 * 0.0762, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CaCO3"], 5), 5.9320 - 1 * (-0.5349), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CaCO3"], 6), 5.5640 - 1 * (-1.2301), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CaCO3"], 7), 4.7890 - 1 * (-2.2107), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CaOH+"], 0), 13.7095 - 1 * 2.0683, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CaOH+"], 1), 12.6887 - 1 * 1.7130, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CaOH+"], 2), 11.5069 - 1 * 1.2133, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CaOH+"], 3), 10.4366 - 1 * 0.6871, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CaOH+"], 4), 9.3958 - 1 * 0.0762, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CaOH+"], 5), 8.5583 - 1 * (-0.5349), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CaOH+"], 6), 7.8155 - 1 * (-1.2301), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CaOH+"], 7), 7.0306 - 1 * (-2.2107), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["OH-"], 0), 14.9325 - 0 * 2.0683, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["OH-"], 1), 13.9868 - 0 * 1.7130, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["OH-"], 2), 13.0199 - 0 * 1.2133, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["OH-"], 3), 12.2403 - 0 * 0.6871, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["OH-"], 4), 11.5940 - 0 * 0.0762, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["OH-"], 5), 11.2191 - 0 * (-0.5349), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["OH-"], 6), 11.0880 - 0 * (-1.2301), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["OH-"], 7), 1001.2844 - 0 * (-2.2107), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["Ca++"], 0), 0 - 1 * 2.0683, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["Ca++"], 1), 0 - 1 * 1.7130, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["Ca++"], 2), 0 - 1 * 1.2133, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["Ca++"], 3), 0 - 1 * 0.6871, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["Ca++"], 4), 0 - 1 * 0.0762, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["Ca++"], 5), 0 - 1 * (-0.5349), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["Ca++"], 6), 0 - 1 * (-1.2301), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["Ca++"], 7), 0 - 1 * (-2.2107), eps);

  // check bulk composition
  ASSERT_NEAR(bulk_composition(0), 0.5, eps);
  ASSERT_NEAR(bulk_composition(1), 1.0, eps);
  ASSERT_NEAR(bulk_composition(2), 1.5, eps);
  ASSERT_NEAR(bulk_composition(3), 4.0, eps);

  // swap back, and check that swap_to_original_basis is the identity
  swapper.performSwap(mgd, bulk_composition, "Calcite", "Ca++");
  ASSERT_EQ(mgd.swap_to_original_basis.n(), (unsigned)4);
  for (unsigned i = 0; i < 4; ++i)
    for (unsigned j = 0; j < 4; ++j)
      if (i == j)
        EXPECT_NEAR(mgd.swap_to_original_basis(i, j), 1.0, 1.0E-6);
      else
        EXPECT_NEAR(mgd.swap_to_original_basis(i, j), 0.0, 1.0E-6);
}

/// Check a complicated swap involving redox and complicated stoichiometric coefficients
TEST(GeochemistrySpeciesSwapperTest, swap2)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);

  // eqm species are: OH-, CO2(aq), CO3--, StoiCheckRedox, StoiCheckGas
  PertinentGeochemicalSystem model(database,
                                   {"H2O", "StoiCheckBasis", "H+", "HCO3-"},
                                   {},
                                   {"StoiCheckGas"},
                                   {},
                                   {},
                                   {},
                                   "O2(aq)",
                                   "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();
  GeochemistrySpeciesSwapper swapper(mgd.basis_species_index.size(), 1E-6);

  ASSERT_EQ(mgd.basis_species_index.size(), (std::size_t)4);
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_name[sp.second], sp.first);

  ASSERT_EQ(mgd.eqm_species_index.size(), (std::size_t)5);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_name[sp.second], sp.first);

  ASSERT_EQ(mgd.swap_to_original_basis.n(), (unsigned)0);
  ASSERT_EQ(mgd.have_swapped_out_of_basis.size(), (std::size_t)0);
  ASSERT_EQ(mgd.have_swapped_into_basis.size(), (std::size_t)0);

  for (const auto & species : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_mineral[species.second], false);
  for (const auto & species : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_mineral[species.second], false);

  for (const auto & species : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_transported[species.second], true);
  for (const auto & species : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_transported[species.second], true);

  ASSERT_EQ(mgd.eqm_species_index.size(), (std::size_t)5);
  for (const auto & sp : {"OH-", "CO2(aq)", "CO3--", "StoiCheckRedox", "StoiCheckGas"})
    ASSERT_EQ(mgd.eqm_species_index.count(sp), (std::size_t)1);

  for (const auto & species : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_gas[species.second], false);
  for (const auto & species : mgd.eqm_species_index)
    if (species.first == "StoiCheckGas")
      ASSERT_EQ(mgd.eqm_species_gas[species.second], true);
    else
      ASSERT_EQ(mgd.eqm_species_gas[species.second], false);

  std::map<std::string, Real> charge_gold;
  charge_gold["H2O"] = 0.0;
  charge_gold["StoiCheckBasis"] = 2.5;
  charge_gold["H+"] = 1.0;
  charge_gold["HCO3-"] = -1.0;
  charge_gold["CO2(aq)"] = 0.0;
  charge_gold["CO3--"] = -2.0;
  charge_gold["OH-"] = -1.0;
  charge_gold["StoiCheckRedox"] = 3.3;
  charge_gold["StoiCheckGas"] = 0.0;
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_charge[sp.second], charge_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_charge[sp.second], charge_gold[sp.first]);

  std::map<std::string, Real> radius_gold;
  radius_gold["H2O"] = 0.0;
  radius_gold["StoiCheckBasis"] = 6.54;
  radius_gold["H+"] = 9.0;
  radius_gold["HCO3-"] = 4.5;
  radius_gold["CO2(aq)"] = 4.0;
  radius_gold["CO3--"] = 4.5;
  radius_gold["OH-"] = 3.5;
  radius_gold["StoiCheckRedox"] = 9.9;
  radius_gold["StoiCheckGas"] = 0.0;
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_radius[sp.second], radius_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_radius[sp.second], radius_gold[sp.first]);

  std::map<std::string, Real> molecular_weight_gold;
  molecular_weight_gold["H2O"] = 18.0152;
  molecular_weight_gold["StoiCheckBasis"] = 55.8470;
  molecular_weight_gold["H+"] = 1.0079;
  molecular_weight_gold["HCO3-"] = 61.0171;
  molecular_weight_gold["CO2(aq)"] = 44.0098;
  molecular_weight_gold["CO3--"] = 60.0092;
  molecular_weight_gold["OH-"] = 17.0073;
  molecular_weight_gold["StoiCheckRedox"] = 55.8470;
  molecular_weight_gold["StoiCheckGas"] = 28.0134;
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_molecular_weight[sp.second], molecular_weight_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_molecular_weight[sp.second], molecular_weight_gold[sp.first]);

  std::map<std::string, Real> molecular_volume_gold;
  molecular_volume_gold["H2O"] = 0.0;
  molecular_volume_gold["StoiCheckBasis"] = 0.0;
  molecular_volume_gold["H+"] = 0.0;
  molecular_volume_gold["HCO3-"] = 0.0;
  molecular_volume_gold["CO2(aq)"] = 0.0;
  molecular_volume_gold["CO3--"] = 0.0;
  molecular_volume_gold["OH-"] = 0.0;
  molecular_volume_gold["StoiCheckRedox"] = 0.0;
  molecular_volume_gold["StoiCheckGas"] = 0.0;
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_molecular_volume[sp.second], molecular_volume_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_molecular_volume[sp.second], molecular_volume_gold[sp.first]);

  std::map<std::string, DenseMatrix<Real>> stoi_gold;
  for (const auto & sp : {"OH-", "CO2(aq)", "CO3--", "StoiCheckRedox", "StoiCheckGas"})
    stoi_gold[sp] = DenseMatrix<Real>(1, 4);
  // remember the order of primaries: {"H2O", "StoiCheckBasis", "H+", "HCO3-"}
  stoi_gold["CO2(aq)"](0, 0) = -1;
  stoi_gold["CO2(aq)"](0, 2) = 1;
  stoi_gold["CO2(aq)"](0, 3) = 1;
  stoi_gold["CO3--"](0, 2) = -1;
  stoi_gold["CO3--"](0, 3) = 1;
  stoi_gold["OH-"](0, 0) = 1;
  stoi_gold["OH-"](0, 2) = -1;
  stoi_gold["StoiCheckRedox"](0, 0) = -0.5;
  stoi_gold["StoiCheckRedox"](0, 1) = 1.5;
  stoi_gold["StoiCheckRedox"](0, 2) = -1;
  stoi_gold["StoiCheckGas"](0, 0) = 2.0;
  stoi_gold["StoiCheckGas"](0, 1) = 3.0;
  stoi_gold["StoiCheckGas"](0, 2) = -5.0;

  for (const auto & sp : {"OH-", "CO2(aq)", "CO3--", "StoiCheckRedox", "StoiCheckGas"})
  {
    const unsigned row = mgd.eqm_species_index[sp];
    ASSERT_EQ(mgd.eqm_stoichiometry.sub_matrix(row, 1, 0, 4), stoi_gold[sp]);
  }

  ASSERT_EQ(mgd.eqm_log10K(mgd.eqm_species_index["CO2(aq)"], 0), -6.5570);
  ASSERT_EQ(mgd.eqm_log10K(mgd.eqm_species_index["CO3--"], 0), 10.6169);
  ASSERT_EQ(mgd.eqm_log10K(mgd.eqm_species_index["OH-"], 0), 14.9325);
  ASSERT_EQ(mgd.eqm_log10K(mgd.eqm_species_index["StoiCheckRedox"], 0), -10.0553);
  ASSERT_EQ(mgd.eqm_log10K(mgd.eqm_species_index["StoiCheckGas"], 0), -2.9620 + 2 * (-10.0553));

  const unsigned sc_basis_posn = mgd.basis_species_index["StoiCheckBasis"];
  const unsigned sc_gas_posn = mgd.eqm_species_index["StoiCheckGas"];

  swapper.performSwap(mgd, "StoiCheckBasis", "StoiCheckGas");

  // check names are swapped correctly
  ASSERT_EQ(mgd.basis_species_index.size(), (std::size_t)4);
  for (const auto & sp : {"H2O", "StoiCheckGas", "H+", "HCO3-"})
    ASSERT_EQ(mgd.basis_species_index.count(sp), (std::size_t)1);
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_name[sp.second], sp.first);
  ASSERT_EQ(mgd.basis_species_index["StoiCheckGas"], sc_basis_posn);

  // check names are swapped correctly
  ASSERT_EQ(mgd.eqm_species_index.size(), (std::size_t)5);
  for (const auto & sp : {"OH-", "CO2(aq)", "CO3--", "StoiCheckRedox", "StoiCheckBasis"})
    ASSERT_EQ(mgd.eqm_species_index.count(sp), (std::size_t)1);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_name[sp.second], sp.first);
  ASSERT_EQ(mgd.eqm_species_index["StoiCheckBasis"], sc_gas_posn);

  // check the swap is recorded correctly
  ASSERT_EQ(mgd.swap_to_original_basis.n(), (unsigned)4);
  ASSERT_EQ(mgd.have_swapped_out_of_basis.size(), (std::size_t)1);
  ASSERT_EQ(mgd.have_swapped_into_basis.size(), (std::size_t)1);
  ASSERT_EQ(mgd.have_swapped_out_of_basis[0], sc_basis_posn);
  ASSERT_EQ(mgd.have_swapped_into_basis[0], sc_gas_posn);

  // check charges swapped correctly
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_charge[sp.second], charge_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_charge[sp.second], charge_gold[sp.first]);

  // check radii swapped correctly
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_radius[sp.second], radius_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_radius[sp.second], radius_gold[sp.first]);

  // check molecular weights swapped correctly
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_molecular_weight[sp.second], molecular_weight_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_molecular_weight[sp.second], molecular_weight_gold[sp.first]);

  // check molecular volumes swapped correctly
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_molecular_volume[sp.second], molecular_volume_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_molecular_volume[sp.second], molecular_volume_gold[sp.first]);

  // check isMineral information has swapped correctly
  for (const auto & species : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_mineral[species.second], false);
  for (const auto & species : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_mineral[species.second], false);

  // check isGas information has swapped correctly
  for (const auto & species : mgd.basis_species_index)
    if (species.first == "StoiCheckGas")
      ASSERT_EQ(mgd.basis_species_gas[species.second], true);
    else
      ASSERT_EQ(mgd.basis_species_gas[species.second], false);
  for (const auto & species : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_gas[species.second], false);

  // check isTransported information has swapped correctly
  for (const auto & species : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_transported[species.second], true);
  for (const auto & species : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_transported[species.second], true);

  // check stoichiometry
  stoi_gold = std::map<std::string, DenseMatrix<Real>>();
  for (const auto & sp : {"OH-", "CO2(aq)", "CO3--", "StoiCheckRedox", "StoiCheckBasis"})
    stoi_gold[sp] = DenseMatrix<Real>(1, 4);
  // remember the order of primaries: {"H2O", "StoiCheckGas", "H+", "HCO3-"}
  stoi_gold["CO2(aq)"](0, 0) = -1;
  stoi_gold["CO2(aq)"](0, 2) = 1;
  stoi_gold["CO2(aq)"](0, 3) = 1;
  stoi_gold["CO3--"](0, 2) = -1;
  stoi_gold["CO3--"](0, 3) = 1;
  stoi_gold["OH-"](0, 0) = 1;
  stoi_gold["OH-"](0, 2) = -1;
  stoi_gold["StoiCheckRedox"](0, 0) = -1.5;
  stoi_gold["StoiCheckRedox"](0, 1) = 0.5;
  stoi_gold["StoiCheckRedox"](0, 2) = 1.5;
  stoi_gold["StoiCheckBasis"](0, 0) = -2.0 / 3.0;
  stoi_gold["StoiCheckBasis"](0, 1) = 1.0 / 3.0;
  stoi_gold["StoiCheckBasis"](0, 2) = 5.0 / 3.0;

  for (const auto & sp : {"OH-", "CO2(aq)", "CO3--", "StoiCheckRedox", "StoiCheckBasis"})
  {
    const unsigned row = mgd.eqm_species_index[sp];
    for (unsigned i = 0; i < 4; ++i)
      ASSERT_NEAR(mgd.eqm_stoichiometry(row, i), stoi_gold[sp](0, i), eps);
  }

  // check eqm constants
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO2(aq)"], 0), -6.5570, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO2(aq)"], 1), -6.3660, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO2(aq)"], 2), -6.3325, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO2(aq)"], 3), -6.4330, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO2(aq)"], 4), -6.7420, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO2(aq)"], 5), -7.1880, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO2(aq)"], 6), -7.7630, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO2(aq)"], 7), -8.4650, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO3--"], 0), 10.6169, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO3--"], 1), 10.3439, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO3--"], 2), 10.2092, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO3--"], 3), 10.2793, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO3--"], 4), 10.5131, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO3--"], 5), 10.8637, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO3--"], 6), 11.2860, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["CO3--"], 7), 11.6319, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["OH-"], 0), 14.9325, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["OH-"], 1), 13.9868, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["OH-"], 2), 13.0199, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["OH-"], 3), 12.2403, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["OH-"], 4), 11.5940, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["OH-"], 5), 11.2191, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["OH-"], 6), 11.0880, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["OH-"], 7), 1001.2844, eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["StoiCheckRedox"], 0), -0.5 * (-2.9620), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["StoiCheckRedox"], 1), -0.5 * (-3.1848), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["StoiCheckRedox"], 2), -0.5 * (-3.3320), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["StoiCheckRedox"], 3), -0.5 * (-3.2902), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["StoiCheckRedox"], 4), -0.5 * (-3.1631), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["StoiCheckRedox"], 5), -0.5 * (-2.9499), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["StoiCheckRedox"], 6), -0.5 * (-2.7827), eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["StoiCheckRedox"], 7), -0.5 * (-2.3699), eps);
  const Real ot = -1.0 / 3.0;
  const Real tt = -2.0 / 3.0;
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["StoiCheckBasis"], 0),
              tt * (-10.0553) + ot * (-2.9620),
              eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["StoiCheckBasis"], 1),
              tt * (-8.4878) + ot * (-3.1848),
              eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["StoiCheckBasis"], 2),
              tt * (-6.6954) + ot * (-3.3320),
              eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["StoiCheckBasis"], 3),
              tt * (-5.0568) + ot * (-3.2902),
              eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["StoiCheckBasis"], 4),
              tt * (-3.4154) + ot * (-3.1631),
              eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["StoiCheckBasis"], 5),
              tt * (-2.0747) + ot * (-2.9499),
              eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["StoiCheckBasis"], 6),
              tt * (-.8908) + ot * (-2.7827),
              eps);
  ASSERT_NEAR(mgd.eqm_log10K(mgd.eqm_species_index["StoiCheckBasis"], 7),
              tt * (.2679) + ot * (-2.3699),
              eps);
}

/// Test the swap works on kinetic species
TEST(GeochemistrySpeciesSwapperTest, swap3)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);

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
  KineticRateUserDescription rate1("Fe(OH)3(ppd)",
                                   1.0,
                                   2.0,
                                   true,
                                   0.0,
                                   0.0,
                                   0.0,
                                   {"H+", "O2(aq)", "CO3--"},
                                   {1.1, 2.2, 3.3},
                                   {-0.1, 0.2, -0.3},
                                   {0.5, 0.25, 0.125},
                                   5.0,
                                   6.0,
                                   7.0,
                                   8.0,
                                   DirectionChoiceEnum::BOTH,
                                   "O2(aq)",
                                   1.0,
                                   -1.0,
                                   0.0);
  model.addKineticRate(rate1);
  KineticRateUserDescription rate2("Fe(OH)3(ppd)",
                                   -1.0,
                                   -2.0,
                                   false,
                                   0.0,
                                   0.0,
                                   0.0,
                                   {"O2(aq)", "Fe+++", "H2O"},
                                   {-1.1, -2.2, -3.3},
                                   {0.75, 0.875, 1.0},
                                   {0.2, 0.3, 0.4},
                                   -5.0,
                                   -6.0,
                                   -7.0,
                                   -8.0,
                                   DirectionChoiceEnum::BOTH,
                                   "Fe++",
                                   -2.0,
                                   -1.0,
                                   0.0);
  model.addKineticRate(rate2);
  KineticRateUserDescription rate3(">(s)FeO-",
                                   1.1,
                                   2.2,
                                   false,
                                   0.0,
                                   0.0,
                                   0.0,
                                   {"CO2(aq)", "Fe+++", "Fe++"},
                                   {1.25, 2.25, 3.25},
                                   {0.1, -0.12, 1.23},
                                   {0.5, 0.6, 0.7},
                                   5.5,
                                   -6.6,
                                   -7.7,
                                   -8.8,
                                   DirectionChoiceEnum::BOTH,
                                   "Fe+++",
                                   2.0,
                                   -1.0,
                                   0.0);
  model.addKineticRate(rate3);
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();
  GeochemistrySpeciesSwapper swapper(mgd.basis_species_index.size(), 1E-6);

  ASSERT_EQ(mgd.basis_species_index.size(), (std::size_t)7);
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_name[sp.second], sp.first);

  ASSERT_EQ(mgd.eqm_species_index.size(), (std::size_t)6);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_name[sp.second], sp.first);

  ASSERT_EQ(mgd.kin_species_index.size(), (std::size_t)4);
  for (const auto & sp : mgd.kin_species_index)
    ASSERT_EQ(mgd.kin_species_name[sp.second], sp.first);

  ASSERT_EQ(mgd.swap_to_original_basis.n(), (unsigned)0);
  ASSERT_EQ(mgd.have_swapped_out_of_basis.size(), (std::size_t)0);
  ASSERT_EQ(mgd.have_swapped_into_basis.size(), (std::size_t)0);

  for (const auto & species : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_mineral[species.second], false);
  for (const auto & species : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_mineral[species.second], false);
  for (const auto & species : mgd.kin_species_index)
    if (species.first == "Fe(OH)3(ppd)" || species.first == "Fe(OH)3(ppd)fake")
      ASSERT_EQ(mgd.kin_species_mineral[species.second], true);
    else
      ASSERT_EQ(mgd.kin_species_mineral[species.second], false);

  for (const auto & species : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_gas[species.second], false);
  for (const auto & species : mgd.eqm_species_index)
    if (species.first == "CH4(g)fake")
      ASSERT_EQ(mgd.eqm_species_gas[species.second], true);
    else
      ASSERT_EQ(mgd.eqm_species_gas[species.second], false);

  for (const auto & species : mgd.basis_species_index)
    if (species.first == ">(s)FeOH" || species.first == ">(w)FeOH")
      ASSERT_EQ(mgd.basis_species_transported[species.second], false);
    else
      ASSERT_EQ(mgd.basis_species_transported[species.second], true);
  for (const auto & species : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_transported[species.second], true);
  for (const auto & species : mgd.kin_species_index)
    if (species.first == "Fe(OH)3(ppd)" || species.first == "Fe(OH)3(ppd)fake" ||
        species.first == ">(s)FeO-")
      ASSERT_EQ(mgd.kin_species_transported[species.second], false);
    else
      ASSERT_EQ(mgd.kin_species_transported[species.second], true);

  ASSERT_EQ(mgd.eqm_species_index.size(), (std::size_t)6);
  for (const auto & sp : {"CO2(aq)", "CO3--", "OH-", "CH4(aq)", "Fe+++", "CH4(g)fake"})
    ASSERT_EQ(mgd.eqm_species_index.count(sp), (std::size_t)1);

  ASSERT_EQ(mgd.kin_species_index.size(), (std::size_t)4);
  for (const auto & sp : {"Fe(OH)3(ppd)", "Fe(OH)3(ppd)fake", "(O-phth)--", ">(s)FeO-"})
    ASSERT_EQ(mgd.kin_species_index.count(sp), (std::size_t)1);

  std::map<std::string, Real> charge_gold;
  charge_gold["H2O"] = 0.0;
  charge_gold["H+"] = 1.0;
  charge_gold[">(s)FeOH"] = 0.0;
  charge_gold[">(w)FeOH"] = 0.0;
  charge_gold["Fe++"] = 2.0;
  charge_gold["HCO3-"] = -1.0;
  charge_gold["O2(aq)"] = 0.0;
  charge_gold["CO2(aq)"] = 0.0;
  charge_gold["CO3--"] = -2.0;
  charge_gold["OH-"] = -1.0;
  charge_gold["CH4(aq)"] = 0.0;
  charge_gold["Fe+++"] = 3.0;
  charge_gold["CH4(g)fake"] = 0.0;
  charge_gold["Fe(OH)3(ppd)"] = 0.0;
  charge_gold["Fe(OH)3(ppd)fake"] = 0.0;
  charge_gold["(O-phth)--"] = -2.0;
  charge_gold[">(s)FeO-"] = -1.0;
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_charge[sp.second], charge_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_charge[sp.second], charge_gold[sp.first]);
  for (const auto & sp : mgd.kin_species_index)
    ASSERT_EQ(mgd.kin_species_charge[sp.second], charge_gold[sp.first]);

  std::map<std::string, Real> radius_gold;
  radius_gold["H2O"] = 0.0;
  radius_gold["H+"] = 9.0;
  radius_gold[">(s)FeOH"] = 0.0;
  radius_gold[">(w)FeOH"] = 0.0;
  radius_gold["Fe++"] = 6.0;
  radius_gold["HCO3-"] = 4.5;
  radius_gold["O2(aq)"] = -0.5;
  radius_gold["CO2(aq)"] = 4.0;
  radius_gold["CO3--"] = 4.5;
  radius_gold["OH-"] = 3.5;
  radius_gold["CH4(aq)"] = -0.5;
  radius_gold["Fe+++"] = 9.0;
  radius_gold["CH4(g)fake"] = 0.0;
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_radius[sp.second], radius_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_radius[sp.second], radius_gold[sp.first]);

  std::map<std::string, Real> molecular_weight_gold;
  molecular_weight_gold["H2O"] = 18.0152;
  molecular_weight_gold["H+"] = 1.0079;
  molecular_weight_gold[">(s)FeOH"] = 72.8543;
  molecular_weight_gold[">(w)FeOH"] = 1234.567;
  molecular_weight_gold["Fe++"] = 55.8470;
  molecular_weight_gold["HCO3-"] = 61.0171;
  molecular_weight_gold["O2(aq)"] = 31.9988;
  molecular_weight_gold["CO2(aq)"] = 44.0098;
  molecular_weight_gold["CO3--"] = 60.0092;
  molecular_weight_gold["OH-"] = 17.0073;
  molecular_weight_gold["CH4(aq)"] = 16.0426;
  molecular_weight_gold["Fe+++"] = 55.8470;
  molecular_weight_gold["CH4(g)fake"] = 16.0426;
  molecular_weight_gold["Fe(OH)3(ppd)"] = 106.8689;
  molecular_weight_gold["Fe(OH)3(ppd)fake"] = 106.8689;
  molecular_weight_gold["(O-phth)--"] = 164.1172;
  molecular_weight_gold[">(s)FeO-"] = 71.8464;
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_molecular_weight[sp.second], molecular_weight_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_molecular_weight[sp.second], molecular_weight_gold[sp.first]);
  for (const auto & sp : mgd.kin_species_index)
    ASSERT_EQ(mgd.kin_species_molecular_weight[sp.second], molecular_weight_gold[sp.first]);

  std::map<std::string, Real> molecular_volume_gold;
  molecular_volume_gold["H2O"] = 0.0;
  molecular_volume_gold["H+"] = 0.0;
  molecular_volume_gold[">(s)FeOH"] = 0.0;
  molecular_volume_gold[">(w)FeOH"] = 0.0;
  molecular_volume_gold["Fe++"] = 0.0;
  molecular_volume_gold["HCO3-"] = 0.0;
  molecular_volume_gold["O2(aq)"] = 0.0;
  molecular_volume_gold["CO2(aq)"] = 0.0;
  molecular_volume_gold["CO3--"] = 0.0;
  molecular_volume_gold["OH-"] = 0.0;
  molecular_volume_gold["CH4(aq)"] = 0.0;
  molecular_volume_gold["Fe+++"] = 0.0;
  molecular_volume_gold["CH4(g)fake"] = 0.0;
  molecular_volume_gold["Fe(OH)3(ppd)"] = 34.3200;
  molecular_volume_gold["Fe(OH)3(ppd)fake"] = 34.3200;
  molecular_volume_gold["(O-phth)--"] = 0.0;
  molecular_volume_gold[">(s)FeO-"] = 0.0;
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_molecular_volume[sp.second], molecular_volume_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_molecular_volume[sp.second], molecular_volume_gold[sp.first]);
  for (const auto & sp : mgd.kin_species_index)
    ASSERT_EQ(mgd.kin_species_molecular_volume[sp.second], molecular_volume_gold[sp.first]);

  std::map<std::string, DenseMatrix<Real>> stoi_gold;
  for (const auto & sp : {"CO2(aq)",
                          "CO3--",
                          "OH-",
                          "CH4(aq)",
                          "Fe+++",
                          "CH4(g)fake",
                          "Fe(OH)3(ppd)",
                          "Fe(OH)3(ppd)fake",
                          "(O-phth)--",
                          ">(s)FeO-"})
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
  stoi_gold["CH4(aq)"](0, 0) = 1;
  stoi_gold["CH4(aq)"](0, 1) = 1;
  stoi_gold["CH4(aq)"](0, 5) = 1;
  stoi_gold["CH4(aq)"](0, 6) = -2;
  stoi_gold["Fe+++"](0, 0) = -0.5;
  stoi_gold["Fe+++"](0, 4) = 1;
  stoi_gold["Fe+++"](0, 1) = 1;
  stoi_gold["Fe+++"](0, 6) = 0.25;
  stoi_gold["CH4(g)fake"](0, 0) = 3;
  stoi_gold["CH4(g)fake"](0, 4) = -2;
  stoi_gold["CH4(g)fake"](0, 5) = 3.5;
  stoi_gold["CH4(g)fake"](0, 6) = -4.5;
  stoi_gold["Fe(OH)3(ppd)"](0, 1) = -2;
  stoi_gold["Fe(OH)3(ppd)"](0, 4) = 1;
  stoi_gold["Fe(OH)3(ppd)"](0, 0) = 2.5;
  stoi_gold["Fe(OH)3(ppd)"](0, 6) = 0.25;
  stoi_gold["Fe(OH)3(ppd)fake"](0, 1) = -1;
  stoi_gold["Fe(OH)3(ppd)fake"](0, 0) = 2;
  stoi_gold["Fe(OH)3(ppd)fake"](0, 4) = 2;
  stoi_gold["Fe(OH)3(ppd)fake"](0, 6) = 0.5;
  stoi_gold["(O-phth)--"](0, 0) = -5;
  stoi_gold["(O-phth)--"](0, 5) = 8;
  stoi_gold["(O-phth)--"](0, 1) = 6;
  stoi_gold["(O-phth)--"](0, 6) = -7.5;
  stoi_gold[">(s)FeO-"](0, 2) = 1;
  stoi_gold[">(s)FeO-"](0, 1) = -1;

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

  const std::vector<Real> feoh3ppd = {
      6.1946, 4.8890, 3.4608, 2.2392, 1.1150, 0.2446, -0.5504, -1.5398};
  const std::vector<Real> fe3 = {10.0553, 8.4878, 6.6954, 5.0568, 3.4154, 2.0747, 0.8908, -0.2679};
  const std::vector<Real> ophth = {
      594.3211, 542.8292, 482.3612, 425.9738, 368.7004, 321.8658, 281.8216, 246.4849};
  const std::vector<Real> feom = {8.93,
                                  8.93 - 0.3 * 25,
                                  8.93 - 0.3 * 60,
                                  8.93 - 0.3 * 100,
                                  8.93 - 0.3 * 150,
                                  8.93 - 0.3 * 200,
                                  8.93 - 0.3 * 250,
                                  8.93 - 0.3 * 300};

  for (unsigned t = 0; t < 8; ++t)
  {
    ASSERT_NEAR(
        mgd.kin_log10K(mgd.kin_species_index.at("Fe(OH)3(ppd)"), t), feoh3ppd[t] - fe3[t], eps);
    ASSERT_NEAR(mgd.kin_log10K(mgd.kin_species_index.at("Fe(OH)3(ppd)fake"), t),
                feoh3ppd[t] - 2 * fe3[t],
                eps);
    ASSERT_NEAR(mgd.kin_log10K(mgd.kin_species_index.at("(O-phth)--"), t), ophth[t], eps);
    ASSERT_NEAR(mgd.kin_log10K(mgd.kin_species_index.at(">(s)FeO-"), t), feom[t], eps);
  }

  ASSERT_EQ(mgd.kin_rate.size(), (std::size_t)3);
  std::vector<std::vector<Real>> kin_rate_gold(3, std::vector<Real>(7 + 6, 0.0));
  std::vector<std::vector<Real>> kin_monod_gold(3, std::vector<Real>(7 + 6, 0.0));
  std::vector<std::vector<Real>> kin_k_gold(3, std::vector<Real>(7 + 6, 0.0));
  kin_rate_gold[0][mgd.basis_species_index.at("H+")] = 1.1;
  kin_rate_gold[0][mgd.basis_species_index.at("O2(aq)")] = 2.2;
  kin_rate_gold[0][7 + mgd.eqm_species_index.at("CO3--")] = 3.3;
  kin_monod_gold[0][mgd.basis_species_index.at("H+")] = -0.1;
  kin_monod_gold[0][mgd.basis_species_index.at("O2(aq)")] = 0.2;
  kin_monod_gold[0][7 + mgd.eqm_species_index.at("CO3--")] = -0.3;
  kin_k_gold[0][mgd.basis_species_index.at("H+")] = 0.5;
  kin_k_gold[0][mgd.basis_species_index.at("O2(aq)")] = 0.25;
  kin_k_gold[0][7 + mgd.eqm_species_index.at("CO3--")] = 0.125;
  kin_rate_gold[1][mgd.basis_species_index.at("O2(aq)")] = -1.1;
  kin_rate_gold[1][7 + mgd.eqm_species_index.at("Fe+++")] = -2.2;
  kin_rate_gold[1][mgd.basis_species_index.at("H2O")] = -3.3;
  kin_monod_gold[1][mgd.basis_species_index.at("O2(aq)")] = 0.75;
  kin_monod_gold[1][7 + mgd.eqm_species_index.at("Fe+++")] = 0.875;
  kin_monod_gold[1][mgd.basis_species_index.at("H2O")] = 1.0;
  kin_k_gold[1][mgd.basis_species_index.at("O2(aq)")] = 0.2;
  kin_k_gold[1][7 + mgd.eqm_species_index.at("Fe+++")] = 0.3;
  kin_k_gold[1][mgd.basis_species_index.at("H2O")] = 0.4;
  kin_rate_gold[2][7 + mgd.eqm_species_index.at("CO2(aq)")] = 1.25;
  kin_rate_gold[2][7 + mgd.eqm_species_index.at("Fe+++")] = 2.25;
  kin_rate_gold[2][mgd.basis_species_index.at("Fe++")] = 3.25;
  kin_monod_gold[2][7 + mgd.eqm_species_index.at("CO2(aq)")] = 0.1;
  kin_monod_gold[2][7 + mgd.eqm_species_index.at("Fe+++")] = -0.12;
  kin_monod_gold[2][mgd.basis_species_index.at("Fe++")] = 1.23;
  kin_k_gold[2][7 + mgd.eqm_species_index.at("CO2(aq)")] = 0.5;
  kin_k_gold[2][7 + mgd.eqm_species_index.at("Fe+++")] = 0.6;
  kin_k_gold[2][mgd.basis_species_index.at("Fe++")] = 0.7;
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 7 + 6; ++j)
    {
      EXPECT_EQ(mgd.kin_rate[i].promoting_indices[j], kin_rate_gold[i][j]);
      EXPECT_EQ(mgd.kin_rate[i].promoting_monod_indices[j], kin_monod_gold[i][j]);
      EXPECT_EQ(mgd.kin_rate[i].promoting_half_saturation[j], kin_k_gold[i][j]);
    }

  EXPECT_EQ(mgd.kin_rate[0].progeny_index, mgd.basis_species_index.at("O2(aq)"));
  EXPECT_EQ(mgd.kin_rate[1].progeny_index, mgd.basis_species_index.at("Fe++"));
  EXPECT_EQ(mgd.kin_rate[2].progeny_index, 7 + mgd.eqm_species_index.at("Fe+++"));

  const unsigned o2aq_posn = mgd.basis_species_index["O2(aq)"];
  const unsigned fe3_posn = mgd.eqm_species_index["Fe+++"];

  swapper.performSwap(mgd, "O2(aq)", "Fe+++");

  // check names are swapped correctly
  ASSERT_EQ(mgd.basis_species_index.size(), (std::size_t)7);
  for (const auto & sp : {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe++", "HCO3-", "Fe+++"})
    ASSERT_EQ(mgd.basis_species_index.count(sp), (std::size_t)1);
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_name[sp.second], sp.first);
  ASSERT_EQ(mgd.basis_species_index["Fe+++"], o2aq_posn);

  // check names are swapped correctly
  ASSERT_EQ(mgd.eqm_species_index.size(), (std::size_t)6);
  for (const auto & sp : {"CO2(aq)", "CO3--", "OH-", "CH4(aq)", "O2(aq)", "CH4(g)fake"})
    ASSERT_EQ(mgd.eqm_species_index.count(sp), (std::size_t)1);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_name[sp.second], sp.first);
  ASSERT_EQ(mgd.eqm_species_index["O2(aq)"], fe3_posn);

  ASSERT_EQ(mgd.kin_species_index.size(), (std::size_t)4);
  for (const auto & sp : mgd.kin_species_index)
    ASSERT_EQ(mgd.kin_species_name[sp.second], sp.first);
  for (const auto & sp : {"Fe(OH)3(ppd)", "Fe(OH)3(ppd)fake", "(O-phth)--", ">(s)FeO-"})
    ASSERT_EQ(mgd.kin_species_index.count(sp), (std::size_t)1);

  // check the swap is recorded correctly
  ASSERT_EQ(mgd.swap_to_original_basis.n(), (unsigned)7);
  ASSERT_EQ(mgd.have_swapped_out_of_basis.size(), (std::size_t)1);
  ASSERT_EQ(mgd.have_swapped_into_basis.size(), (std::size_t)1);
  ASSERT_EQ(mgd.have_swapped_out_of_basis[0], o2aq_posn);
  ASSERT_EQ(mgd.have_swapped_into_basis[0], fe3_posn);

  // check charges swapped correctly
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_charge[sp.second], charge_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_charge[sp.second], charge_gold[sp.first]);
  for (const auto & sp : mgd.kin_species_index)
    ASSERT_EQ(mgd.kin_species_charge[sp.second], charge_gold[sp.first]);

  // check radii swapped correctly
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_radius[sp.second], radius_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_radius[sp.second], radius_gold[sp.first]);

  // check molecular weights swapped correctly
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_molecular_weight[sp.second], molecular_weight_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_molecular_weight[sp.second], molecular_weight_gold[sp.first]);
  for (const auto & sp : mgd.kin_species_index)
    ASSERT_EQ(mgd.kin_species_molecular_weight[sp.second], molecular_weight_gold[sp.first]);

  // check molecular volumes swapped correctly
  for (const auto & sp : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_molecular_volume[sp.second], molecular_volume_gold[sp.first]);
  for (const auto & sp : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_molecular_volume[sp.second], molecular_volume_gold[sp.first]);
  for (const auto & sp : mgd.kin_species_index)
    ASSERT_EQ(mgd.kin_species_molecular_volume[sp.second], molecular_volume_gold[sp.first]);

  // check isMineral information has swapped correctly
  for (const auto & species : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_mineral[species.second], false);
  for (const auto & species : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_mineral[species.second], false);
  for (const auto & species : mgd.kin_species_index)
    if (species.first == "Fe(OH)3(ppd)" || species.first == "Fe(OH)3(ppd)fake")
      ASSERT_EQ(mgd.kin_species_mineral[species.second], true);
    else
      ASSERT_EQ(mgd.kin_species_mineral[species.second], false);

  // check isGas information has swapped correctly
  for (const auto & species : mgd.basis_species_index)
    ASSERT_EQ(mgd.basis_species_gas[species.second], false);
  for (const auto & species : mgd.eqm_species_index)
    if (species.first == "CH4(g)fake")
      ASSERT_EQ(mgd.eqm_species_gas[species.second], true);
    else
      ASSERT_EQ(mgd.eqm_species_gas[species.second], false);

  // check isTransported information has swapped correctly
  for (const auto & species : mgd.basis_species_index)
    if (species.first == ">(s)FeOH" || species.first == ">(w)FeOH")
      ASSERT_EQ(mgd.basis_species_transported[species.second], false);
    else
      ASSERT_EQ(mgd.basis_species_transported[species.second], true);
  for (const auto & species : mgd.eqm_species_index)
    ASSERT_EQ(mgd.eqm_species_transported[species.second], true);
  for (const auto & species : mgd.kin_species_index)
    if (species.first == "Fe(OH)3(ppd)" || species.first == "Fe(OH)3(ppd)fake" ||
        species.first == ">(s)FeO-")
      ASSERT_EQ(mgd.kin_species_transported[species.second], false);
    else
      ASSERT_EQ(mgd.kin_species_transported[species.second], true);

  // check stoichiometry
  for (const auto & sp : {"CO2(aq)",
                          "CO3--",
                          "OH-",
                          "CH4(aq)",
                          "O2(aq)",
                          "CH4(g)fake",
                          "Fe(OH)3(ppd)",
                          "Fe(OH)3(ppd)fake",
                          "(O-phth)--",
                          ">(s)FeO-"})
    stoi_gold[sp] = DenseMatrix<Real>(1, 7);
  // remember the order of primaries:
  // {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe++", "HCO3-", "Fe+++"}
  stoi_gold["CO2(aq)"](0, 0) = -1;
  stoi_gold["CO2(aq)"](0, 1) = 1;
  stoi_gold["CO2(aq)"](0, 5) = 1;
  stoi_gold["CO3--"](0, 5) = 1;
  stoi_gold["CO3--"](0, 1) = -1;
  stoi_gold["OH-"](0, 0) = 1;
  stoi_gold["OH-"](0, 1) = -1;
  stoi_gold["CH4(aq)"](0, 0) = -3;
  stoi_gold["CH4(aq)"](0, 1) = 9;
  stoi_gold["CH4(aq)"](0, 4) = 8;
  stoi_gold["CH4(aq)"](0, 5) = 1;
  stoi_gold["CH4(aq)"](0, 6) = -8;
  stoi_gold["O2(aq)"](0, 0) = 2;
  stoi_gold["O2(aq)"](0, 1) = -4;
  stoi_gold["O2(aq)"](0, 4) = -4;
  stoi_gold["O2(aq)"](0, 6) = 4;
  stoi_gold["CH4(g)fake"](0, 0) = -6;
  stoi_gold["CH4(g)fake"](0, 1) = 18;
  stoi_gold["CH4(g)fake"](0, 4) = 16;
  stoi_gold["CH4(g)fake"](0, 5) = 3.5;
  stoi_gold["CH4(g)fake"](0, 6) = -18;
  stoi_gold["Fe(OH)3(ppd)"](0, 0) = 3;
  stoi_gold["Fe(OH)3(ppd)"](0, 1) = -3;
  stoi_gold["Fe(OH)3(ppd)"](0, 6) = 1;
  stoi_gold["Fe(OH)3(ppd)fake"](0, 0) = 3;
  stoi_gold["Fe(OH)3(ppd)fake"](0, 1) = -3;
  stoi_gold["Fe(OH)3(ppd)fake"](0, 6) = 2;
  stoi_gold["(O-phth)--"](0, 0) = -5 - 7.5 * 2;
  stoi_gold["(O-phth)--"](0, 1) = 6 + 7.5 * 4;
  stoi_gold["(O-phth)--"](0, 4) = 7.5 * 4;
  stoi_gold["(O-phth)--"](0, 5) = 8;
  stoi_gold["(O-phth)--"](0, 6) = -7.5 * 4;
  stoi_gold[">(s)FeO-"](0, 2) = 1;
  stoi_gold[">(s)FeO-"](0, 1) = -1;

  for (const auto & sp : {"CO2(aq)", "CO3--", "OH-", "CH4(aq)", "O2(aq)", "CH4(g)fake"})
  {
    const unsigned row = mgd.eqm_species_index[sp];
    for (unsigned i = 0; i < 7; ++i)
      ASSERT_NEAR(mgd.eqm_stoichiometry(row, i), stoi_gold[sp](0, i), eps);
  }

  for (const auto & sp : {"Fe(OH)3(ppd)", "Fe(OH)3(ppd)fake", "(O-phth)--", ">(s)FeO-"})
  {
    const unsigned row = mgd.kin_species_index[sp];
    for (unsigned i = 0; i < 7; ++i)
      ASSERT_NEAR(mgd.kin_stoichiometry(row, i), stoi_gold[sp](0, i), eps);
  }

  for (unsigned t = 0; t < 8; ++t)
  {
    ASSERT_NEAR(mgd.kin_log10K(mgd.kin_species_index.at("Fe(OH)3(ppd)"), t), feoh3ppd[t], eps);
    ASSERT_NEAR(mgd.kin_log10K(mgd.kin_species_index.at("Fe(OH)3(ppd)fake"), t), feoh3ppd[t], eps);
    ASSERT_NEAR(mgd.kin_log10K(mgd.kin_species_index.at("(O-phth)--"), t),
                ophth[t] - (7.5 / 0.25) * fe3[t],
                eps);
    ASSERT_NEAR(mgd.kin_log10K(mgd.kin_species_index.at(">(s)FeO-"), t), feom[t], eps);
  }

  ASSERT_EQ(mgd.kin_rate.size(), (std::size_t)3);
  std::vector<std::vector<Real>> new_kin_rate_gold(3, std::vector<Real>(7 + 6, 0.0));
  std::vector<std::vector<Real>> new_kin_monod_gold(3, std::vector<Real>(7 + 6, 0.0));
  std::vector<std::vector<Real>> new_kin_k_gold(3, std::vector<Real>(7 + 6, 0.0));
  new_kin_rate_gold[0][mgd.basis_species_index.at("H+")] = 1.1;
  new_kin_rate_gold[0][7 + mgd.eqm_species_index.at("O2(aq)")] = 2.2;
  new_kin_rate_gold[0][7 + mgd.eqm_species_index.at("CO3--")] = 3.3;
  new_kin_monod_gold[0][mgd.basis_species_index.at("H+")] = -0.1;
  new_kin_monod_gold[0][7 + mgd.eqm_species_index.at("O2(aq)")] = 0.2;
  new_kin_monod_gold[0][7 + mgd.eqm_species_index.at("CO3--")] = -0.3;
  new_kin_k_gold[0][mgd.basis_species_index.at("H+")] = 0.5;
  new_kin_k_gold[0][7 + mgd.eqm_species_index.at("O2(aq)")] = 0.25;
  new_kin_k_gold[0][7 + mgd.eqm_species_index.at("CO3--")] = 0.125;
  new_kin_rate_gold[1][7 + mgd.eqm_species_index.at("O2(aq)")] = -1.1;
  new_kin_rate_gold[1][mgd.basis_species_index.at("Fe+++")] = -2.2;
  new_kin_rate_gold[1][mgd.basis_species_index.at("H2O")] = -3.3;
  new_kin_monod_gold[1][7 + mgd.eqm_species_index.at("O2(aq)")] = 0.75;
  new_kin_monod_gold[1][mgd.basis_species_index.at("Fe+++")] = 0.875;
  new_kin_monod_gold[1][mgd.basis_species_index.at("H2O")] = 1.0;
  new_kin_k_gold[1][7 + mgd.eqm_species_index.at("O2(aq)")] = 0.2;
  new_kin_k_gold[1][mgd.basis_species_index.at("Fe+++")] = 0.3;
  new_kin_k_gold[1][mgd.basis_species_index.at("H2O")] = 0.4;
  new_kin_rate_gold[2][7 + mgd.eqm_species_index.at("CO2(aq)")] = 1.25;
  new_kin_rate_gold[2][mgd.basis_species_index.at("Fe+++")] = 2.25;
  new_kin_rate_gold[2][mgd.basis_species_index.at("Fe++")] = 3.25;
  new_kin_monod_gold[2][7 + mgd.eqm_species_index.at("CO2(aq)")] = 0.1;
  new_kin_monod_gold[2][mgd.basis_species_index.at("Fe+++")] = -0.12;
  new_kin_monod_gold[2][mgd.basis_species_index.at("Fe++")] = 1.23;
  new_kin_k_gold[2][7 + mgd.eqm_species_index.at("CO2(aq)")] = 0.5;
  new_kin_k_gold[2][mgd.basis_species_index.at("Fe+++")] = 0.6;
  new_kin_k_gold[2][mgd.basis_species_index.at("Fe++")] = 0.7;
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 7 + 6; ++j)
    {
      EXPECT_EQ(mgd.kin_rate[i].promoting_indices[j], new_kin_rate_gold[i][j]);
      EXPECT_EQ(mgd.kin_rate[i].promoting_monod_indices[j], new_kin_monod_gold[i][j]);
      EXPECT_EQ(mgd.kin_rate[i].promoting_half_saturation[j], new_kin_k_gold[i][j]);
    }

  // check progeny_index is swapped correctly
  EXPECT_EQ(mgd.kin_rate[0].progeny_index, 7 + fe3_posn);
  EXPECT_EQ(mgd.kin_rate[1].progeny_index, mgd.basis_species_index.at("Fe++"));
  EXPECT_EQ(mgd.kin_rate[2].progeny_index, o2aq_posn);
}

/// Test the swap works on redox in disequilibrium
TEST(GeochemistrySpeciesSwapperTest, swap_redox)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);

  PertinentGeochemicalSystem model(database,
                                   {"H2O", "H+", "Fe++", "O2(aq)", "HCO3-", "(O-phth)--", "Fe+++"},
                                   {},
                                   {},
                                   {},
                                   {},
                                   {},
                                   "O2(aq)",
                                   "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();
  GeochemistrySpeciesSwapper swapper(mgd.basis_species_index.size(), 1E-6);

  EXPECT_EQ(mgd.redox_lhs, "e-");
  EXPECT_EQ(mgd.redox_stoichiometry.m(), (unsigned)3);
  const Real p = 1.0 / 4.0 / 7.5;
  // not sure of the order of the redox_stoichiometry stuff: this tells us
  const bool fe3_is_slot_one =
      (std::abs(mgd.redox_stoichiometry(1, 2)) > 1.0E-8); // note: involves Fe++
  const unsigned fe3_slot = (fe3_is_slot_one ? 1 : 2);
  const unsigned ophth_slot = (fe3_is_slot_one ? 2 : 1);
  // e- = p*(O-phth)-- + (5p+0.5)*H20 - 8p*HCO3- + (-6p-1)*H+
  EXPECT_NEAR(mgd.redox_stoichiometry(ophth_slot, 0), 5.0 * p + 0.5, 1.0E-8);
  EXPECT_NEAR(mgd.redox_stoichiometry(ophth_slot, 1), -6.0 * p - 1.0, 1.0E-8);
  EXPECT_NEAR(mgd.redox_stoichiometry(ophth_slot, 2), 0.0, 1.0E-8);
  EXPECT_NEAR(mgd.redox_stoichiometry(ophth_slot, 3), 0.0, 1.0E-8);
  EXPECT_NEAR(mgd.redox_stoichiometry(ophth_slot, 4), -8.0 * p, 1.0E-8);
  EXPECT_NEAR(mgd.redox_stoichiometry(ophth_slot, 5), p, 1.0E-8);
  EXPECT_NEAR(mgd.redox_stoichiometry(ophth_slot, 6), 0.0, 1.0E-8);
  // e- = Fe++ - Fe+++
  EXPECT_NEAR(mgd.redox_stoichiometry(fe3_slot, 0), 0.0, 1.0E-8);
  EXPECT_NEAR(mgd.redox_stoichiometry(fe3_slot, 1), 0.0, 1.0E-8);
  EXPECT_NEAR(mgd.redox_stoichiometry(fe3_slot, 2), 1.0, 1.0E-8);
  EXPECT_NEAR(mgd.redox_stoichiometry(fe3_slot, 3), 0.0, 1.0E-8);
  EXPECT_NEAR(mgd.redox_stoichiometry(fe3_slot, 4), 0.0, 1.0E-8);
  EXPECT_NEAR(mgd.redox_stoichiometry(fe3_slot, 5), 0.0, 1.0E-8);
  EXPECT_NEAR(mgd.redox_stoichiometry(fe3_slot, 6), -1.0, 1.0E-8);
  // record stoichiometry and log10K to compare with below
  DenseMatrix<Real> orig_stoi = mgd.redox_stoichiometry;
  DenseMatrix<Real> orig_log10K = mgd.redox_log10K;

  ASSERT_EQ(mgd.swap_to_original_basis.n(), (unsigned)0);
  ASSERT_EQ(mgd.have_swapped_out_of_basis.size(), (std::size_t)0);
  ASSERT_EQ(mgd.have_swapped_into_basis.size(), (std::size_t)0);

  const unsigned hco3_posn = mgd.basis_species_index["HCO3-"];
  const unsigned co3_posn = mgd.eqm_species_index["CO3--"];

  // swap CO3-- into the basis in place of HCO3-
  swapper.performSwap(mgd, "HCO3-", "CO3--");

  ASSERT_EQ(mgd.have_swapped_out_of_basis.size(), (std::size_t)1);
  ASSERT_EQ(mgd.have_swapped_into_basis.size(), (std::size_t)1);
  ASSERT_EQ(mgd.have_swapped_out_of_basis[0], hco3_posn);
  ASSERT_EQ(mgd.have_swapped_into_basis[0], co3_posn);
  ASSERT_EQ(mgd.swap_to_original_basis.n(), (unsigned)7);
  DenseMatrix<Real> gold_swap_matrix(7, 7);
  for (unsigned i = 0; i < 7; ++i)
  {
    gold_swap_matrix(i, i) = 1.0;
    gold_swap_matrix(hco3_posn, i) = 0.0;
  }
  gold_swap_matrix(hco3_posn, 1) = -1.0; // H+
  gold_swap_matrix(hco3_posn, 4) = 1.0;  // HCO3-
  for (unsigned i = 0; i < 7; ++i)
    for (unsigned j = 0; j < 7; ++j)
      EXPECT_EQ(mgd.swap_to_original_basis(i, j), gold_swap_matrix(i, j));

  EXPECT_EQ(mgd.redox_lhs, "e-");
  EXPECT_EQ(mgd.redox_stoichiometry.m(), (unsigned)3);

  // Since HCO3- = CO3-- + H+
  // e- = p*(O-phth)-- + (5p+0.5)*H20 - 8p*CO3-- + (-6p-1-8p)*H+
  EXPECT_NEAR(mgd.redox_stoichiometry(ophth_slot, 0), 5.0 * p + 0.5, 1.0E-8);
  EXPECT_NEAR(mgd.redox_stoichiometry(ophth_slot, 1), -6.0 * p - 1.0 - 8.0 * p, 1.0E-8);
  EXPECT_NEAR(mgd.redox_stoichiometry(ophth_slot, 2), 0.0, 1.0E-8);
  EXPECT_NEAR(mgd.redox_stoichiometry(ophth_slot, 3), 0.0, 1.0E-8);
  EXPECT_NEAR(mgd.redox_stoichiometry(ophth_slot, 4), -8.0 * p, 1.0E-8);
  EXPECT_NEAR(mgd.redox_stoichiometry(ophth_slot, 5), p, 1.0E-8);
  EXPECT_NEAR(mgd.redox_stoichiometry(ophth_slot, 6), 0.0, 1.0E-8);
  EXPECT_NEAR(
      mgd.redox_log10K(ophth_slot, 0), orig_log10K(ophth_slot, 0) + 8.0 * p * 10.6169, 1.0E-8);
  EXPECT_NEAR(
      mgd.redox_log10K(ophth_slot, 1), orig_log10K(ophth_slot, 1) + 8.0 * p * 10.3439, 1.0E-8);
  EXPECT_NEAR(
      mgd.redox_log10K(ophth_slot, 2), orig_log10K(ophth_slot, 2) + 8.0 * p * 10.2092, 1.0E-8);
  EXPECT_NEAR(
      mgd.redox_log10K(ophth_slot, 3), orig_log10K(ophth_slot, 3) + 8.0 * p * 10.2793, 1.0E-8);
  EXPECT_NEAR(
      mgd.redox_log10K(ophth_slot, 4), orig_log10K(ophth_slot, 4) + 8.0 * p * 10.5131, 1.0E-8);
  EXPECT_NEAR(
      mgd.redox_log10K(ophth_slot, 5), orig_log10K(ophth_slot, 5) + 8.0 * p * 10.8637, 1.0E-8);
  EXPECT_NEAR(
      mgd.redox_log10K(ophth_slot, 6), orig_log10K(ophth_slot, 6) + 8.0 * p * 11.2860, 1.0E-8);
  EXPECT_NEAR(
      mgd.redox_log10K(ophth_slot, 7), orig_log10K(ophth_slot, 7) + 8.0 * p * 11.6319, 1.0E-8);
  // e- = Fe++ - Fe+++ is unimpacted by the swap
  for (unsigned basis_i = 0; basis_i < 7; ++basis_i)
    EXPECT_NEAR(mgd.redox_stoichiometry(fe3_slot, basis_i), orig_stoi(fe3_slot, basis_i), 1.0E-8);
  for (unsigned temp = 0; temp < 8; ++temp)
    EXPECT_NEAR(mgd.redox_log10K(fe3_slot, temp), orig_log10K(fe3_slot, temp), 1.0E-8);
}

/// Check findBestEqmSwap execption
TEST(GeochemistrySpeciesSwapperTest, findBestEqmSwapException)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);

  // eqm species are: CO2(aq), CO3--, CaCO3, CaOH+, OH-, Calcite
  PertinentGeochemicalSystem model(
      database, {"H2O", "Ca++", "HCO3-", "H+"}, {"Calcite"}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();
  GeochemistrySpeciesSwapper swapper(mgd.basis_species_index.size(), 1E-6);
  unsigned best;

  try
  {
    const std::vector<Real> eqm_molality(6, 1.0);
    swapper.findBestEqmSwap(4, mgd, eqm_molality, false, false, false, best);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("basis index 4 must be less than 4") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    const std::vector<Real> eqm_molality(5, 1.0);
    swapper.findBestEqmSwap(1, mgd, eqm_molality, false, false, false, best);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Size of eqm_molality is 5 which is not equal to the number of "
                         "equilibrium species 6") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Check findBestEqmSwap
TEST(GeochemistrySpeciesSwapperTest, findBestEqmSwap)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, true, false);

  // eqm species are: CO2(aq), CO3--, CaCO3, CaOH+, OH-, Calcite
  PertinentGeochemicalSystem model(
      database, {"H2O", "Ca++", "HCO3-", "H+"}, {"Calcite"}, {}, {}, {}, {}, "O2(aq)", "e-");
  ModelGeochemicalDatabase mgd = model.modelGeochemicalDatabase();
  GeochemistrySpeciesSwapper swapper(mgd.basis_species_index.size(), 1E-6);
  unsigned best;
  // the following equilibrium molality has molality=5.0 for equilibrium mineral Calcite, which
  // violates the assumption in the Geochemistry module that all equilibrium minerals have zero
  // molality, but i'm setting this for testing only
  const std::vector<Real> eqm_molality = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0};
  bool legit = swapper.findBestEqmSwap(1, mgd, eqm_molality, false, false, false, best);
  EXPECT_TRUE(legit);
  EXPECT_EQ(best, (unsigned)3);
  legit = swapper.findBestEqmSwap(1, mgd, eqm_molality, true, false, false, best);
  EXPECT_TRUE(legit);
  EXPECT_EQ(best, (unsigned)5);
  legit = swapper.findBestEqmSwap(1, mgd, eqm_molality, true, true, true, best);
  EXPECT_TRUE(legit);
  EXPECT_EQ(best, (unsigned)5);

  // The following system has mineral, gas and sorption, with secondary species: CO2(aq), CO3--,
  // OH-, (O-phth)--, CH4(aq), Fe+++, CO2(aq), CO3--, OH-, >(s)FeO- (surface-sorption related),
  // Fe(OH)3(ppd) (mineral), CH4(g)fake (gas)
  PertinentGeochemicalSystem model_s(
      database,
      {"H2O", "H+", ">(s)FeOH", ">(w)FeOH", "Fe++", "HCO3-", "O2(aq)"},
      {"Fe(OH)3(ppd)"},
      {"CH4(g)fake"},
      {},
      {},
      {},
      "O2(aq)",
      "e-");
  ModelGeochemicalDatabase mgd_s = model_s.modelGeochemicalDatabase();
  GeochemistrySpeciesSwapper swapper_s(mgd_s.basis_species_index.size(), 1E-6);

  const std::vector<Real> eqm_molality_s = {0.0, 1.0, 2.0, 3.0, 4.0, 50.0, 6.0E3, 7.0E3, 8.0E3};
  legit = swapper_s.findBestEqmSwap(1, mgd_s, eqm_molality_s, false, false, false, best);
  EXPECT_TRUE(legit);
  EXPECT_EQ(best, (unsigned)5);
  legit = swapper_s.findBestEqmSwap(1, mgd_s, eqm_molality_s, true, false, false, best);
  EXPECT_TRUE(legit);
  EXPECT_EQ(best, (unsigned)7);
  legit = swapper_s.findBestEqmSwap(5, mgd_s, eqm_molality_s, false, true, false, best);
  EXPECT_TRUE(legit);
  EXPECT_EQ(best, (unsigned)8);
  legit = swapper_s.findBestEqmSwap(1, mgd_s, eqm_molality_s, false, false, true, best);
  EXPECT_TRUE(legit);
  EXPECT_EQ(best, (unsigned)6);
}
