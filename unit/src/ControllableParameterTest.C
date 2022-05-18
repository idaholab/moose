//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "ControllableParameter.h"
#include "ControllableItem.h"
#include "InputParameters.h"

namespace
{
libMesh::Parameters::Value *
get_value(const std::string & name, const InputParameters & params)
{
  for (libMesh::Parameters::const_iterator map_iter = params.begin(); map_iter != params.end();
       ++map_iter)
    if (name == map_iter->first)
      return MooseUtils::get(map_iter->second);
  return nullptr;
}
}

TEST(ControllableParameter, basic)
{
  InputParameters params = emptyInputParameters();
  params.addParam<int>("control", 1949, "A parameter to be controlled.");

  libMesh::Parameters::Value * value = get_value("control", params);
  MooseObjectParameterName name("System", "Object", "control");
  ControllableItem item(name, value);
  ControllableParameter param;
  EXPECT_FALSE(param.check<int>());
  param.add(&item);
  EXPECT_TRUE(param.check<int>());

  EXPECT_EQ(params.get<int>("control"), 1949);
  EXPECT_EQ(param.get<int>(), std::vector<int>(1, 1949));
  param.set<int>(1980);
  EXPECT_EQ(params.get<int>("control"), 1980);
  EXPECT_EQ(param.get<int>(), std::vector<int>(1, 1980));
}

TEST(ControllableParameter, multiple)
{
  InputParameters params = emptyInputParameters();
  params.addParam<int>("control", 1949, "A parameter to be controlled.");
  params.addParam<int>("control2", 1954, "Another parameter to be controlled.");

  libMesh::Parameters::Value * value = get_value("control", params);
  MooseObjectParameterName name("System", "Object", "control");
  ControllableItem item(name, value);

  libMesh::Parameters::Value * value2 = get_value("control2", params);
  MooseObjectParameterName name2("System", "Object", "control2");
  ControllableItem item2(name2, value2);

  ControllableParameter param;
  EXPECT_FALSE(param.check<int>());
  param.add(&item);
  param.add(&item2);
  EXPECT_TRUE(param.check<int>());

  EXPECT_EQ(params.get<int>("control"), 1949);
  EXPECT_EQ(params.get<int>("control2"), 1954);
  ASSERT_EQ(param.get<int>().size(), 2);
  EXPECT_EQ(param.get<int>()[0], 1949);
  EXPECT_EQ(param.get<int>()[1], 1954);
  param.set<int>(1980);
  EXPECT_EQ(params.get<int>("control"), 1980);
  EXPECT_EQ(params.get<int>("control2"), 1980);
  EXPECT_EQ(param.get<int>(), std::vector<int>(2, 1980));
}

TEST(ControllableParameter, multiple_types)
{
  InputParameters params = emptyInputParameters();
  params.addParam<int>("control", 1949, "A parameter to be controlled.");
  params.addParam<double>("control2", 1954, "Another parameter to be controlled.");

  libMesh::Parameters::Value * value = get_value("control", params);
  MooseObjectParameterName name("System", "Object", "control");
  ControllableItem item(name, value);

  libMesh::Parameters::Value * value2 = get_value("control2", params);
  MooseObjectParameterName name2("System", "Object", "control2");
  ControllableItem item2(name2, value2);

  ControllableParameter param;
  EXPECT_FALSE(param.check<std::string>());

  param.add(&item);
  param.add(&item2);

  EXPECT_EQ(params.get<int>("control"), 1949);
  EXPECT_EQ(params.get<double>("control2"), 1954);

  ASSERT_EQ(param.get<int>(false).size(), 1);
  EXPECT_EQ(param.get<int>(false)[0], 1949);
  ASSERT_EQ(param.get<double>(false).size(), 1);
  EXPECT_EQ(param.get<double>(false)[0], 1954);

  param.set<int>(1980, false);
  EXPECT_EQ(params.get<int>("control"), 1980);
  EXPECT_EQ(params.get<double>("control2"), 1954);
  ASSERT_EQ(param.get<int>(false).size(), 1);
  EXPECT_EQ(param.get<int>(false)[0], 1980);

  param.set<double>(1980, false);
  EXPECT_EQ(params.get<int>("control"), 1980);
  EXPECT_EQ(params.get<double>("control2"), 1980);
  ASSERT_EQ(param.get<double>(false).size(), 1);
  EXPECT_EQ(param.get<double>(false)[0], 1980);

  EXPECT_FALSE(param.check<int>());
  EXPECT_FALSE(param.check<double>());
}

TEST(ControllableParameter, errors)
{
  InputParameters params = emptyInputParameters();
  params.addParam<int>("control", 1949, "A parameter to be controlled.");
  params.addParam<double>("control2", 1.2345, "Another parameter to be controlled.");

  libMesh::Parameters::Value * value = get_value("control", params);
  MooseObjectParameterName name("System", "Object", "control");
  ControllableItem item(name, value);

  ControllableParameter param;
  param.add(&item);

  try
  {
    param.set<double>(1980.);
    FAIL() << "Missing the expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_NE(msg.find("Failed to set the 'System/Object/control"), std::string::npos);
  }

  try
  {
    param.get<double>();
    FAIL() << "Missing the expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_NE(msg.find("Failed to get the 'System/Object/control"), std::string::npos);
  }
}

TEST(ControllableParameter, dump)
{
  InputParameters params = emptyInputParameters();
  params.addParam<int>("control", 1949, "A parameter to be controlled.");
  params.addParam<int>("control2", 1954, "Another parameter to be controlled.");

  libMesh::Parameters::Value * value = get_value("control", params);
  MooseObjectParameterName name("System", "Object", "control");
  ControllableItem item(name, value);

  libMesh::Parameters::Value * value2 = get_value("control2", params);
  MooseObjectParameterName name2("System", "Object", "control2");
  ControllableItem item2(name2, value2);

  ControllableParameter param;
  param.add(&item);
  param.add(&item2);

  std::string out = param.dump();
  EXPECT_NE(out.find("System/Object/control"), std::string::npos);
  EXPECT_NE(out.find("System/Object/control2"), std::string::npos);
  EXPECT_NE(out.find("1949"), std::string::npos);
  EXPECT_NE(out.find("1954"), std::string::npos);
  EXPECT_NE(out.find("<int>"), std::string::npos);
}

TEST(ControllableParameter, warn_when_values_differ)
{
  InputParameters params = emptyInputParameters();
  params.addParam<int>("control", 1949, "A parameter to be controlled.");
  params.addParam<int>("control2", 1954, "Another parameter to be controlled.");

  libMesh::Parameters::Value * value = get_value("control", params);
  MooseObjectParameterName name("System", "Object", "control");
  ControllableItem item(name, value);

  libMesh::Parameters::Value * value2 = get_value("control2", params);
  MooseObjectParameterName name2("System", "Object", "control2");
  ControllableItem item2(name2, value2);
  item.connect(&item2);

  ControllableParameter param;
  param.add(&item);
  param.add(&item2);

  try
  {
    param.get<int>(true, true);
    FAIL() << "Missing the expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_NE(msg.find("The following controlled"), std::string::npos);
  }
}
