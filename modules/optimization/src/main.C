//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptimizationTestApp.h"
#include "MooseInit.h"
#include "Moose.h"
#include "MooseApp.h"
#include "AppFactory.h"
#include "MooseCreate.h"

// Create a performance log
PerfLog Moose::perf_log("optimization");

// Begin the main program.
int
main(int argc, char * argv[])
{
  // Initialize MPI, solvers and MOOSE
  MooseInit init(argc, argv);

  // Register this application's MooseApp and any it depends on
  OptimizationTestApp::registerApps();

  // Instanitiate Moose App
  MooseCreate create("OptimizationTestApp", argc, argv);

  // Get the created application;
  std::shared_ptr<MooseApp> app = create.getApp();

  // Execute the application
  app->run();

  return 0;
}
