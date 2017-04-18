
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
#include "MooseInit.h"
#include "MooseTestApp.h"
#include "Moose.h"
#include "AppFactory.h"

// Create a performance log
PerfLog Moose::perf_log("Moose Test");

int
main(int argc, char * argv[])
{
  // Initialize MPI, solvers and MOOSE
  MooseInit init(argc, argv);

  // Register this application's MooseApp and any it depends on
  MooseTestApp::registerApps();

  // Create an instance of the application and store it in a smart pointer for easy cleanup
  std::shared_ptr<MooseApp> app = AppFactory::createAppShared("MooseTestApp", argc, argv);

  // Execute the application
  app->run();

  return 0;
}
