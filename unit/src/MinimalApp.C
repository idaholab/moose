//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "MooseMain.h"
#include "FEProblemBase.h"

TEST(MinimalApp, create)
{
  const char * argv[3] = {"unused", "--minimal", "\0"};
  std::shared_ptr<MooseApp> app = Moose::createMooseApp("MooseUnitApp", 2, (char **)argv);
  app->run();
  Executioner * exec = app->getExecutioner();
  EXPECT_EQ(exec->name(), "Executioner");
  FEProblemBase & fe_problem = exec->feProblem();
  EXPECT_EQ(fe_problem.name(), "MOOSE Problem");
  MooseMesh & mesh = fe_problem.mesh();
  EXPECT_EQ(mesh.nElem(), 1);
}
