#include "MooseInit.h"
#include "MooseTest.h"
#include "MooseTestApp.h"
#include "Moose.h"
#include "AppFactory.h"

// Create a performance log
PerfLog Moose::perf_log("Moose Test");

int main(int argc, char *argv[])
{
  // Initialize MPI, solvers and MOOSE
  MooseInit init(argc, argv);

  // Register this application's MooseApp and any it depends on
  MooseTest::registerApps();

  // This creates dynamic memory that we're responsible for deleting
  MooseApp * app = AppFactory::createApp("MooseTestApp", argc, argv);

  // Execute the application
  app->run();

  // Free up the memory we created earlier
  delete app;

  return 0;
}
