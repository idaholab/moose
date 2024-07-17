//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "TimesTest.h"
#include "Times.h"

TEST_F(TimesTest, getUninitialized)
{
  // SimulationTimes is uninitialized at construction, any other Times with such
  // behavior would do
  InputParameters params = _factory.getValidParams("SimulationTimes");
  auto & times = addObject<Times>("SimulationTimes", "times", params);

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

TEST_F(TimesTest, getters)
{
  InputParameters params = _factory.getValidParams("InputTimes");
  params.set<std::vector<Real>>("times") = {0.2, 0.8, 1.2};
  auto & times = addObject<Times>("InputTimes", "times", params);

  // Test getters
  EXPECT_EQ(times.getTimes()[0], 0.2);
  EXPECT_EQ(*times.getUniqueTimes().begin(), 0.2);
  EXPECT_EQ(times.getTimeAtIndex(0), 0.2);
  EXPECT_EQ(times.getCurrentTime(), 0);
  EXPECT_EQ(times.getPreviousTime(1), 0.8);
  EXPECT_EQ(times.getNextTime(1, true), 1.2);
}
