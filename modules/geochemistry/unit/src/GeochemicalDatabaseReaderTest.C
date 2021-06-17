//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include <algorithm>

#include "GeochemicalDatabaseReader.h"

TEST(GeochemicalDatabaseReaderTest, filename)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  EXPECT_EQ(database.filename(), "database/moose_testdb.json");
}

TEST(GeochemicalDatabaseReaderTest, faultyDB)
{
  // Test that the database validator throws an error when a faulty
  // database is read (the full range of validation errors are tested
  // in GeochemicalDatabaseValidatorTest.C)
  try
  {
    GeochemicalDatabaseReader database("database/faultydbs/missing_header.json");
  }
  catch (const std::exception & err)
  {
    const std::string msg = "The MOOSE database database/faultydbs/missing_header.json does not "
                            "have a required \"Header\" field";

    std::size_t pos = std::string(err.what()).find(msg);
    ASSERT_TRUE(pos != std::string::npos);
  }
}

TEST(GeochemicalDatabaseReaderTest, getActivityModel)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  EXPECT_EQ(database.getActivityModel(), "debye-huckel");
}

TEST(GeochemicalDatabaseReaderTest, getFugacityModel)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  EXPECT_EQ(database.getFugacityModel(), "tsonopoulos");
}

TEST(GeochemicalDatabaseReaderTest, getLogKModel)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  EXPECT_EQ(database.getLogKModel(), "fourth-order");

  GeochemicalDatabaseReader database2("database/moose_testdb.json", true);

  EXPECT_EQ(database2.getLogKModel(), "fourth-order");

  GeochemicalDatabaseReader database3("database/moose_testdb.json", false, true);

  EXPECT_EQ(database3.getLogKModel(), "piecewise-linear");
}

TEST(GeochemicalDatabaseReaderTest, getTemperatures)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // Get the temperature points from the database and compare with the expected
  // valules
  auto temperature_points = database.getTemperatures();

  std::vector<Real> temperature_points_gold{0.0, 25.0, 60.0, 100.0, 150.0, 200.0, 250.0, 300.0};
  EXPECT_EQ(temperature_points, temperature_points_gold);
}

TEST(GeochemicalDatabaseReaderTest, getPressures)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // Get the pressure points from the database and compare with the expected
  // valules
  auto pressure_points = database.getPressures();

  std::vector<Real> pressure_points_gold{
      1.0134, 1.0134, 1.0134, 1.0134, 4.7600, 15.5490, 39.7760, 85.9270};
  EXPECT_EQ(pressure_points, pressure_points_gold);
}

TEST(GeochemicalDatabaseReaderTest, getDebyeHuckel)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // Get the Debye-Huckel from the database and compare with the expected
  // valules
  auto dh = database.getDebyeHuckel();

  std::vector<Real> adh_gold{.4913, .5092, .5450, .5998, .6898, .8099, .9785, 1.2555};
  std::vector<Real> bdh_gold{.3247, .3283, .3343, .3422, .3533, .3655, .3792, .3965};
  std::vector<Real> bdot_gold{.0174, .0410, .0440, .0460, .0470, .0470, .0340, 0.0000};

  EXPECT_EQ(dh.adh, adh_gold);
  EXPECT_EQ(dh.bdh, bdh_gold);
  EXPECT_EQ(dh.bdot, bdot_gold);
}

TEST(GeochemicalDatabaseReaderTest, getNeutralSpeciesActivity)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // Get the neutral species activity coefficients from the database
  // and compare with the expected values
  auto nsa = database.getNeutralSpeciesActivity();

  auto ns = nsa["co2"];
  std::vector<Real> a_gold{.1224, .1127, .09341, .08018, .08427, .09892, .1371, .1967};
  std::vector<Real> b_gold{-.004679, -.01049, -.0036, -.001503, -.01184, -.0104, -.007086, -.01809};
  std::vector<Real> c_gold{
      -0.0004114, 0.001545, 9.609e-05, 0.0005009, 0.003118, 0.001386, -0.002887, -0.002497};
  std::vector<Real> d_gold(8);

  EXPECT_EQ(ns.a, a_gold);
  EXPECT_EQ(ns.b, b_gold);
  EXPECT_EQ(ns.c, c_gold);
  EXPECT_TRUE(ns.d.empty());

  ns = nsa["h2o"];
  a_gold = {1.4203, 1.45397, 1.5012, 1.5551, 1.6225, 1.6899, 1.7573, 1.8247};
  b_gold = {0.0177, 0.022357, 0.0289, 0.036478, 0.045891, 0.0553, 0.0647, 0.0741};
  c_gold = {0.0103, 0.0093804, 0.008, 0.0064366, 0.0045221, 0.0026, 0.0006, -0.0013};
  d_gold = {-0.0005, -0.0005362, -0.0006, -0.0007132, -0.0008312, -0.0009, -0.0011, -0.0012};

  EXPECT_EQ(ns.a, a_gold);
  EXPECT_EQ(ns.b, b_gold);
  EXPECT_EQ(ns.c, c_gold);
  EXPECT_EQ(ns.d, d_gold);

  // check error is thrown if database has no neutral species
  GeochemicalDatabaseReader database_non("database/faultydbs/no_neutral_species.json");
  try
  {
    auto nsa = database_non.getNeutralSpeciesActivity();
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("No neutral species activity coefficients in database") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST(GeochemicalDatabaseReaderTest, getElements)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // Get the elements from the database and compare with the expected
  // valules
  auto els = database.getElements();
  auto el = els["Ag"];

  EXPECT_EQ(el.name, "Silver");
  EXPECT_EQ(el.molecular_weight, 107.8680);

  el = els["Al"];

  EXPECT_EQ(el.name, "Aluminum");
  EXPECT_EQ(el.molecular_weight, 26.9815);
}

TEST(GeochemicalDatabaseReaderTest, getBasisSpecies)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // Vector of primary species
  std::vector<std::string> bs_names{"Ca++", "HCO3-", "H+"};

  // Check the basis species
  auto bs = database.getBasisSpecies(bs_names);

  // Check that only the species in bs_names are returned
  std::vector<std::string> bs_names_returned;
  for (auto b : bs)
    bs_names_returned.push_back(b.first);

  std::sort(bs_names_returned.begin(), bs_names_returned.end());
  std::sort(bs_names.begin(), bs_names.end());
  EXPECT_EQ(bs_names_returned, bs_names);

  // Check each species
  auto species = bs["Ca++"];

  std::map<std::string, Real> els_gold = {{"Ca", 1}};
  EXPECT_EQ(species.radius, 6);
  EXPECT_EQ(species.charge, 2);
  EXPECT_EQ(species.molecular_weight, 40.08);
  EXPECT_EQ(species.elements, els_gold);

  species = bs["HCO3-"];

  els_gold = {{"C", 1}, {"H", 1}, {"O", 3}};
  EXPECT_EQ(species.radius, 4.5);
  EXPECT_EQ(species.charge, -1);
  EXPECT_EQ(species.molecular_weight, 61.0171);
  EXPECT_EQ(species.elements, els_gold);

  species = bs["H+"];

  els_gold = {{"H", 1}};
  EXPECT_EQ(species.radius, 9);
  EXPECT_EQ(species.charge, 1);
  EXPECT_EQ(species.molecular_weight, 1.0079);
  EXPECT_EQ(species.elements, els_gold);

  // check that an error is thrown if the species does not exist
  try
  {
    auto bs_not = database.getBasisSpecies({"does_not_exist"});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("does_not_exist does not exist in database database/moose_testdb.json") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST(GeochemicalDatabaseReaderTest, getEquilibriumSpecies)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // Vector of secondary species
  std::vector<std::string> ss_names{"CO2(aq)", "CO3--", "CaCO3", "CaOH+", "OH-"};

  // Check the secondary species
  auto ss = database.getEquilibriumSpecies(ss_names);

  // Check that only the species in ss_names are returned
  std::vector<std::string> ss_names_returned;
  for (auto s : ss)
    ss_names_returned.push_back(s.first);

  std::sort(ss_names_returned.begin(), ss_names_returned.end());
  std::sort(ss_names.begin(), ss_names.end());
  EXPECT_EQ(ss_names_returned, ss_names);

  // Check each species
  auto sspecies = ss["CO2(aq)"];
  std::vector<Real> logk_gold{
      -6.5570, -6.3660, -6.3325, -6.4330, -6.7420, -7.1880, -7.7630, -8.4650};
  std::map<std::string, Real> bs_gold = {{"H2O", -1}, {"H+", 1}, {"HCO3-", 1}};

  EXPECT_EQ(sspecies.radius, 4);
  EXPECT_EQ(sspecies.charge, 0);
  EXPECT_EQ(sspecies.molecular_weight, 44.0098);
  EXPECT_EQ(sspecies.basis_species, bs_gold);
  EXPECT_EQ(sspecies.equilibrium_const, logk_gold);

  sspecies = ss["CO3--"];
  logk_gold = {10.6169, 10.3439, 10.2092, 10.2793, 10.5131, 10.8637, 11.2860, 11.6319};
  bs_gold = {{"H+", -1}, {"HCO3-", 1}};

  EXPECT_EQ(sspecies.radius, 4.5);
  EXPECT_EQ(sspecies.charge, -2);
  EXPECT_EQ(sspecies.molecular_weight, 60.0092);
  EXPECT_EQ(sspecies.basis_species, bs_gold);
  EXPECT_EQ(sspecies.equilibrium_const, logk_gold);

  sspecies = ss["CaCO3"];
  logk_gold = {7.5520, 7.1280, 6.7340, 6.4350, 6.1810, 5.9320, 5.5640, 4.7890};
  bs_gold = {{"Ca++", 1}, {"HCO3-", 1}, {"H+", -1}};

  EXPECT_EQ(sspecies.radius, 4);
  EXPECT_EQ(sspecies.charge, 0);
  EXPECT_EQ(sspecies.molecular_weight, 100.0892);
  EXPECT_EQ(sspecies.basis_species, bs_gold);
  EXPECT_EQ(sspecies.equilibrium_const, logk_gold);

  sspecies = ss["CaOH+"];
  logk_gold = {13.7095, 12.6887, 11.5069, 10.4366, 9.3958, 8.5583, 7.8155, 7.0306};
  bs_gold = {{"Ca++", 1}, {"H2O", 1}, {"H+", -1}};

  EXPECT_EQ(sspecies.radius, 4);
  EXPECT_EQ(sspecies.charge, 1);
  EXPECT_EQ(sspecies.molecular_weight, 57.0873);
  EXPECT_EQ(sspecies.basis_species, bs_gold);
  EXPECT_EQ(sspecies.equilibrium_const, logk_gold);

  sspecies = ss["OH-"];
  logk_gold = {14.9325, 13.9868, 13.0199, 12.2403, 11.5940, 11.2191, 11.0880, 1001.2844};
  bs_gold = {{"H2O", 1}, {"H+", -1}};

  EXPECT_EQ(sspecies.radius, 3.5);
  EXPECT_EQ(sspecies.charge, -1);
  EXPECT_EQ(sspecies.molecular_weight, 17.0073);
  EXPECT_EQ(sspecies.basis_species, bs_gold);
  EXPECT_EQ(sspecies.equilibrium_const, logk_gold);

  // check that an error is thrown if the species does not exist
  try
  {
    auto bs_not = database.getEquilibriumSpecies({"does_not_exist"});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("does_not_exist does not exist in database database/moose_testdb.json") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST(GeochemicalDatabaseReaderTest, getMineralSpecies)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // Vector of mineral species to be read
  std::vector<std::string> ms_names{"Calcite", "Fe(OH)3(ppd)"};

  // Check the mineral species
  auto ms = database.getMineralSpecies(ms_names);

  // Check that only the species in ms_names are returned
  std::vector<std::string> ms_names_returned;
  for (auto s : ms)
    ms_names_returned.push_back(s.first);

  std::sort(ms_names_returned.begin(), ms_names_returned.end());
  std::sort(ms_names.begin(), ms_names.end());
  EXPECT_EQ(ms_names_returned, ms_names);

  auto mspecies = ms["Calcite"];
  std::vector<Real> logk_gold = {2.0683, 1.7130, 1.2133, .6871, .0762, -.5349, -1.2301, -2.2107};
  std::map<std::string, Real> bs_gold = {{"Ca++", 1}, {"H+", -1}, {"HCO3-", 1}};

  EXPECT_EQ(mspecies.molecular_volume, 36.934);
  EXPECT_EQ(mspecies.molecular_weight, 100.0892);
  EXPECT_EQ(mspecies.basis_species, bs_gold);
  EXPECT_EQ(mspecies.equilibrium_const, logk_gold);
  EXPECT_EQ(mspecies.surface_area, 0.0);

  mspecies = ms["Fe(OH)3(ppd)"];
  logk_gold = {6.1946, 4.8890, 3.4608, 2.2392, 1.1150, .2446, -.5504, -1.5398};
  bs_gold = {{"H+", -3}, {"Fe+++", 1}, {"H2O", 3}};

  EXPECT_EQ(mspecies.molecular_volume, 34.32);
  EXPECT_EQ(mspecies.molecular_weight, 106.8689);
  EXPECT_EQ(mspecies.basis_species, bs_gold);
  EXPECT_EQ(mspecies.equilibrium_const, logk_gold);
  EXPECT_EQ(mspecies.surface_area, 600.0);
  bs_gold = {{">(s)FeOH", 0.005}, {">(w)FeOH", 0.2}};
  EXPECT_EQ(mspecies.sorption_sites, bs_gold);

  // check that an error is thrown if the species does not exist
  try
  {
    auto bs_not = database.getMineralSpecies({"does_not_exist"});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("does_not_exist does not exist in database database/moose_testdb.json") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST(GeochemicalDatabaseReaderTest, getGasSpecies)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // Vector of gas species to be read
  std::vector<std::string> gs_names{"N2(g)", "CH4(g)"};

  // Check the gas species
  auto gs = database.getGasSpecies(gs_names);

  // Check that only the species in gs_names are returned
  std::vector<std::string> gs_names_returned;
  for (auto s : gs)
    gs_names_returned.push_back(s.first);

  std::sort(gs_names_returned.begin(), gs_names_returned.end());
  std::sort(gs_names.begin(), gs_names.end());
  EXPECT_EQ(gs_names_returned, gs_names);

  auto gspecies = gs["N2(g)"];
  std::vector<Real> logk_gold = {
      -2.9620, -3.1848, -3.3320, -3.2902, -3.1631, -2.9499, -2.7827, -2.3699};
  std::map<std::string, Real> bs_gold = {{"N2(aq)", 1}};

  EXPECT_EQ(gspecies.molecular_weight, 28.0134);
  EXPECT_EQ(gspecies.basis_species, bs_gold);
  EXPECT_EQ(gspecies.equilibrium_const, logk_gold);
  EXPECT_TRUE(gspecies.chi.empty());
  EXPECT_EQ(gspecies.Pcrit, 33.9);
  EXPECT_EQ(gspecies.Tcrit, 126.2);
  EXPECT_EQ(gspecies.omega, .039);

  gspecies = gs["CH4(g)"];
  logk_gold = {-2.6487, -2.8202, -2.9329, -2.9446, -2.9163, -2.7253, -2.4643, -2.1569};
  bs_gold = {{"CH4(aq)", 1}};
  std::vector<Real> chi_gold = {-537.779, 1.54946, -.000927827, 1.20861, -.00370814, 3.33804e-6};

  EXPECT_EQ(gspecies.molecular_weight, 16.0426);
  EXPECT_EQ(gspecies.basis_species, bs_gold);
  EXPECT_EQ(gspecies.equilibrium_const, logk_gold);
  EXPECT_EQ(gspecies.chi, chi_gold);
  EXPECT_EQ(gspecies.Pcrit, 46.0);
  EXPECT_EQ(gspecies.Tcrit, 190.4);
  EXPECT_EQ(gspecies.omega, .011);

  // check that an error is thrown if the species does not exist
  try
  {
    auto bs_not = database.getGasSpecies({"does_not_exist"});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("does_not_exist does not exist in database database/moose_testdb.json") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST(GeochemicalDatabaseReaderTest, getRedoxSpecies)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // Vector of redox species
  std::vector<std::string> rs_names{"Am++++"};

  // Check the secondary species
  auto rs = database.getRedoxSpecies(rs_names);

  // Check that only the species in ss_names are returned
  std::vector<std::string> rs_names_returned;
  for (auto s : rs)
    rs_names_returned.push_back(s.first);

  std::sort(rs_names_returned.begin(), rs_names_returned.end());
  std::sort(rs_names.begin(), rs_names.end());
  EXPECT_EQ(rs_names_returned, rs_names);

  auto rspecies = rs["Am++++"];
  std::vector<Real> logk_gold = {
      18.7967, 18.0815, 17.2698, 16.5278, 15.8024, 15.2312, 14.7898, 14.4250};
  std::map<std::string, Real> bs_gold = {{"H2O", -0.5}, {"H+", 1}, {"Am+++", 1}, {"O2(aq)", 0.250}};

  EXPECT_EQ(rspecies.radius, 11);
  EXPECT_EQ(rspecies.charge, 4);
  EXPECT_EQ(rspecies.molecular_weight, 241.0600);
  EXPECT_EQ(rspecies.basis_species, bs_gold);
  EXPECT_EQ(rspecies.equilibrium_const, logk_gold);

  // check that an error is thrown if the species does not exist
  try
  {
    auto bs_not = database.getRedoxSpecies({"does_not_exist"});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("does_not_exist does not exist in database database/moose_testdb.json") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST(GeochemicalDatabaseReaderTest, getOxideSpecies)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // Vector of gas species to be read
  std::vector<std::string> os_names{"Cu2O"};

  // Check the oxide species
  auto os = database.getOxideSpecies(os_names);

  // Check that only the species in gs_names are returned
  std::vector<std::string> os_names_returned;
  for (auto s : os)
    os_names_returned.push_back(s.first);

  EXPECT_EQ(os_names_returned, os_names);

  auto ospecies = os["Cu2O"];
  std::map<std::string, Real> bs_gold = {{"H+", -2}, {"Cu+", 2}, {"H2O", 1}};

  EXPECT_EQ(ospecies.molecular_weight, 143.0929);
  EXPECT_EQ(ospecies.basis_species, bs_gold);

  // check that an error is thrown if the species does not exist
  try
  {
    auto bs_not = database.getOxideSpecies({"does_not_exist"});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("does_not_exist does not exist in database database/moose_testdb.json") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST(GeochemicalDatabaseReaderTest, getSurfaceSpecies)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  std::vector<std::string> ss_names{">(s)FeO-"};

  auto ss = database.getSurfaceSpecies(ss_names);

  // Check that only the species in ss_names are returned
  std::vector<std::string> ss_names_returned;
  for (auto s : ss)
    ss_names_returned.push_back(s.first);

  EXPECT_EQ(ss_names_returned, ss_names);

  auto sspecies = ss[">(s)FeO-"];
  std::map<std::string, Real> ss_gold = {{">(s)FeOH", 1}, {"H+", -1}};

  EXPECT_EQ(sspecies.basis_species, ss_gold);

  // check that an error is thrown if the species does not exist
  try
  {
    auto bs_not = database.getSurfaceSpecies({"does_not_exist"});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("does_not_exist does not exist in database database/moose_testdb.json") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST(GeochemicalDatabaseReaderTest, equilibriumReactions)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // Secondary equilibrium species
  std::vector<std::string> names{"CO2(aq)", "CO3--", "CaCO3", "CaOH+", "OH-"};

  auto reactions = database.equilibriumReactions(names);

  EXPECT_EQ(reactions[0], "CO2(aq) = H+ - H2O + HCO3-");
  EXPECT_EQ(reactions[1], "CO3-- = - H+ + HCO3-");
  EXPECT_EQ(reactions[2], "CaCO3 = Ca++ - H+ + HCO3-");
  EXPECT_EQ(reactions[3], "CaOH+ = Ca++ - H+ + H2O");
  EXPECT_EQ(reactions[4], "OH- = - H+ + H2O");

  // check that an error is thrown if the species does not exist
  try
  {
    auto reactions_not = database.equilibriumReactions({"does_not_exist"});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("does_not_exist does not exist in database database/moose_testdb.json") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST(GeochemicalDatabaseReaderTest, mineralReactions)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // Secondary mineral species
  std::vector<std::string> names{"Calcite"};

  auto reactions = database.mineralReactions(names);

  EXPECT_EQ(reactions[0], "Calcite = Ca++ - H+ + HCO3-");

  // check that an error is thrown if the species does not exist
  try
  {
    auto reactions_not = database.mineralReactions({"does_not_exist"});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("does_not_exist does not exist in database database/moose_testdb.json") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST(GeochemicalDatabaseReaderTest, gasReactions)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // Secondary gas species
  std::vector<std::string> names{"CH4(g)", "N2(g)"};

  auto reactions = database.gasReactions(names);

  EXPECT_EQ(reactions[0], "CH4(g) = CH4(aq)");
  EXPECT_EQ(reactions[1], "N2(g) = N2(aq)");

  // check that an error is thrown if the species does not exist
  try
  {
    auto reactions_not = database.gasReactions({"does_not_exist"});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("does_not_exist does not exist in database database/moose_testdb.json") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST(GeochemicalDatabaseReaderTest, redoxReactions)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // Secondary redox couples
  std::vector<std::string> names{"(O-phth)--", "Am++++"};

  auto reactions = database.redoxReactions(names);

  EXPECT_EQ(reactions[0], "(O-phth)-- = 6H+ -5H2O + 8HCO3- -7.5O2(aq)");
  EXPECT_EQ(reactions[1], "Am++++ = Am+++ + H+ -0.5H2O + 0.25O2(aq)");

  // check that an error is thrown if the species does not exist
  try
  {
    auto reactions_not = database.redoxReactions({"does_not_exist"});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("does_not_exist does not exist in database database/moose_testdb.json") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST(GeochemicalDatabaseReaderTest, oxideReactions)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  // Secondary oxide species
  std::vector<std::string> names{"Cu2O"};

  auto reactions = database.oxideReactions(names);

  EXPECT_EQ(reactions[0], "Cu2O = 2Cu+ -2H+ + H2O");

  // check that an error is thrown if the species does not exist
  try
  {
    auto reactions_not = database.oxideReactions({"does_not_exist"});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("does_not_exist does not exist in database database/moose_testdb.json") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST(GeochemicalDatabaseReaderTest, isBasisSpecies)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  EXPECT_TRUE(database.isBasisSpecies("Ca++"));
  EXPECT_TRUE(database.isBasisSpecies("H2O"));
  EXPECT_FALSE(database.isBasisSpecies("Ag"));
  EXPECT_FALSE(database.isBasisSpecies("CO2(aq)"));
  EXPECT_FALSE(database.isBasisSpecies("CO3--"));
  EXPECT_FALSE(database.isBasisSpecies("Calcite"));
  EXPECT_FALSE(database.isBasisSpecies("Fe(OH)3(ppd)"));
  EXPECT_FALSE(database.isBasisSpecies("CH4(g)"));
  EXPECT_FALSE(database.isBasisSpecies("(O-phth)--"));
  EXPECT_FALSE(database.isBasisSpecies("Fe+++"));
  EXPECT_FALSE(database.isBasisSpecies("Cu2O"));
  EXPECT_FALSE(database.isBasisSpecies(">(s)FeO-"));
  EXPECT_FALSE(database.isBasisSpecies("e-"));
}

TEST(GeochemicalDatabaseReaderTest, isSecondarySpecies)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  EXPECT_FALSE(database.isSecondarySpecies("Ca++"));
  EXPECT_FALSE(database.isSecondarySpecies("H2O"));
  EXPECT_FALSE(database.isSecondarySpecies("Ag"));
  EXPECT_TRUE(database.isSecondarySpecies("CO2(aq)"));
  EXPECT_TRUE(database.isSecondarySpecies("CO3--"));
  EXPECT_TRUE(database.isSecondarySpecies("OH-"));
  EXPECT_FALSE(database.isSecondarySpecies("Calcite"));
  EXPECT_FALSE(database.isSecondarySpecies("Fe(OH)3(ppd)"));
  EXPECT_FALSE(database.isSecondarySpecies("CH4(g)"));
  EXPECT_FALSE(database.isSecondarySpecies("(O-phth)--"));
  EXPECT_FALSE(database.isSecondarySpecies("Fe+++"));
  EXPECT_FALSE(database.isSecondarySpecies("Cu2O"));
  EXPECT_FALSE(database.isSecondarySpecies(">(s)FeO-"));
  EXPECT_TRUE(database.isSecondarySpecies("e-"));
}

TEST(GeochemicalDatabaseReaderTest, isMineralSpecies)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  EXPECT_FALSE(database.isMineralSpecies("Ca++"));
  EXPECT_FALSE(database.isMineralSpecies("H2O"));
  EXPECT_FALSE(database.isMineralSpecies("Ag"));
  EXPECT_FALSE(database.isMineralSpecies("CO2(aq)"));
  EXPECT_FALSE(database.isMineralSpecies("CO3--"));
  EXPECT_TRUE(database.isMineralSpecies("Calcite"));
  EXPECT_TRUE(database.isMineralSpecies("Fe(OH)3(ppd)"));
  EXPECT_FALSE(database.isMineralSpecies("CH4(g)"));
  EXPECT_FALSE(database.isMineralSpecies("(O-phth)--"));
  EXPECT_FALSE(database.isMineralSpecies("Fe+++"));
  EXPECT_FALSE(database.isMineralSpecies("Cu2O"));
  EXPECT_FALSE(database.isMineralSpecies(">(s)FeO-"));
  EXPECT_FALSE(database.isMineralSpecies("e-"));
}

TEST(GeochemicalDatabaseReaderTest, isRedoxSpecies)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  EXPECT_FALSE(database.isRedoxSpecies("Ca++"));
  EXPECT_FALSE(database.isRedoxSpecies("H2O"));
  EXPECT_FALSE(database.isRedoxSpecies("Ag"));
  EXPECT_FALSE(database.isRedoxSpecies("CO2(aq)"));
  EXPECT_FALSE(database.isRedoxSpecies("CO3--"));
  EXPECT_FALSE(database.isRedoxSpecies("Calcite"));
  EXPECT_FALSE(database.isRedoxSpecies("Fe(OH)3(ppd)"));
  EXPECT_FALSE(database.isRedoxSpecies("CH4(g)"));
  EXPECT_TRUE(database.isRedoxSpecies("(O-phth)--"));
  EXPECT_TRUE(database.isRedoxSpecies("Fe+++"));
  EXPECT_FALSE(database.isRedoxSpecies("Cu2O"));
  EXPECT_FALSE(database.isRedoxSpecies(">(s)FeO-"));
  EXPECT_FALSE(database.isRedoxSpecies("e-"));
}

TEST(GeochemicalDatabaseReaderTest, isGasSpecies)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  EXPECT_FALSE(database.isGasSpecies("Ca++"));
  EXPECT_FALSE(database.isGasSpecies("H2O"));
  EXPECT_FALSE(database.isGasSpecies("Ag"));
  EXPECT_FALSE(database.isGasSpecies("CO2(aq)"));
  EXPECT_FALSE(database.isGasSpecies("CO3--"));
  EXPECT_FALSE(database.isGasSpecies("Calcite"));
  EXPECT_FALSE(database.isGasSpecies("Fe(OH)3(ppd)"));
  EXPECT_TRUE(database.isGasSpecies("CH4(g)"));
  EXPECT_FALSE(database.isGasSpecies("(O-phth)--"));
  EXPECT_FALSE(database.isGasSpecies("Fe+++"));
  EXPECT_FALSE(database.isGasSpecies("Cu2O"));
  EXPECT_FALSE(database.isGasSpecies(">(s)FeO-"));
  EXPECT_FALSE(database.isGasSpecies("e-"));
}

TEST(GeochemicalDatabaseReaderTest, isSorbingMineral)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  EXPECT_FALSE(database.isSorbingMineral("Ca++"));
  EXPECT_FALSE(database.isSorbingMineral("H2O"));
  EXPECT_FALSE(database.isSorbingMineral("Ag"));
  EXPECT_FALSE(database.isSorbingMineral("CO2(aq)"));
  EXPECT_FALSE(database.isSorbingMineral("CO3--"));
  EXPECT_FALSE(database.isSorbingMineral("Calcite"));
  EXPECT_TRUE(database.isSorbingMineral("Fe(OH)3(ppd)"));
  EXPECT_FALSE(database.isSorbingMineral("CH4(g)"));
  EXPECT_FALSE(database.isSorbingMineral("(O-phth)--"));
  EXPECT_FALSE(database.isSorbingMineral("Fe+++"));
  EXPECT_FALSE(database.isSorbingMineral("Cu2O"));
  EXPECT_FALSE(database.isSorbingMineral(">(s)FeO-"));
  EXPECT_FALSE(database.isSorbingMineral("e-"));
}

TEST(GeochemicalDatabaseReaderTest, isOxideSpecies)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  EXPECT_FALSE(database.isOxideSpecies("Ca++"));
  EXPECT_FALSE(database.isOxideSpecies("H2O"));
  EXPECT_FALSE(database.isOxideSpecies("Ag"));
  EXPECT_FALSE(database.isOxideSpecies("CO2(aq)"));
  EXPECT_FALSE(database.isOxideSpecies("CO3--"));
  EXPECT_FALSE(database.isOxideSpecies("Calcite"));
  EXPECT_FALSE(database.isOxideSpecies("Fe(OH)3(ppd)"));
  EXPECT_FALSE(database.isOxideSpecies("CH4(g)"));
  EXPECT_FALSE(database.isOxideSpecies("(O-phth)--"));
  EXPECT_FALSE(database.isOxideSpecies("Fe+++"));
  EXPECT_TRUE(database.isOxideSpecies("Cu2O"));
  EXPECT_FALSE(database.isOxideSpecies(">(s)FeO-"));
  EXPECT_FALSE(database.isOxideSpecies("e-"));
}

TEST(GeochemicalDatabaseReaderTest, isSurfaceSpecies)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  EXPECT_FALSE(database.isSurfaceSpecies("Ca++"));
  EXPECT_FALSE(database.isSurfaceSpecies("H2O"));
  EXPECT_FALSE(database.isSurfaceSpecies("Ag"));
  EXPECT_FALSE(database.isSurfaceSpecies("CO2(aq)"));
  EXPECT_FALSE(database.isSurfaceSpecies("CO3--"));
  EXPECT_FALSE(database.isSurfaceSpecies("Calcite"));
  EXPECT_FALSE(database.isSurfaceSpecies("Fe(OH)3(ppd)"));
  EXPECT_FALSE(database.isSurfaceSpecies("CH4(g)"));
  EXPECT_FALSE(database.isSurfaceSpecies("(O-phth)--"));
  EXPECT_FALSE(database.isSurfaceSpecies("Fe+++"));
  EXPECT_FALSE(database.isSurfaceSpecies("Cu2O"));
  EXPECT_TRUE(database.isSurfaceSpecies(">(s)FeO-"));
  EXPECT_FALSE(database.isSurfaceSpecies("e-"));
}

TEST(GeochemicalDatabaseReaderTest, mineralSpeciesNames)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  std::vector<std::string> names = database.mineralSpeciesNames();
  for (const auto & n : {"Calcite",
                         "Calcite_asdf",
                         "Fe(OH)3(ppd)",
                         "Fe(OH)3(ppd)fake",
                         "Goethite",
                         "Something",
                         "problematic_sorber"})
    EXPECT_TRUE(std::find(names.begin(), names.end(), n) != names.end());
  EXPECT_EQ(names.size(), (std::size_t)7);
}

TEST(GeochemicalDatabaseReaderTest, secondarySpeciesNames)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  std::vector<std::string> names = database.secondarySpeciesNames();
  for (const auto & n : {"CO2(aq)", "CO3--", "CaCO3", "CaOH+", "OH-", "e-", "seq_radius_neg1"})
    EXPECT_TRUE(std::find(names.begin(), names.end(), n) != names.end());
  EXPECT_EQ(names.size(), (std::size_t)8);
}

TEST(GeochemicalDatabaseReaderTest, redoxCoupleNames)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  std::vector<std::string> names = database.redoxCoupleNames();
  for (const auto & n : {"(O-phth)--", "Am++++", "CH4(aq)", "Fe+++", "StoiCheckRedox"})
    EXPECT_TRUE(std::find(names.begin(), names.end(), n) != names.end());
  EXPECT_EQ(names.size(), (std::size_t)5);
}

TEST(GeochemicalDatabaseReaderTest, surfaceSpeciesNames)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  std::vector<std::string> names = database.surfaceSpeciesNames();
  for (const auto & n : {">(s)FeO-", ">(s)FeOCa+"})
    EXPECT_TRUE(std::find(names.begin(), names.end(), n) != names.end());
  EXPECT_EQ(names.size(), (std::size_t)2);
}

TEST(GeochemicalDatabaseReaderTest, getSpeciesData)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  std::string data = database.getSpeciesData("Ca++");
  std::string gold = "Ca++:\n{\n    \"charge\": 2,\n    \"elements\": {\n        \"Ca\": 1.0\n    "
                     "},\n    \"molecular weight\": 40.08,\n    \"radius\": 6.0\n}";
  EXPECT_EQ(data, gold);

  // check that an error is thrown if the species does not exist
  try
  {
    auto reactions_not = database.getSpeciesData({"does_not_exist"});
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("does_not_exist is not a species in the database") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Test the reexpression of the free electron in terms of O2(aq)
TEST(GeochemicalDatabaseReaderTest, freeElectron)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json");

  auto fe = database.getEquilibriumSpecies({"e-"})["e-"];

  std::vector<Real> logk_gold{
      23.4266, 21.50045, 19.277525, 17.24705, 15.238975, 13.64975, 12.34665, 11.27355};
  std::map<std::string, Real> bs_gold = {{"H2O", 0.5}, {"H+", -1}, {"O2(aq)", -0.25}};
  EXPECT_EQ(fe.charge, -1.0);
  EXPECT_EQ(fe.radius, 0.0);
  EXPECT_EQ(fe.molecular_weight, 0.0);
  EXPECT_EQ(fe.basis_species, bs_gold);
  EXPECT_EQ(fe.equilibrium_const, logk_gold);
}

/// Test that the free electron can be determined in terms of O2(g)
TEST(GeochemicalDatabaseReaderTest, freeElectronNoReexpress)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", false);

  auto fe = database.getEquilibriumSpecies({"e-"})["e-"];

  std::vector<Real> logk_gold{
      22.76135, 20.7757, 18.513025, 16.4658, 14.473225, 12.92125, 11.68165, 10.67105};
  std::map<std::string, Real> bs_gold = {{"H2O", 0.5}, {"H+", -1}, {"O2(g)", -0.25}};
  EXPECT_EQ(fe.charge, -1.0);
  EXPECT_EQ(fe.radius, 0.0);
  EXPECT_EQ(fe.molecular_weight, 0.0);
  EXPECT_EQ(fe.basis_species, bs_gold);
  EXPECT_EQ(fe.equilibrium_const, logk_gold);
}

/// Test the DatabaseReader when the secondary species that contain extrapolated logK are removed
TEST(GeochemicalDatabaseReaderTest, isSecondarySpecies_noextrap)
{
  GeochemicalDatabaseReader database("database/moose_testdb.json", true, false, true);

  EXPECT_FALSE(database.isSecondarySpecies("Ca++"));
  EXPECT_FALSE(database.isSecondarySpecies("H2O"));
  EXPECT_FALSE(database.isSecondarySpecies("Ag"));
  EXPECT_TRUE(database.isSecondarySpecies("CO2(aq)"));
  EXPECT_TRUE(database.isSecondarySpecies("CO3--"));
  EXPECT_FALSE(database.isSecondarySpecies("OH-"));
  EXPECT_FALSE(database.isSecondarySpecies("Calcite"));
  EXPECT_FALSE(database.isSecondarySpecies("Fe(OH)3(ppd)"));
  EXPECT_FALSE(database.isSecondarySpecies("CH4(g)"));
  EXPECT_FALSE(database.isSecondarySpecies("(O-phth)--"));
  EXPECT_FALSE(database.isSecondarySpecies("Fe+++"));
  EXPECT_FALSE(database.isSecondarySpecies("Cu2O"));
  EXPECT_FALSE(database.isSecondarySpecies(">(s)FeO-"));
  EXPECT_TRUE(database.isSecondarySpecies("e-"));
}
