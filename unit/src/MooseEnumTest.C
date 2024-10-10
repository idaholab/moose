//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MooseEnum.h"
#include "MultiMooseEnum.h"
#include "ExecFlagEnum.h"
#include "MooseUtils.h"

#include <algorithm> // std::set_symmetric_difference

TEST(MooseEnum, multiTestOne)
{
  MultiMooseEnum mme("one two three four", "two");

  EXPECT_EQ(mme.contains("one"), false);
  EXPECT_EQ(mme.contains("two"), true);
  EXPECT_EQ(mme.contains("three"), false);
  EXPECT_EQ(mme.contains("four"), false);

  mme.setAdditionalValue("four");
  EXPECT_EQ(mme.contains("one"), false);
  EXPECT_EQ(mme.contains("two"), true);
  EXPECT_EQ(mme.contains("three"), false);
  EXPECT_EQ(mme.contains("four"), true);

  // isValid
  EXPECT_EQ(mme.isValid(), true);

  mme.clearSetValues();
  EXPECT_EQ(mme.isValid(), false);

  mme.setAdditionalValue("one three");
  EXPECT_EQ(mme.contains("one"), true);
  EXPECT_EQ(mme.contains("two"), false);
  EXPECT_EQ(mme.contains("three"), true);
  EXPECT_EQ(mme.contains("four"), false);

  std::vector<std::string> mvec(2);
  mvec[0] = "one";
  mvec[1] = "two";

  std::set<std::string> mset;
  mset.insert("two");
  mset.insert("three");

  // Assign
  mme = mvec;
  EXPECT_EQ(mme.contains("one"), true);
  EXPECT_EQ(mme.contains("two"), true);
  EXPECT_EQ(mme.contains("three"), false);
  EXPECT_EQ(mme.contains("four"), false);

  mme = mset;
  EXPECT_EQ(mme.contains("one"), false);
  EXPECT_EQ(mme.contains("two"), true);
  EXPECT_EQ(mme.contains("three"), true);
  EXPECT_EQ(mme.contains("four"), false);

  // Insert
  mme.setAdditionalValue(mvec);
  EXPECT_EQ(mme.contains("one"), true);
  EXPECT_EQ(mme.contains("two"), true);
  EXPECT_EQ(mme.contains("three"), true);
  EXPECT_EQ(mme.contains("four"), false);

  // Insert another valid multi-enum
  mme.clearSetValues();
  mme = "one four";
  MultiMooseEnum mme2("one two three four", "three");
  mme.setAdditionalValue(mme2);
  EXPECT_EQ(mme.contains("one"), true);
  EXPECT_EQ(mme.contains("two"), false);
  EXPECT_EQ(mme.contains("three"), true);
  EXPECT_EQ(mme.contains("four"), true);

  mme.clearSetValues();
  mme = "one four";
  EXPECT_EQ(mme.contains("one"), true);
  EXPECT_EQ(mme.contains("two"), false);
  EXPECT_EQ(mme.contains("three"), false);
  EXPECT_EQ(mme.contains("four"), true);

  mme.setAdditionalValue("three four");
  EXPECT_EQ(mme.contains("one"), true);
  EXPECT_EQ(mme.contains("two"), false);
  EXPECT_EQ(mme.contains("three"), true);
  EXPECT_EQ(mme.contains("four"), true);

  // Size
  EXPECT_EQ(mme.size(), 4);

  // All but "two" should be in the Enum
  std::set<std::string> compare_set, return_set, difference;
  for (MooseEnumIterator it = mme.begin(); it != mme.end(); ++it)
    return_set.insert(*it);

  compare_set.insert("ONE");
  compare_set.insert("THREE");
  compare_set.insert("FOUR");

  std::set_symmetric_difference(return_set.begin(),
                                return_set.end(),
                                compare_set.begin(),
                                compare_set.end(),
                                std::inserter(difference, difference.end()));
  EXPECT_EQ(difference.size(), 0);

  // Order and indexing
  mme.clearSetValues();
  mme = "one two four";
  EXPECT_EQ(mme.contains("three"), false);

  EXPECT_EQ(mme[0], "one");
  EXPECT_EQ(mme[1], "two");
  EXPECT_EQ(mme[2], "four");
}

TEST(MooseEnum, testDeprecate)
{
  // Intentionally misspelling
  try
  {
    MooseEnum me("one too three four", "too");
    me.deprecate("too", "two");
    FAIL() << "missing expected error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("Cannot deprecate the enum item too, since the replaced"), std::string::npos)
        << "failed with unexpected error: " << msg;
  }

  try
  {
    MooseEnum me("one too two three four", "one");
    me.deprecate("too", "two");
    me = "too";
    FAIL() << "missing expected error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("is deprecated, consider using"), std::string::npos)
        << "failed with unexpected error: " << msg;
  }

  try
  {
    MultiMooseEnum mme("one too two three four");
    mme.deprecate("too", "two");
    mme.setAdditionalValue("one");
    mme = "too";
    FAIL() << "missing expected error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("is deprecated, consider using"), std::string::npos)
        << "failed with unexpected error: " << msg;
  }
}

TEST(MooseEnum, testErrors)
{
  // Assign invalid item
  try
  {
    MultiMooseEnum error_check("one two three");
    error_check = "four";
    FAIL() << "missing expected error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("Invalid option"), std::string::npos)
        << "failed with unexpected error: " << msg;
  }

  // Whitespace around equals sign
  try
  {
    MultiMooseEnum error_check("one= 1 two three");
    FAIL() << "missing expected error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("You cannot place whitespace around the '=' character"), std::string::npos)
        << "failed with unexpected error: " << msg;
  }

  // Duplicate IDs
  try
  {
    MultiMooseEnum error_check("one=1 two=1");
    FAIL() << "missing expected error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(
        msg.find("The supplied id 1 already exists in the enumeration, cannot not add 'two'."),
        std::string::npos)
        << "failed with unexpected error: " << msg;
  }

  // Duplicate name
  try
  {
    MultiMooseEnum error_check("one=1 two=2 one=3");
    FAIL() << "missing expected error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("The name 'ONE' already exists in the enumeration."), std::string::npos)
        << "failed with unexpected error: " << msg;
  }
}

TEST(MultiMooseEnum, testExecuteOn)
{
  ExecFlagEnum exec_enum = MooseUtils::getDefaultExecFlagEnum();
  exec_enum = EXEC_INITIAL;

  // Checks that names are added and removed
  EXPECT_EQ(exec_enum.getRawNames(),
            "NONE INITIAL LINEAR NONLINEAR_CONVERGENCE NONLINEAR POSTCHECK TIMESTEP_END "
            "TIMESTEP_BEGIN MULTIAPP_FIXED_POINT_END MULTIAPP_FIXED_POINT_BEGIN FINAL CUSTOM");
  std::vector<std::string> opts = {"NONE",
                                   "INITIAL",
                                   "LINEAR",
                                   "NONLINEAR_CONVERGENCE",
                                   "NONLINEAR",
                                   "POSTCHECK",
                                   "TIMESTEP_END",
                                   "TIMESTEP_BEGIN",
                                   "MULTIAPP_FIXED_POINT_END",
                                   "MULTIAPP_FIXED_POINT_BEGIN",
                                   "FINAL",
                                   "CUSTOM"};
  EXPECT_EQ(exec_enum.getNames(), opts);

  // Check that added names can be used
  EXPECT_TRUE(exec_enum.contains("initial"));
  EXPECT_TRUE(exec_enum.contains(EXEC_INITIAL));

  exec_enum.addAvailableFlags(EXEC_FAILED);
  EXPECT_FALSE(exec_enum.contains("failed"));
  EXPECT_FALSE(exec_enum.contains(EXEC_FAILED));

  exec_enum = "failed";
  EXPECT_TRUE(exec_enum.contains("failed"));
  EXPECT_TRUE(exec_enum.contains(EXEC_FAILED));

  EXPECT_EQ(exec_enum.size(), 1);
  EXPECT_EQ(exec_enum[0], "FAILED");
  EXPECT_EQ(exec_enum.get(0), EXEC_FAILED);

  // Error when bad name is removed
  try
  {
    ExecFlagType WRONG("wrong", 99);
    exec_enum.removeAvailableFlags(WRONG);
    FAIL() << "missing expected error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("The supplied item 'wrong' is not an available enum item for the "
                       "MultiMooseEnum object"),
              std::string::npos)
        << "failed with unexpected error: " << msg;
  }

  try
  {
    exec_enum.removeAvailableFlags(EXEC_FAILED);
    FAIL() << "missing expected error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(
        msg.find("The supplied item 'FAILED' is a selected item, thus it can not be removed."),
        std::string::npos)
        << "failed with unexpected error: " << msg;
  }

  // MultiMooseEnum doc string generation
  std::string doc = exec_enum.getDocString();
  EXPECT_EQ(
      doc,
      "The list of flag(s) indicating when this object should be executed. For a description of "
      "each flag, see https://mooseframework.inl.gov/source/interfaces/SetupInterface.html.");

  // Tests with ExecFlagType assignment operators
  exec_enum = EXEC_FINAL;
  EXPECT_TRUE(exec_enum.contains(EXEC_FINAL));

  exec_enum = {EXEC_FINAL, EXEC_LINEAR};
  EXPECT_TRUE(exec_enum.contains(EXEC_LINEAR));

  exec_enum += EXEC_CUSTOM;
  EXPECT_TRUE(exec_enum.contains(EXEC_LINEAR));
  EXPECT_TRUE(exec_enum.contains(EXEC_CUSTOM));

  exec_enum += {EXEC_TIMESTEP_END, EXEC_TIMESTEP_BEGIN};
  EXPECT_TRUE(exec_enum.contains(EXEC_CUSTOM));
  EXPECT_TRUE(exec_enum.contains(EXEC_TIMESTEP_END));
  EXPECT_TRUE(exec_enum.contains(EXEC_TIMESTEP_BEGIN));
}

TEST(MooseEnum, compareCurrent)
{
  MooseEnum a("a=1 b=2", "a");
  MooseEnum b("a=1 b=2 c=3", "a");
  MooseEnum c("a=2 b=1", "a");

  EXPECT_TRUE(a.compareCurrent(b));
  EXPECT_TRUE(a.compareCurrent(b, MooseEnum::CompareMode::COMPARE_ID));
  EXPECT_TRUE(a.compareCurrent(b, MooseEnum::CompareMode::COMPARE_BOTH));

  b = "b";
  EXPECT_FALSE(a.compareCurrent(b));
  EXPECT_FALSE(a.compareCurrent(b, MooseEnum::CompareMode::COMPARE_ID));
  EXPECT_FALSE(a.compareCurrent(b, MooseEnum::CompareMode::COMPARE_BOTH));

  b = "c";
  EXPECT_FALSE(a.compareCurrent(b));
  EXPECT_FALSE(a.compareCurrent(b, MooseEnum::CompareMode::COMPARE_ID));
  EXPECT_FALSE(a.compareCurrent(b, MooseEnum::CompareMode::COMPARE_BOTH));

  EXPECT_TRUE(a.compareCurrent(c));
  EXPECT_FALSE(a.compareCurrent(c, MooseEnum::CompareMode::COMPARE_ID));
  EXPECT_FALSE(a.compareCurrent(c, MooseEnum::CompareMode::COMPARE_BOTH));

  c = "b";
  EXPECT_FALSE(a.compareCurrent(c));
  EXPECT_TRUE(a.compareCurrent(c, MooseEnum::CompareMode::COMPARE_ID));
  EXPECT_FALSE(a.compareCurrent(c, MooseEnum::CompareMode::COMPARE_BOTH));
}

TEST(MooseEnum, operatorEqual)
{
  MooseEnum a("a=1 b=2", "a");
  EXPECT_EQ(a, 1);
  a = 2;
  EXPECT_EQ(a, 2);

  try
  {
    a = 3;
    FAIL() << "missing error when setting MooseEnum to invalid int.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("Invalid id \"3\" in MooseEnum. Valid ids are \"1, 2\"."), std::string::npos)
        << "failed with unexpected error: " << msg;
  }
}

TEST(MooseEnum, getIDs)
{
  MooseEnum a("a=1980 e=1949", "e");
  EXPECT_EQ(a.getIDs(), std::vector<int>({1949, 1980})); // stored as set so they come out sorted
}
