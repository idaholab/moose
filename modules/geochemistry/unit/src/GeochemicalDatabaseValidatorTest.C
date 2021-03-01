//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include <fstream>

#include "GeochemicalDatabaseValidator.h"

void
testExceptionMessage(const FileName filename, const std::string msg)
{
  // Read the faulty JSON database
  std::ifstream jsondata(filename);
  nlohmann::json root;
  jsondata >> root;

  GeochemicalDatabaseValidator database(filename, root);

  // Check that validating throws the expected error message
  try
  {
    database.validate();
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what()).find(msg);
    ASSERT_TRUE(pos != std::string::npos);
  }
}

TEST(GeochemicalDatabaseValidatorTest, validateTestDB)
{
  // Read the JSON database
  FileName filename = "database/moose_testdb.json";
  std::ifstream jsondata(filename);
  nlohmann::json root;
  jsondata >> root;

  // This database should have valid entries for all species and sections,
  // so shouldn't throw any exceptions
  GeochemicalDatabaseValidator database(filename, root);
  EXPECT_NO_THROW(database.validate());
}

TEST(GeochemicalDatabaseValidatorTest, missingHeader)
{
  const FileName filename = "database/faultydbs/missing_header.json";
  const std::string msg =
      "The MOOSE database " + filename + " does not have a required \"Header\" field";
  testExceptionMessage(filename, msg);
}

TEST(GeochemicalDatabaseValidatorTest, missingTemperature)
{
  const FileName filename = "database/faultydbs/missing_temperature.json";
  const std::string msg =
      "The MOOSE database " + filename + " does not have a required \"Header:temperatures\" field";
  testExceptionMessage(filename, msg);
}

TEST(GeochemicalDatabaseValidatorTest, missingActivityModel)
{
  const FileName filename = "database/faultydbs/missing_activity.json";
  const std::string msg = "The MOOSE database " + filename +
                          " does not have a required \"Header:activity model\" field";
  testExceptionMessage(filename, msg);
}

TEST(GeochemicalDatabaseValidatorTest, missingLogKValue)
{
  const FileName filename = "database/faultydbs/missing_value.json";
  const std::string msg =
      "The number of values in the logk field of secondary species CO2(aq) in " + filename +
      " is not equal to the number of temperature values";
  testExceptionMessage(filename, msg);
}

TEST(GeochemicalDatabaseValidatorTest, nonRealValue)
{
  const FileName filename = "database/faultydbs/nonreal_value.json";
  const std::string msg = "radius value \"4.5x\" of the secondary species CO3-- in "
                          "database/faultydbs/nonreal_value.json cannot be converted to Real";
  testExceptionMessage(filename, msg);
}

TEST(GeochemicalDatabaseValidatorTest, nonReaArraylValue)
{
  const FileName filename = "database/faultydbs/nonreal_arrayvalue.json";
  const std::string msg = "Array value \"abcd\" in the logk field of mineral species Calcite in "
                          "database/faultydbs/nonreal_arrayvalue.json cannot be converted to Real";
  testExceptionMessage(filename, msg);
}

TEST(GeochemicalDatabaseValidatorTest, nonReaWeight)
{
  const FileName filename = "database/faultydbs/nonreal_weight.json";
  const std::string msg =
      "Weight value \"1.xyz\" of constituent HCO3- of the secondary species CO3-- in "
      "database/faultydbs/nonreal_weight.json cannot be converted to Real";
  testExceptionMessage(filename, msg);
}

TEST(GeochemicalDatabaseValidatorTest, nonReaNeutralSpeciesArray)
{
  const FileName filename = "database/faultydbs/nonreal_neutralspecies.json";
  const std::string msg = "Array value \".1967cg\" in the Header:neutral species:co2:a field of " +
                          filename + " cannot be converted to Real";
  testExceptionMessage(filename, msg);
}

TEST(GeochemicalDatabaseValidatorTest, missingNeutralValue)
{
  const FileName filename = "database/faultydbs/missing_neutral_value.json";
  const std::string msg = "The number of values in the Header:neutral species:h2o:a field of " +
                          filename + " is not equal to the number of temperature values";
  testExceptionMessage(filename, msg);
}

TEST(GeochemicalDatabaseValidatorTest, noCharge)
{
  const FileName filename = "database/faultydbs/no_charge.json";
  const std::string msg = "The secondary species CO3-- in " + filename + " does not have a charge";
  testExceptionMessage(filename, msg);
}

TEST(GeochemicalDatabaseValidatorTest, noStoichiometry)
{
  const FileName filename = "database/faultydbs/no_stoi.json";
  const std::string msg = "The secondary species CO3-- in " + filename + " does not have a species";
  testExceptionMessage(filename, msg);
}
