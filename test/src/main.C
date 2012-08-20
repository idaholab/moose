#include "MooseInit.h"
#include "MooseTest.h"
#include "Moose.h"
// libMesh includes
#include "perf_log.h"

// Create a performance log
PerfLog Moose::perf_log("Moose Test");

int main(int argc, char *argv[])
{
  MooseInit init(argc, argv);

  MooseTestApp app(argc, argv);
  app.run();

  return 0;
}
