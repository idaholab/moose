//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "InputTimes.h"
#include "SimulationTimes.h"
#include "AppFactory.h"
#include "Executioner.h"

TEST(Times, getUninitialized)
{
  // Create a minimal app that can create objects
  const char * argv[2] = {"foo", "\0"};
  const auto & app = AppFactory::createAppShared("MooseUnitApp", 1, (char **)argv);
  const auto & factory = &app->getFactory();
  app->parameters().set<bool>("minimal") = true;
  app->run();
  Executioner * exec = app->getExecutioner();
  FEProblemBase * fe_problem = &exec->feProblem();

  // SimulationTimes is uninitialized at construction, any other Times with such
  // behavior would do
  InputParameters params = factory->getValidParams("SimulationTimes");
  params.set<FEProblemBase *>("_fe_problem_base") = fe_problem;
  params.set<SubProblem *>("_subproblem") = fe_problem;
  params.set<std::string>("_object_name") = "test";
  params.set<std::string>("_type") = "SimulationTimes";
  SimulationTimes Times(params);

  try
  {
    Times.getTimes();
    FAIL() << "Missing the expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_NE(msg.find("Times vector has not been initialized."), std::string::npos);
  }
}

TEST(Times, getters)
{
  // Create a minimal app that can create objects
  const char * argv[2] = {"foo", "\0"};
  const auto & app = AppFactory::createAppShared("MooseUnitApp", 1, (char **)argv);
  const auto & factory = &app->getFactory();
  app->parameters().set<bool>("minimal") = true;
  app->run();
  Executioner * exec = app->getExecutioner();
  FEProblemBase * fe_problem = &exec->feProblem();

  InputParameters params = factory->getValidParams("InputTimes");
  params.set<FEProblemBase *>("_fe_problem_base") = fe_problem;
  params.set<SubProblem *>("_subproblem") = fe_problem;
  params.set<std::vector<Real>>("times") = {0.2, 0.8, 1.2};
  params.set<std::string>("_object_name") = "test";
  params.set<std::string>("_type") = "InputTimes";
  InputTimes Times(params);

  // Test getters
  EXPECT_EQ(Times.getTimes()[0], 0.2);
  EXPECT_EQ(*Times.getUniqueTimes().begin(), 0.2);
  EXPECT_EQ(Times.getTimeAtIndex(0), 0.2);
  EXPECT_EQ(Times.getCurrentTime(), 1);
  EXPECT_EQ(Times.getPreviousTime(1), 0.8);
  EXPECT_EQ(Times.getNextTime(1), 1.2);
}
