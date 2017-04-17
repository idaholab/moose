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

#include "AppFactory.h"
#include "Executioner.h"
#include "MooseMesh.h"

TEST(MinimalApp, create)
{
  const char * argv[1] = {"\0"};
  std::unique_ptr<MooseApp> app(AppFactory::createApp("MooseUnitApp", 1, (char **)argv));
  app->parameters().set<bool>("minimal") = true;
  app->run();
  EXPECT_EQ(app->executioner()->name(), "Executioner");
  EXPECT_EQ(app->executioner()->feProblem().name(), "MOOSE Problem");
  EXPECT_EQ(app->executioner()->feProblem().mesh().nElem(), 1);
}
