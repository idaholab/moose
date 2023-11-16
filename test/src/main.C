//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseInit.h"
#include "MooseTestApp.h"
#include "Moose.h"
#include "MooseCreate.h"

#include <iostream>

// Create a performance log
PerfLog Moose::perf_log("Moose Test");

int
main(int argc, char * argv[])
{
  // Initialize MPI, solvers and MOOSE
  MooseInit init(argc, argv);

  // Register this application's MooseApp and any it depends on
  MooseTestApp::registerApps();

  // Instanitiate Moose App
  MooseCreate create(argc, argv);

  // Get the created application;
  std::shared_ptr<MooseApp> app = create.getApp();

  // Execute the application
  app->run();

  return 0;
}
