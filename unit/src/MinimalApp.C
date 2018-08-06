//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest_include.h"

#include "AppFactory.h"
#include "Executioner.h"
#include "MooseMesh.h"

TEST(MinimalApp, create)
{
  const char * argv[1] = {"\0"};
  std::shared_ptr<MooseApp> app = AppFactory::createAppShared("MooseUnitApp", 1, (char **)argv);
  app->parameters().set<bool>("minimal") = true;
  app->run();
  Executioner * exec = app->getExecutioner();
  EXPECT_EQ(exec->name(), "Executioner");
  FEProblemBase & fe_problem = exec->feProblem();
  EXPECT_EQ(fe_problem.name(), "MOOSE Problem");
  MooseMesh & mesh = fe_problem.mesh();
  EXPECT_EQ(mesh.nElem(), 1);
}
