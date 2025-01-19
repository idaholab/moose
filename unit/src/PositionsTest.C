//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "PositionsTest.h"
#include "Positions.h"

TEST_F(PositionsTest, getUninitialized)
{
  // MultiAppPositions is uninitialized at construction, any other Positions with such
  // behavior would do
  InputParameters params = _factory.getValidParams("MultiAppPositions");
  params.set<std::vector<MultiAppName>>("multiapps") = {"m1"};
  auto & positions = addObject<Positions>("MultiAppPositions", "test", params);

  try
  {
    positions.getPositions(false);
    FAIL() << "Missing the expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_NE(msg.find("Positions vector has not been initialized."), std::string::npos);
  }

  try
  {
    positions.getPositionsVector2D();
    FAIL() << "Missing the expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_NE(msg.find("2D positions vectors have not been initialized."), std::string::npos);
  }

  try
  {
    positions.getPositionsVector3D();
    FAIL() << "Missing the expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_NE(msg.find("3D positions vectors have not been initialized."), std::string::npos);
  }

  try
  {
    positions.getPositionsVector4D();
    FAIL() << "Missing the expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_NE(msg.find("4D positions vectors have not been initialized."), std::string::npos);
  }
}

TEST_F(PositionsTest, getters)
{
  InputParameters params = _factory.getValidParams("InputPositions");
  params.set<std::vector<Point>>("positions") = {Point(1, 0, 0), Point(0, 0, 1)};
  auto & positions = addObject<Positions>("InputPositions", "test", params);

  // Test getters
  EXPECT_EQ(positions.getPositions(false)[0], Point(1, 0, 0));
  EXPECT_EQ(positions.getPosition(0, false), Point(1, 0, 0));
  EXPECT_EQ(positions.getNearestPosition(Point(0.8, 0, 0), false), Point(1, 0, 0));
}
