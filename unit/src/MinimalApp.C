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

#include "MinimalApp.h"
#include "AppFactory.h"
#include "Executioner.h"
#include "MooseMesh.h"

CPPUNIT_TEST_SUITE_REGISTRATION(MinimalApp);

void
MinimalApp::createMinimalAppTest()
{
  const char * argv[1] = {"\0"};
  MooseApp * app = AppFactory::createApp("MooseUnitApp", 1, (char **)argv);
  app->parameters().set<bool>("minimal") = true;
  app->run();
  CPPUNIT_ASSERT(app->executioner()->name() == "Executioner");
  CPPUNIT_ASSERT(app->executioner()->feProblem().name() == "MOOSE Problem");
  CPPUNIT_ASSERT(app->executioner()->feProblem().mesh().nElem() == 1);
  delete app;
}
