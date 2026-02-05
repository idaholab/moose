//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "Times.h"

TEST(TimesTest, getUninitialized)
{
  std::vector<Real> times_vec = {};
  const Real current_time = 0;
  Times times(times_vec, current_time, /*is_dynamic*/ false);

  try
  {
    times.getTimes();
    FAIL() << "Missing the expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_NE(msg.find("Times vector has not been initialized."), std::string::npos);
  }
}

#ifndef NDEBUG
TEST(TimesTest, unsorted)
{
  std::vector<Real> times_vec = {3.0, 2.0, 1.0};
  const Real current_time = 0;
  Times times(times_vec, current_time, /*is_dynamic*/ false);

  try
  {
    times.getPreviousTime(2);
    FAIL() << "Missing the expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_NE(msg.find("Times vector must be sorted."), std::string::npos);
  }
  try
  {
    times.getNextTime(2, true);
    FAIL() << "Missing the expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_NE(msg.find("Times vector must be sorted."), std::string::npos);
  }
}
#endif

TEST(TimesTest, getters)
{
  std::vector<Real> times_vec = {0.2, 0.8, 1.2};
  const Real current_time = 0;
  Times times(times_vec, current_time, /*is_dynamic*/ false);

  // Test getters
  EXPECT_EQ(times.getTimes()[0], 0.2);
  EXPECT_EQ(*times.getUniqueTimes().begin(), 0.2);
  EXPECT_EQ(times.getTimeAtIndex(0), 0.2);
  EXPECT_EQ(times.getCurrentTime(), 0);
  EXPECT_EQ(times.getPreviousTime(1), 0.8);
  EXPECT_EQ(times.getPreviousTime(0, false), -std::numeric_limits<Real>::max());
  EXPECT_EQ(times.getNextTime(1, true), 1.2);
  EXPECT_EQ(times.getNextTime(2, false), std::numeric_limits<Real>::max());

  try
  {
    times.getPreviousTime(0);
    FAIL() << "Missing the expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_NE(
        msg.find("No previous time in Times vector for time 0. Minimum time in vector is 0.2"),
        std::string::npos);
  }
  try
  {
    times.getNextTime(2, true);
    FAIL() << "Missing the expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_NE(msg.find("No next time in Times vector for time 2. Maximum time in vector is 1.2"),
              std::string::npos);
  }
}
