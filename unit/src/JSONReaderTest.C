//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "JSONFileReader.h"
#include "AppFactory.h"
#include "Executioner.h"

TEST(JSONFileReader, errors)
{
  // Create a minimal app that can create objects
  const char * argv[2] = {"foo", "\0"};
  const auto & app = AppFactory::createAppShared("MooseUnitApp", 1, (char **)argv);
  const auto & factory = &app->getFactory();
  app->parameters().set<bool>("minimal") = true;
  app->run();
  Executioner * exec = app->getExecutioner();
  FEProblemBase * fe_problem = &exec->feProblem();

  // JSONFileReader is uninitialized at construction, any other JSONFileReader with such
  // behavior would do
  InputParameters params = factory->getValidParams("JSONFileReader");
  params.set<FEProblemBase *>("_fe_problem_base") = fe_problem;
  params.set<SubProblem *>("_subproblem") = fe_problem;
  params.set<std::string>("_object_name") = "test";
  params.set<std::string>("_type") = "JSONFileReader";
  params.set<FileName>("filename") = "data/json/function_values.json";
  JSONFileReader JSONFileReader(params);

  // Scalar getters
  try
  {
    Real a;
    JSONFileReader.getScalar("not_a_key", a);
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
    JSONFileReader.getScalar(std::vector<std::string>(), a);
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
    JSONFileReader.getVector("not_a_key", a);
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
    JSONFileReader.getVector(std::vector<std::string>(), a);
    FAIL() << "Missing the expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_NE(msg.find("There should be at least one key to retrieve a value from the JSON"),
              std::string::npos);
  }
}

TEST(JSONFileReader, getters)
{
  // Create a minimal app that can create objects
  const char * argv[2] = {"foo", "\0"};
  const auto & app = AppFactory::createAppShared("MooseUnitApp", 1, (char **)argv);
  const auto & factory = &app->getFactory();
  app->parameters().set<bool>("minimal") = true;
  app->run();
  Executioner * exec = app->getExecutioner();
  FEProblemBase * fe_problem = &exec->feProblem();

  InputParameters params = factory->getValidParams("JSONFileReader");
  params.set<FEProblemBase *>("_fe_problem_base") = fe_problem;
  params.set<SubProblem *>("_subproblem") = fe_problem;
  params.set<std::string>("_object_name") = "test";
  params.set<std::string>("_type") = "JSONFileReader";
  params.set<FileName>("filename") = "data/json/function_values.json";
  JSONFileReader JSONFileReader(params);

  // Test scalar getters
  Real from_json;
  JSONFileReader.getScalar("direct_key", from_json);
  EXPECT_EQ(from_json, 3);
  JSONFileReader.getScalar(std::vector<std::string>({"the_data", "random_other_key"}), from_json);
  EXPECT_EQ(from_json, 2);

  // Test vector getters
  std::vector<Real> from_json_vec;
  JSONFileReader.getVector("direct_vector_key", from_json_vec);
  EXPECT_EQ(from_json_vec[2], 2);
  JSONFileReader.getVector(std::vector<std::string>({"the_data", "some_key", "some_other_key"}),
                           from_json_vec);
  EXPECT_EQ(from_json_vec[2], 7);
}
