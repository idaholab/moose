//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "Syntax.h"

class SyntaxTest : public ::testing::Test
{
protected:
  void SetUp()
  {
    _syntax.registerTaskName("first", false);
    _syntax.registerTaskName("second", true);

    // MOOSE object task
    _syntax.registerTaskName("first_mo", "MooseSystem1", false);
    _syntax.registerTaskName("second_mo", "MooseSystem2", true);

    _syntax.registerActionSyntax("SomeAction", "TopBlock");
    _syntax.registerActionSyntax("SomeAction", "TopBlock", "second");
  }

  Syntax _syntax;
};

TEST_F(SyntaxTest, errorChecks)
{
  try
  {
    _syntax.registerTaskName("first", "FakeSystem", true);
    FAIL() << "missing expected error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());

    EXPECT_NE(msg.find("is already registered"), std::string::npos)
        << "failed with unexpected error: " << msg;
  }

  try
  {
    _syntax.registerTaskName("second_mo", "FakeSystem", false);
    FAIL() << "missing expected error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());

    EXPECT_NE(msg.find("is already registered"), std::string::npos)
        << "failed with unexpected error: " << msg;
  }

  try
  {
    _syntax.appendTaskName("third", "MooseSystem");
    FAIL() << "missing expected error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());

    EXPECT_NE(msg.find("is not a registered task name"), std::string::npos)
        << "failed with unexpected error: " << msg;
  }

  try
  {
    _syntax.addDependency("forth", "third");
    FAIL() << "missing expected error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());

    EXPECT_NE(msg.find("is not a registered task name"), std::string::npos)
        << "failed with unexpected error: " << msg;
  }
}

TEST_F(SyntaxTest, general)
{
  EXPECT_TRUE(_syntax.hasTask("second"));
  EXPECT_FALSE(_syntax.hasTask("third"));

  EXPECT_FALSE(_syntax.shouldAutoBuild("first"));
  EXPECT_TRUE(_syntax.shouldAutoBuild("second_mo"));

  // TODO: test this
  _syntax.replaceActionSyntax("MooseSystem", "NewBlock", "first");
}

TEST_F(SyntaxTest, deprecated)
{
  _syntax.deprecateActionSyntax("TopBlock");

  EXPECT_TRUE(_syntax.isDeprecatedSyntax("TopBlock"));

  const std::string message = _syntax.deprecatedActionSyntaxMessage("TopBlock");
  EXPECT_EQ(message, "\"[TopBlock]\" is deprecated.");
}

TEST_F(SyntaxTest, deprecatedCustomMessage)
{
  _syntax.deprecateActionSyntax("TopBlock", "Replace [TopBlock] with [NewBlock].");

  EXPECT_TRUE(_syntax.isDeprecatedSyntax("TopBlock"));

  const std::string message = _syntax.deprecatedActionSyntaxMessage("TopBlock");
  EXPECT_EQ(message, "Replace [TopBlock] with [NewBlock].");
}

TEST(CyclicSyntaxTest, cyclic)
{
  Syntax syntax;
  syntax.registerTaskName("a");
  syntax.registerTaskName("b");
  syntax.addDependency("a", "b");
  syntax.addDependency("b", "a");
  try
  {
    syntax.getSortedTask();
    FAIL() << "missing expected error";
  }
  catch (std::runtime_error & e)
  {
    EXPECT_TRUE(std::string(e.what()).find("a <- b") != std::string::npos);
  }
}
