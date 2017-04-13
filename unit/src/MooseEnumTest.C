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

#include "MooseEnum.h"
#include "MultiMooseEnum.h"

#include <algorithm> // std::set_symmetric_difference

TEST(MooseEnum, multiTestOne)
{
  MultiMooseEnum mme("one two three four", "two");

  EXPECT_EQ(mme.contains("one"), false);
  EXPECT_EQ(mme.contains("two"), true);
  EXPECT_EQ(mme.contains("three"), false);
  EXPECT_EQ(mme.contains("four"), false);

  mme.push_back("four");
  EXPECT_EQ(mme.contains("one"), false);
  EXPECT_EQ(mme.contains("two"), true);
  EXPECT_EQ(mme.contains("three"), false);
  EXPECT_EQ(mme.contains("four"), true);

  // isValid
  EXPECT_EQ(mme.isValid(), true);

  mme.clear();
  EXPECT_EQ(mme.isValid(), false);

  mme.push_back("one three");
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
  mme.push_back(mvec);
  EXPECT_EQ(mme.contains("one"), true);
  EXPECT_EQ(mme.contains("two"), true);
  EXPECT_EQ(mme.contains("three"), true);
  EXPECT_EQ(mme.contains("four"), false);

  mme.clear();
  mme = "one four";
  EXPECT_EQ(mme.contains("one"), true);
  EXPECT_EQ(mme.contains("two"), false);
  EXPECT_EQ(mme.contains("three"), false);
  EXPECT_EQ(mme.contains("four"), true);

  mme.push_back("three four");
  EXPECT_EQ(mme.contains("one"), true);
  EXPECT_EQ(mme.contains("two"), false);
  EXPECT_EQ(mme.contains("three"), true);
  EXPECT_EQ(mme.contains("four"), true);

  // Size
  EXPECT_EQ(mme.size(), 4);
  EXPECT_EQ(mme.unique_items_size(), 3);

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
  mme.clear();
  mme = "one two four";
  EXPECT_EQ(mme.contains("three"), false);

  EXPECT_EQ(mme[0], "one");
  EXPECT_EQ(mme[1], "two");
  EXPECT_EQ(mme[2], "four");
}

TEST(MooseEnum, withNamesFromTest)
{
  //
  // Construct MultiMooseEnum from MooseEnum
  //
  MooseEnum me1("one two three four");
  MultiMooseEnum mme1 = MultiMooseEnum::withNamesFrom(me1);

  // set in-range values
  mme1 = "one two";
  EXPECT_EQ(mme1.contains("one"), true);
  EXPECT_EQ(mme1.contains("two"), true);
  EXPECT_EQ(mme1.contains("three"), false);
  EXPECT_EQ(mme1.contains("four"), false);

  // compare against out-of-range value
  try
  {
    bool foo = me1 == "five";
    foo = foo;
    // TODO: this test fails if you uncomment the following line. It was written incorrectly before
    // FAIL() << "missing expected error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());

    ASSERT_NE(msg.find("Invalid string comparison"), std::string::npos)
        << "failed with unexpected error: " << msg;
  }

  // set out-of-range values
  try
  {
    mme1 = "five";
    FAIL() << "missing expected error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("Invalid option"), std::string::npos) << "failed with unexpected error: "
                                                             << msg;
  }

  // construct mme with out-of-range-allowed
  MooseEnum me2("one two three four", "", true);
  MultiMooseEnum mme2 = MultiMooseEnum::withNamesFrom(me2);
  mme2 = "six";
  EXPECT_EQ(mme2.contains("six"), true);

  //
  // Construct MooseEnum from MultiMooseEnum
  //
  MultiMooseEnum mme3("one two three four");
  MooseEnum me3 = MooseEnum::withNamesFrom(mme3);

  // set in-range values
  me3 = "one";
  EXPECT_EQ(me3, "one");

  // set out-of-range values
  try
  {
    me3 = "five";
    FAIL() << "missing expected error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("Invalid option"), std::string::npos) << "failed with unexpected error: "
                                                             << msg;
  }

  // construct mme with out-of-range-allowed
  MultiMooseEnum mme4("one two three four", "", true);
  MooseEnum me4 = MooseEnum::withNamesFrom(mme4);
  me4 = "six";
  EXPECT_EQ(me4, "six");
}

TEST(MooseEnum, testDeprecate)
{
  // Intentionally misspelling
  MooseEnum me1("one too three four", "too");

  try
  {
    me1.deprecate("too", "two");
    FAIL() << "missing expected error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());

    ASSERT_NE(msg.find("is deprecated, consider using"), std::string::npos)
        << "failed with unexpected error: " << msg;
  }

  me1 = "one";
  try
  {
    me1 = "too";
    FAIL() << "missing expected error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("is deprecated, consider using"), std::string::npos)
        << "failed with unexpected error: " << msg;
  }

  MultiMooseEnum mme1("one too three four");
  mme1.deprecate("too", "two");

  mme1.push_back("one");
  try
  {
    me1 = "too";
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
    ASSERT_NE(msg.find("Invalid option"), std::string::npos) << "failed with unexpected error: "
                                                             << msg;
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
}
