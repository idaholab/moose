//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "MooseObjectParameterName.h"

TEST(MooseObjectParameterName, methods)
{
  MooseObjectParameterName name1("Materials", "wood", "density");
  EXPECT_EQ("Materials", name1.tag());
  EXPECT_EQ("wood", name1.name());
  EXPECT_EQ("density", name1.parameter());

  MooseObjectParameterName name2("Materials::wood/density");
  EXPECT_EQ("Materials", name2.tag());
  EXPECT_EQ("wood", name2.name());
  EXPECT_EQ("density", name2.parameter());

  MooseObjectParameterName name3("Materials/wood/density");
  EXPECT_EQ("Materials", name3.tag());
  EXPECT_EQ("wood", name3.name());
  EXPECT_EQ("density", name3.parameter());

  MooseObjectParameterName name4("Materials/wood/oak/density");
  EXPECT_EQ("Materials/wood", name4.tag());
  EXPECT_EQ("oak", name4.name());
  EXPECT_EQ("density", name4.parameter());
}

TEST(MooseObjectParameterName, operators)
{
  // operator==
  MooseObjectParameterName name1("Materials", "wood", "walnut");
  MooseObjectParameterName name2("Materials::wood/walnut");
  MooseObjectParameterName name3("Materials/wood/walnut");
  EXPECT_EQ(name1, name2);
  EXPECT_EQ(name1, name3);
  EXPECT_EQ(name2, name3);

  // operator!=
  MooseObjectParameterName name4("Fuctions/wood/walnut");
  EXPECT_NE(name4, name1);
  EXPECT_NE(name4, name2);
  EXPECT_NE(name4, name3);

  MooseObjectParameterName name5("Materials::steel/walnut");
  EXPECT_NE(name5, name1);
  EXPECT_NE(name5, name2);
  EXPECT_NE(name5, name3);

  MooseObjectParameterName name6("Materials", "wood", "oak");
  EXPECT_NE(name6, name1);
  EXPECT_NE(name6, name2);
  EXPECT_NE(name6, name3);

  MooseObjectParameterName name7("Functions", "wood", "walnut");
  EXPECT_NE(name7, name1);
  EXPECT_NE(name7, name2);
  EXPECT_NE(name7, name3);

  // operator<
  EXPECT_LT(name4, name1);
  EXPECT_LT(name4, name2);
  EXPECT_LT(name4, name3);

  EXPECT_LT(name5, name1);
  EXPECT_LT(name5, name2);
  EXPECT_LT(name5, name3);

  EXPECT_LT(name6, name1);
  EXPECT_LT(name6, name2);
  EXPECT_LT(name6, name3);
}

TEST(MooseObjectParameterName, find)
{
  // Tests that map look up is working, which uses the operator<
  MooseObjectParameterName name1("Materials", "wood", "walnut");
  MooseObjectParameterName name2("Materials/wood/oak");
  MooseObjectParameterName name3("Materials::wood/teak");
  MooseObjectParameterName name4("Materials", "wood", "mahogany");
  MooseObjectParameterName name5("Functions/add/number");
  MooseObjectParameterName name6("Functions::subtract/number");

  std::map<MooseObjectParameterName, int> data;
  data[name1] = 1;
  data[name2] = 2;
  data[name3] = 3;
  data[name4] = 4;
  data[name5] = 5;
  data[name6] = 6;

  std::map<MooseObjectParameterName, int>::const_iterator iter;
  iter = data.find(MooseObjectParameterName("Materials::wood/walnut"));
  EXPECT_EQ(iter->second, 1);

  iter = data.find(MooseObjectParameterName("Materials", "wood", "oak"));
  EXPECT_EQ(iter->second, 2);

  iter = data.find(MooseObjectParameterName("Materials/wood/teak"));
  EXPECT_EQ(iter->second, 3);

  iter = data.find(MooseObjectParameterName("Materials/wood/mahogany"));
  EXPECT_EQ(iter->second, 4);

  iter = data.find(MooseObjectParameterName("Functions::add/number"));
  EXPECT_EQ(iter->second, 5);

  iter = data.find(MooseObjectParameterName("Functions", "subtract", "number"));
  EXPECT_EQ(iter->second, 6);
}

TEST(MooseObjectParameterName, askerisk)
{
  MooseObjectParameterName name1("Materials", "wood", "walnut");
  MooseObjectParameterName name2("Materials", "wood", "*");
  MooseObjectParameterName name3("Materials", "*", "walnut");
  MooseObjectParameterName name4("*", "wood", "walnut");
  MooseObjectParameterName name5("Materials", "*", "*");
  MooseObjectParameterName name6("*", "wood", "*");
  MooseObjectParameterName name7("*", "*", "walnut");

  EXPECT_EQ(name1, name2);
  EXPECT_EQ(name1, name3);
  EXPECT_EQ(name1, name4);
  EXPECT_EQ(name1, name5);
  EXPECT_EQ(name1, name6);
  EXPECT_EQ(name1, name7);

  EXPECT_EQ(name2, name3);
  EXPECT_EQ(name2, name4);
  EXPECT_EQ(name2, name5);
  EXPECT_EQ(name2, name6);
  EXPECT_EQ(name2, name7);

  EXPECT_EQ(name3, name4);
  EXPECT_EQ(name3, name5);
  EXPECT_EQ(name3, name6);
  EXPECT_EQ(name3, name7);

  EXPECT_EQ(name4, name5);
  EXPECT_EQ(name4, name6);
  EXPECT_EQ(name4, name7);

  EXPECT_EQ(name5, name6);
  EXPECT_EQ(name5, name7);

  EXPECT_EQ(name6, name7);
}

TEST(MooseObjectParameterName, errors)
{
  try
  {
    MooseObjectParameterName name("foo");
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_TRUE(msg.find("The supplied tag cannot be empty") != std::string::npos)
        << "MooseObjectParameterName failed with unexpected error: " << msg;
  }

  try
  {
    MooseObjectParameterName name("foo", "bar", "");
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_TRUE(msg.find("The supplied parameter name cannot be empty") != std::string::npos)
        << "MooseObjectParameterName failed with unexpected error: " << msg;
  }
}

TEST(MooseObjectParameterName, copy)
{
  MooseObjectParameterName name0("tag", "object", "param");
  MooseObjectParameterName name1(name0);
  EXPECT_EQ(name0, name1);
}
