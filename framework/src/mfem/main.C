#include "PlatypusTestApp.h"
#include "MooseMain.h"

// Begin the main program.
int
main(int argc, char * argv[])
{
  Moose::main<PlatypusTestApp>(argc, argv);

  return 0;
}