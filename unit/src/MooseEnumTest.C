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

#include "MooseEnumTest.h"
#include "MooseEnum.h"
#include "MultiMooseEnum.h"

#include <algorithm> // std::set_symmetric_difference

CPPUNIT_TEST_SUITE_REGISTRATION(MooseEnumTest);

void
MooseEnumTest::multiTestOne()
{
  MultiMooseEnum mme("one two three four", "two");

  CPPUNIT_ASSERT(mme.contains("one") == false);
  CPPUNIT_ASSERT(mme.contains("two") == true);
  CPPUNIT_ASSERT(mme.contains("three") == false);
  CPPUNIT_ASSERT(mme.contains("four") == false);

  mme.push_back("four");
  CPPUNIT_ASSERT(mme.contains("one") == false);
  CPPUNIT_ASSERT(mme.contains("two") == true);
  CPPUNIT_ASSERT(mme.contains("three") == false);
  CPPUNIT_ASSERT(mme.contains("four") == true);

  // isValid
  CPPUNIT_ASSERT(mme.isValid() == true);

  mme.clear();
  CPPUNIT_ASSERT(mme.isValid() == false);

  mme.push_back("one three");
  CPPUNIT_ASSERT(mme.contains("one") == true);
  CPPUNIT_ASSERT(mme.contains("two") == false);
  CPPUNIT_ASSERT(mme.contains("three") == true);
  CPPUNIT_ASSERT(mme.contains("four") == false);

  std::vector<std::string> mvec(2);
  mvec[0] = "one";
  mvec[1] = "two";

  std::set<std::string> mset;
  mset.insert("two");
  mset.insert("three");

  // Assign
  mme = mvec;
  CPPUNIT_ASSERT(mme.contains("one") == true);
  CPPUNIT_ASSERT(mme.contains("two") == true);
  CPPUNIT_ASSERT(mme.contains("three") == false);
  CPPUNIT_ASSERT(mme.contains("four") == false);

  mme = mset;
  CPPUNIT_ASSERT(mme.contains("one") == false);
  CPPUNIT_ASSERT(mme.contains("two") == true);
  CPPUNIT_ASSERT(mme.contains("three") == true);
  CPPUNIT_ASSERT(mme.contains("four") == false);

  // Insert
  mme.push_back(mvec);
  CPPUNIT_ASSERT(mme.contains("one") == true);
  CPPUNIT_ASSERT(mme.contains("two") == true);
  CPPUNIT_ASSERT(mme.contains("three") == true);
  CPPUNIT_ASSERT(mme.contains("four") == false);

  mme.clear();
  mme = "one four";
  CPPUNIT_ASSERT(mme.contains("one") == true);
  CPPUNIT_ASSERT(mme.contains("two") == false);
  CPPUNIT_ASSERT(mme.contains("three") == false);
  CPPUNIT_ASSERT(mme.contains("four") == true);

  mme.push_back("three four");
  CPPUNIT_ASSERT(mme.contains("one") == true);
  CPPUNIT_ASSERT(mme.contains("two") == false);
  CPPUNIT_ASSERT(mme.contains("three") == true);
  CPPUNIT_ASSERT(mme.contains("four") == true);

  // Size
  CPPUNIT_ASSERT(mme.size() == 4);
  CPPUNIT_ASSERT(mme.unique_items_size() == 3);

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
  CPPUNIT_ASSERT(difference.size() == 0);

  // Order and indexing
  mme.clear();
  mme = "one two four";
  CPPUNIT_ASSERT(mme.contains("three") == false);

  CPPUNIT_ASSERT(mme[0] == "one");
  CPPUNIT_ASSERT(mme[1] == "two");
  CPPUNIT_ASSERT(mme[2] == "four");
}

void
MooseEnumTest::withNamesFromTest()
{
  //
  // Construct MultiMooseEnum from MooseEnum
  //
  MooseEnum me1("one two three four");
  MultiMooseEnum mme1 = MultiMooseEnum::withNamesFrom(me1);

  // set in-range values
  mme1 = "one two";
  CPPUNIT_ASSERT(mme1.contains("one") == true);
  CPPUNIT_ASSERT(mme1.contains("two") == true);
  CPPUNIT_ASSERT(mme1.contains("three") == false);
  CPPUNIT_ASSERT(mme1.contains("four") == false);

  // compare against out-of-range value
  try
  {
    if (me1 == "five")
      // Unreachable
      CPPUNIT_ASSERT(false);
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());

    CPPUNIT_ASSERT(msg.find("Invalid string comparison") != std::string::npos);
  }

  // set out-of-range values
  try
  {
    mme1 = "five";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    CPPUNIT_ASSERT(msg.find("Invalid option") != std::string::npos);
  }

  // construct mme with out-of-range-allowed
  MooseEnum me2("one two three four", "", true);
  MultiMooseEnum mme2 = MultiMooseEnum::withNamesFrom(me2);
  mme2 = "six";
  CPPUNIT_ASSERT(mme2.contains("six") == true);

  //
  // Construct MooseEnum from MultiMooseEnum
  //
  MultiMooseEnum mme3("one two three four");
  MooseEnum me3 = MooseEnum::withNamesFrom(mme3);

  // set in-range values
  me3 = "one";
  CPPUNIT_ASSERT(me3 == "one");

  // set out-of-range values
  try
  {
    me3 = "five";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    CPPUNIT_ASSERT(msg.find("Invalid option") != std::string::npos);
  }

  // construct mme with out-of-range-allowed
  MultiMooseEnum mme4("one two three four", "", true);
  MooseEnum me4 = MooseEnum::withNamesFrom(mme4);
  me4 = "six";
  CPPUNIT_ASSERT(me4 == "six");
}

void
MooseEnumTest::testDeprecate()
{
  // Intentionally misspelling
  MooseEnum me1("one too three four", "too");

  try
  {
    me1.deprecate("too", "two");

    // Unreachable
    CPPUNIT_ASSERT(false);
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());

    CPPUNIT_ASSERT(msg.find("is deprecated, consider using") != std::string::npos);
  }

  me1 = "one";
  try
  {
    me1 = "too";

    // Unreachable
    CPPUNIT_ASSERT(false);
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    CPPUNIT_ASSERT(msg.find("is deprecated, consider using") != std::string::npos);
  }

  MultiMooseEnum mme1("one too three four");
  mme1.deprecate("too", "two");

  mme1.push_back("one");
  try
  {
    me1 = "too";

    // Unreachable
    CPPUNIT_ASSERT(false);
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    CPPUNIT_ASSERT(msg.find("is deprecated, consider using") != std::string::npos);
  }
}

void
MooseEnumTest::testErrors()
{
  // Assign invalid item
  try
  {
    MultiMooseEnum error_check("one two three");
    error_check = "four";

    // Unreachable
    CPPUNIT_ASSERT(false);
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    CPPUNIT_ASSERT(msg.find("Invalid option") != std::string::npos);
  }

  // Whitespace around equals sign
  try
  {
    MultiMooseEnum error_check("one= 1 two three");

    // Unreachable
    CPPUNIT_ASSERT(false);
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    CPPUNIT_ASSERT(msg.find("You cannot place whitespace around the '=' character") !=
                   std::string::npos);
  }
}
