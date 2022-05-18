//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
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

TEST(ControllableItem, basic)
{
  InputParameters params = emptyInputParameters();
  params.addParam<int>("control", 1949, "A parameter to be controlled.");

  libMesh::Parameters::Value * value = get_value("control", params);
  MooseObjectParameterName name("System", "Object", "control");
  ControllableItem item(name, value);

  EXPECT_EQ(params.get<int>("control"), 1949);
  EXPECT_EQ(item.get<int>(), std::vector<int>(1, 1949));
  item.set<int>(1980);
  EXPECT_EQ(params.get<int>("control"), 1980);
  EXPECT_EQ(item.get<int>(), std::vector<int>(1, 1980));
  EXPECT_EQ(item.name(), name);
  EXPECT_EQ(item.type(), "int");
  EXPECT_TRUE(item.check<int>());
  EXPECT_FALSE(item.check<std::string>());
}

TEST(ControllableItem, multiple)
{
  InputParameters params0 = emptyInputParameters();
  params0.addParam<int>("control", 1949, "A parameter to be controlled.");
  MooseObjectParameterName name0("System", "Object0", "control");
  ControllableItem item0(name0, get_value("control", params0));

  InputParameters params1 = emptyInputParameters();
  params1.addParam<int>("control", 1954, "Another parameter to be controlled.");
  MooseObjectParameterName name1("System", "Object1", "control");
  ControllableItem item1(name1, get_value("control", params1));

  EXPECT_EQ(item0.get<int>().size(), 1);
  EXPECT_EQ(item0.get<int>()[0], 1949);

  item0.connect(&item1);

  EXPECT_EQ(item0.get<int>().size(), 2);
  EXPECT_EQ(params0.get<int>("control"), 1949);
  EXPECT_EQ(params1.get<int>("control"), 1954);
  EXPECT_EQ(item0.get<int>()[0], 1949);
  EXPECT_EQ(item0.get<int>()[1], 1954);
  item0.set<int>(1980);
  EXPECT_EQ(params0.get<int>("control"), 1980);
  EXPECT_EQ(params1.get<int>("control"), 1980);
  EXPECT_EQ(item0.get<int>().size(), 2);
  EXPECT_EQ(params0.get<int>("control"), 1980);
  EXPECT_EQ(params1.get<int>("control"), 1980);

  std::string out = item0.dump();
  EXPECT_NE(out.find("System/Object0/control"), std::string::npos);
  EXPECT_NE(out.find("System/Object1/control"), std::string::npos);
  EXPECT_NE(out.find("1980"), std::string::npos);
  EXPECT_EQ(out.find("1949"), std::string::npos);
  EXPECT_EQ(out.find("1954"), std::string::npos);
}

TEST(ControllableItem, multiple_types)
{
  InputParameters params = emptyInputParameters();
  params.addParam<int>("control", 1949, "");
  params.addParam<double>("control2", 1954, "");

  libMesh::Parameters::Value * value = get_value("control", params);
  MooseObjectParameterName name("System", "Object", "control");
  ControllableItem item(name, value);

  libMesh::Parameters::Value * value2 = get_value("control2", params);
  MooseObjectParameterName name2("System", "Object", "control2");
  ControllableItem item2(name2, value2);

  item.connect(&item2, false);
  EXPECT_EQ(params.get<int>("control"), 1949);
  EXPECT_EQ(params.get<double>("control2"), 1954);
  ASSERT_EQ(item.get<int>(false).size(), 1);
  EXPECT_EQ(item.get<int>(false)[0], 1949);
  ASSERT_EQ(item.get<double>(false).size(), 1);
  EXPECT_EQ(item.get<double>(false)[0], 1954);

  item.set<int>(2011, false);

  EXPECT_EQ(params.get<int>("control"), 2011);
  EXPECT_EQ(params.get<double>("control2"), 1954);
  ASSERT_EQ(item.get<int>(false).size(), 1);
  EXPECT_EQ(item.get<int>(false)[0], 2011);
  ASSERT_EQ(item.get<double>(false).size(), 1);
  EXPECT_EQ(item.get<double>(false)[0], 1954);

  item.set<double>(2013, false);

  EXPECT_EQ(params.get<int>("control"), 2011);
  EXPECT_EQ(params.get<double>("control2"), 2013);
  ASSERT_EQ(item.get<int>(false).size(), 1);
  EXPECT_EQ(item.get<int>(false)[0], 2011);
  ASSERT_EQ(item.get<double>(false).size(), 1);
  EXPECT_EQ(item.get<double>(false)[0], 2013);

  EXPECT_FALSE(item.check<int>());
  EXPECT_FALSE(item.check<double>());
}

TEST(ControllableItem, errors)
{
  InputParameters params = emptyInputParameters();
  params.addParam<int>("control", 1949, "A parameter to be controlled.");
  params.addParam<double>("control2", 1.2345, "Another parameter to be controlled.");

  libMesh::Parameters::Value * value = get_value("control", params);
  MooseObjectParameterName name("System", "Object", "control");
  ControllableItem item(name, value);

  try
  {
    item.set<double>(1980);
    FAIL() << "Missing the expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_NE(msg.find("Failed to set the 'System/Object/control"), std::string::npos);
  }

  try
  {
    item.get<double>();
    FAIL() << "Missing the expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_NE(msg.find("Failed to get the 'System/Object/control"), std::string::npos);
  }

  libMesh::Parameters::Value * value2 = get_value("control2", params);
  MooseObjectParameterName name2("System", "Object", "control2");
  ControllableItem item2(name2, value2);

  try
  {
    item.connect(&item2);
    FAIL() << "Missing the expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_NE(msg.find("The master parameter (System/Object/control"), std::string::npos);
  }
}

TEST(ControllableAlias, basic)
{
  InputParameters params = emptyInputParameters();
  params.addParam<int>("control", 1949, "A parameter to be controlled.");

  libMesh::Parameters::Value * value = get_value("control", params);
  MooseObjectParameterName name("System", "Object", "control");
  ControllableItem item(name, value);

  MooseObjectParameterName alias_name("Not", "An", "Param");
  ControllableAlias alias(alias_name, &item);

  EXPECT_EQ(params.get<int>("control"), 1949);
  EXPECT_EQ(item.get<int>().size(), 1);
  EXPECT_EQ(item.get<int>()[0], 1949);
  EXPECT_EQ(alias.get<int>().size(), 1);
  EXPECT_EQ(alias.get<int>()[0], 1949);
  alias.set<int>(1980);
  EXPECT_EQ(params.get<int>("control"), 1980);
  EXPECT_EQ(item.get<int>().size(), 1);
  EXPECT_EQ(item.get<int>()[0], 1980);
  EXPECT_EQ(alias.get<int>().size(), 1);
  EXPECT_EQ(alias.get<int>()[0], 1980);

  std::string out = alias.dump();
  EXPECT_NE(out.find("System/Object/control"), std::string::npos);
  EXPECT_NE(out.find("1980"), std::string::npos);
  EXPECT_EQ(out.find("1949"), std::string::npos);
}
