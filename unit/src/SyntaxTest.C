/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
    _syntax.registerTaskName("first", true);
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
    _syntax.registerTaskName("second_mo", false);
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

  EXPECT_FALSE(_syntax.isActionRequired("first"));
  EXPECT_TRUE(_syntax.isActionRequired("second_mo"));

  // TODO: test this
  _syntax.replaceActionSyntax("MooseSystem", "NewBlock", "first");
}

TEST_F(SyntaxTest, deprecated)
{
  _syntax.deprecateActionSyntax("TopBlock");

  EXPECT_TRUE(_syntax.isDeprecatedSyntax("TopBlock"));
}
