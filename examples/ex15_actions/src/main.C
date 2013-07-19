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

/**
 * Example 15: Custom Actions
 * This example runs our favorite Convection Diffusion problems but uses
 * a custom parser block to add the kernels all together with one new
 * block in our input file instead of listing everything explicitly
 * It also shows how to inherit from MooseApp and use it.
 */

#include "MooseInit.h"
#include "Moose.h"
#include "MooseApp.h"
#include "AppFactory.h"
#include "ExampleApp.h"

// Create a performance log
PerfLog Moose::perf_log("Example");

 // Begin the main program.
int main(int argc, char *argv[])
{
  // Initialize MPI, solvers and MOOSE
  MooseInit init(argc, argv);

  // Register this application's MooseApp and any it depends on
  ExampleApp::registerApps();

  // This creates dynamic memory that we're responsible for deleting
  MooseApp * app = AppFactory::createApp("ExampleApp", argc, argv);

  // Execute the application
  app->run();

  // Free up the memory we created earlier
  delete app;

  return 0;
}
