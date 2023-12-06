//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

/**
 * Example 13: Functions - Using function objects
 */

#include "ExampleApp.h"

// Moose Includes
#include "MooseInit.h"
#include "MooseApp.h"
#include "AppFactory.h"
#include "MooseMain.h"

// Create a performance log
PerfLog Moose::perf_log("Example");

// Begin the main program.
int
main(int argc, char * argv[])
{
  Moose::main<ExampleApp>(argc, argv);

  return 0;
}
