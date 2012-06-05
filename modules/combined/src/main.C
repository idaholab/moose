/**
 * Elk Application
 */

#include "Elk.h"
//Moose Includes
#include "MooseInit.h"
#include "ElkTestApp.h"

// libMesh includes
#include "perf_log.h"

// Create a performance log
PerfLog Moose::perf_log("Elk");

 // Begin the main program.
int main (int argc, char** argv)
{
  MooseInit init (argc, argv);
  ElkTestApp app(argc, argv);

  app.setCheckUnusedFlag( true );
  app.setSortAlpha( true );
  app.run();

  return 0;
}
