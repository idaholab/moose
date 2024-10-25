//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "JSONReaderTest.h"
#include "JSONFileReader.h"

TEST_F(JSONReaderTest, errors)
{
  // JSONFileReader is uninitialized at construction, any other JSONFileReader with such
  // behavior would do
  InputParameters params = _factory.getValidParams("JSONFileReader");
  params.set<std::string>("_type") = "JSONFileReader";
  params.set<FileName>("filename") = "data/json/function_values.json";
  auto & reader = addObject<JSONFileReader>("JSONFileReader", "test", params);

  // single value getters
  try
  {
    Real a;
    reader.getValue<Real>("not_a_key", a);
    FAIL() << "Missing the expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_NE(
        msg.find("Attempted to get \'not_a_key\' but the JSON file does not contain this key"),
        std::string::npos);
  }
  try
  {
    Real a;
    reader.getValue<Real>(std::vector<std::string>(), a);
    FAIL() << "Missing the expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_NE(msg.find("There should be at least one key to retrieve a value from the JSON"),
              std::string::npos);
  }

  // Vector getters
  try
  {
    std::vector<Real> a;
    reader.getVector<Real>("not_a_key", a);
    FAIL() << "Missing the expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_NE(msg.find("Attempted to get 'not_a_key' but the JSON file does not contain this key"),
              std::string::npos);
  }
  try
  {
    std::vector<Real> a;
    reader.getVector<Real>(std::vector<std::string>(), a);
    FAIL() << "Missing the expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_NE(msg.find("There should be at least one key to retrieve a value from the JSON"),
              std::string::npos);
  }
}

TEST_F(JSONReaderTest, getters)
{
  InputParameters params = _factory.getValidParams("JSONFileReader");
  params.set<FileName>("filename") = "data/json/function_values.json";
  auto & reader = addObject<JSONFileReader>("JSONFileReader", "test", params);

  // Test single value getters
  Real from_json;
  reader.getValue<Real>("direct_key", from_json);
  EXPECT_EQ(from_json, 3);
  reader.getValue<Real>(std::vector<std::string>({"the_data", "random_other_key"}), from_json);
  EXPECT_EQ(from_json, 2);

  // Test vector getters
  std::vector<Real> from_json_vec;
  reader.getVector<Real>("direct_vector_key", from_json_vec);
  EXPECT_EQ(from_json_vec[2], 2);
  reader.getVector<Real>(std::vector<std::string>({"the_data", "some_key", "some_other_key"}),
                         from_json_vec);
  EXPECT_EQ(from_json_vec[2], 7);
}
