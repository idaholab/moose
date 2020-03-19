//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "MooseObjectName.h"

TEST(MooseObjectName, methods)
{
  MooseObjectName name1("Materials", "wood");
  EXPECT_EQ("Materials", name1.tag());
  EXPECT_EQ("wood", name1.name());

  MooseObjectName name2("Materials::wood");
  EXPECT_EQ("Materials", name2.tag());
  EXPECT_EQ("wood", name2.name());

  MooseObjectName name3("Materials/wood");
  EXPECT_EQ("Materials", name3.tag());
  EXPECT_EQ("wood", name3.name());

  MooseObjectName name4("Materials/wood/oak");
  EXPECT_EQ("Materials/wood", name4.tag());
  EXPECT_EQ("oak", name4.name());
}

TEST(MooseObjectName, operators)
{
  // operator==
  MooseObjectName name1("Materials", "wood");
  MooseObjectName name2("Materials::wood");
  MooseObjectName name3("Materials/wood");
  EXPECT_EQ(name1, name2);
  EXPECT_EQ(name1, name3);
  EXPECT_EQ(name2, name3);

  // operator!=
  MooseObjectName name4("Materials/steel");
  EXPECT_NE(name4, name1);
  EXPECT_NE(name4, name2);
  EXPECT_NE(name4, name3);

  MooseObjectName name5("Materials::steel");
  EXPECT_NE(name5, name1);
  EXPECT_NE(name5, name2);
  EXPECT_NE(name5, name3);

  MooseObjectName name6("Materials", "steel");
  EXPECT_NE(name6, name1);
  EXPECT_NE(name6, name2);
  EXPECT_NE(name6, name3);

  MooseObjectName name7("Functions", "steel");
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

TEST(MooseObjectName, find)
{
  // Tests that map look up is working, which uses the operator<
  MooseObjectName name1("Materials", "walnut");
  MooseObjectName name2("Materials/oak");
  MooseObjectName name3("Materials::teak");
  MooseObjectName name4("Materials", "mahogany");
  MooseObjectName name5("Functions/add");
  MooseObjectName name6("Functions::subtract");

  std::map<MooseObjectName, int> data;
  data[name1] = 1;
  data[name2] = 2;
  data[name3] = 3;
  data[name4] = 4;
  data[name5] = 5;
  data[name6] = 6;

  std::map<MooseObjectName, int>::const_iterator iter;
  iter = data.find(MooseObjectName("Materials::walnut"));
  EXPECT_EQ(iter->second, 1);

  iter = data.find(MooseObjectName("Materials", "oak"));
  EXPECT_EQ(iter->second, 2);

  iter = data.find(MooseObjectName("Materials/teak"));
  EXPECT_EQ(iter->second, 3);

  iter = data.find(MooseObjectName("Materials/mahogany"));
  EXPECT_EQ(iter->second, 4);

  iter = data.find(MooseObjectName("Functions::add"));
  EXPECT_EQ(iter->second, 5);

  iter = data.find(MooseObjectName("Functions", "subtract"));
  EXPECT_EQ(iter->second, 6);
}

TEST(MooseObjectName, askerisk)
{
  MooseObjectName name1("Materials", "walnut");
  MooseObjectName name2("Materials", "*");
  MooseObjectName name3("*", "walnut");

  EXPECT_EQ(name1, name2);
  EXPECT_EQ(name1, name3);
  EXPECT_EQ(name2, name3);
}

TEST(MooseObjectName, errors)
{
  try
  {
    MooseObjectName name("foo");
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_TRUE(msg.find("The supplied tag cannot be empty") != std::string::npos)
        << "MooseObjectName failed with unexpected error: " << msg;
  }

  try
  {
    MooseObjectName name("", "foo");
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_TRUE(msg.find("The supplied tag cannot be empty") != std::string::npos)
        << "MooseObjectName failed with unexpected error: " << msg;
  }

  try
  {
    MooseObjectName name("foo", "");
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_TRUE(msg.find("The supplied name cannot be empty") != std::string::npos)
        << "MooseObjectName failed with unexpected error: " << msg;
  }
}
