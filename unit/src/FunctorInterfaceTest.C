//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "FunctorInterface.h"
#include "InputParameters.h"
#include "MooseTypes.h"

TEST(FunctorInterfaceTest, deduceFunctorName)
{
  const auto base_params = emptyInputParameters();
  {
    auto params = base_params;
    params.set<MooseFunctorName>("test") = "functor";
    EXPECT_EQ(FunctorInterface::deduceFunctorName("test", params), "functor");
  }
  {
    auto params = base_params;
    params.set<MaterialPropertyName>("test") = "prop";
    EXPECT_EQ(FunctorInterface::deduceFunctorName("test", params), "prop");
  }
  {
    auto params = base_params;
    params.set<VariableName>("test") = "var";
    EXPECT_EQ(FunctorInterface::deduceFunctorName("test", params), "var");
  }
  {
    auto params = base_params;
    params.set<std::vector<VariableName>>("test") = {"vector_var"};
    EXPECT_EQ(FunctorInterface::deduceFunctorName("test", params), "vector_var");
  }
  {
    auto params = base_params;
    params.set<std::vector<VariableName>>("test") = {"vector_var", "bad_vector_var"};
    try
    {
      FunctorInterface::deduceFunctorName("test", params);
      ASSERT_TRUE(false);
    }
    catch (std::runtime_error & e)
    {
      ASSERT_TRUE(std::string(e.what()).find("single variable name") != std::string::npos);
    }
  }
  {
    auto params = base_params;
    params.set<NonlinearVariableName>("test") = "nl_var";
    EXPECT_EQ(FunctorInterface::deduceFunctorName("test", params), "nl_var");
  }
  {
    auto params = base_params;
    params.set<FunctionName>("test") = "function";
    EXPECT_EQ(FunctorInterface::deduceFunctorName("test", params), "function");
  }
  {
    auto params = base_params;
    params.set<PostprocessorName>("test") = "pp";
    try
    {
      FunctorInterface::deduceFunctorName("test", params);
      ASSERT_TRUE(false);
    }
    catch (std::runtime_error & e)
    {
      ASSERT_TRUE(std::string(e.what()).find("Invalid parameter type for retrieving a functor") !=
                  std::string::npos);
    }
  }
  {
    auto params = base_params;
    EXPECT_EQ(FunctorInterface::deduceFunctorName("test", params), "test");
  }
}
