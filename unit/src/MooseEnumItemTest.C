//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MooseEnumItem.h"
#include "MooseException.h"

TEST(MooseEnumItem, construction)
{
  MooseEnumItem item("LuggageCombo  ", 12345);
  EXPECT_EQ(item.name(), "LUGGAGECOMBO");
  EXPECT_EQ(item.id(), 12345);
  EXPECT_EQ(item.rawName(), "LuggageCombo");
}

TEST(MooseEnumItem, int_and_string_operators)
{
  MooseEnumItem item("Andrew", 1980);
  std::string name(item);
  int id(item);
  EXPECT_EQ(name, "ANDREW");
  EXPECT_EQ(id, 1980);
}

TEST(MooseEnumItem, comparison)
{
  MooseEnumItem item("Edward", 1949);
  EXPECT_EQ(item, "EDWARD");
  EXPECT_EQ(item, "Edward");

  EXPECT_EQ(item, std::string("EDWARD"));
  EXPECT_EQ(item, std::string("Edward"));

  EXPECT_EQ(item, 1949);
  EXPECT_NE(item, 1989);
}

TEST(MooseEnumItem, assignment_and_copy)
{
  MooseEnumItem item("Andrew", 1980);
  MooseEnumItem item2("Edward", 1949);
  item = item2;
  EXPECT_EQ(item.name(), "EDWARD");
  EXPECT_EQ(item.id(), 1949);
  EXPECT_EQ(item.rawName(), "Edward");

  MooseEnumItem item3(item2);
  EXPECT_EQ(item3.name(), "EDWARD");
  EXPECT_EQ(item3.id(), 1949);
  EXPECT_EQ(item3.rawName(), "Edward");
}

TEST(MooseEnumItem, lessthan)
{
  MooseEnumItem item1("Andrew", 1980);
  MooseEnumItem item2("Deanne", 1980);
  MooseEnumItem item3("Isaac", 2011);
  MooseEnumItem item4("Allison", 2013);

  std::set<MooseEnumItem> items;
  items.insert(item4);
  items.insert(item3);
  items.insert(item2);
  items.insert(item1);
  EXPECT_NE(items.find(item1), items.end());
  EXPECT_NE(items.find(item2), items.end());
  EXPECT_NE(items.find(item3), items.end());
  EXPECT_NE(items.find(item4), items.end());
  EXPECT_EQ(items.find(MooseEnumItem("Edward", 1949)), items.end());
}

TEST(MooseEnumItem, setID)
{
  MooseEnumItem item("LuggageCombo");
  EXPECT_EQ(item.id(), MooseEnumItem::INVALID_ID);

  item.setID(12345);
  EXPECT_EQ(item.id(), 12345);

  try
  {
    item.setID(54321);
    FAIL() << "missing expected error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("The ID of a MooseEnumItem can not be changed if it is valid, the item "
                       "LUGGAGECOMBO has a valid id of 12345."),
              std::string::npos)
        << "failed with unexpected error: " << msg;
  }
}
