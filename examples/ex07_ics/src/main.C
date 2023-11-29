//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

/**
 * Example 7: Initial Conditions
 * This example demonstrates how to create and apply custom initial condition object
 * for one of our simulation variables
 */

#include "ExampleApp.h"

// Moose Includes
#include "Moose.h"
#include "MooseMain.h"

// Create a performance log
PerfLog Moose::perf_log("Example");

// Begin the main program.
int
main(int argc, char * argv[])
{
  moose::main<ExampleApp>(argc, argv);

  return 0;
}
