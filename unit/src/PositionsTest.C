//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "MultiAppPositions.h"
#include "InputPositions.h"
#include "AppFactory.h"
#include "Executioner.h"

TEST(Positions, getUninitialized)
{
  // Create a minimal app that can create objects
  const char * argv[2] = {"foo", "\0"};
  const auto & app = AppFactory::createAppShared("MooseUnitApp", 1, (char **)argv);
  const auto & factory = &app->getFactory();
  app->parameters().set<bool>("minimal") = true;
  app->run();
  Executioner * exec = app->getExecutioner();
  FEProblemBase * fe_problem = &exec->feProblem();

  // MultiAppPositions is uninitialized at construction, any other Positions with such
  // behavior would do
  InputParameters params = factory->getValidParams("MultiAppPositions");
  params.set<FEProblemBase *>("_fe_problem_base") = fe_problem;
  params.set<SubProblem *>("_subproblem") = fe_problem;
  params.set<std::vector<MultiAppName>>("multiapps") = {"m1"};
  params.set<std::string>("_object_name") = "test";
  params.set<std::string>("_type") = "MultiAppPositions";
  MultiAppPositions positions(params);

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

TEST(Positions, getters)
{
  // Create a minimal app that can create objects
  const char * argv[2] = {"foo", "\0"};
  const auto & app = AppFactory::createAppShared("MooseUnitApp", 1, (char **)argv);
  const auto & factory = &app->getFactory();
  app->parameters().set<bool>("minimal") = true;
  app->run();
  Executioner * exec = app->getExecutioner();
  FEProblemBase * fe_problem = &exec->feProblem();

  InputParameters params = factory->getValidParams("InputPositions");
  params.set<FEProblemBase *>("_fe_problem_base") = fe_problem;
  params.set<SubProblem *>("_subproblem") = fe_problem;
  params.set<std::vector<Point>>("positions") = {Point(1, 0, 0), Point(0, 0, 1)};
  params.set<std::string>("_object_name") = "test";
  params.set<std::string>("_type") = "InputPositions";
  params.set<bool>("auto_sort") = true;
  InputPositions positions(params);

  // Test getters
  EXPECT_EQ(positions.getPositions(false)[0], Point(1, 0, 0));
  EXPECT_EQ(positions.getPosition(0, false), Point(1, 0, 0));
  EXPECT_EQ(positions.getNearestPosition(Point(0.8, 0, 0), false), Point(1, 0, 0));
}
