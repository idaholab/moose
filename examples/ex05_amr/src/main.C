//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

/**
 * Example 5: Adaptive Mesh Refinement:
 * This example shows how to enable AMR in the input file.
 *
 * Note: This file contains no additional changes from the previous example
 */

#include "ExampleApp.h"
#include "MooseMain.h"

// Begin the main program.
int
main(int argc, char * argv[])
{
  return Moose::main<ExampleApp>(argc, argv);
}
