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
 * Example 5: Adaptive Mesh Refinement:
 * This example shows how to enable AMR in the input file.
 *
 * Note: This file contains no additional changes from the previous example
 */

//Moose Includes
#include "MooseInit.h"
#include "Moose.h"
#include "ExampleApp.h"

PerfLog Moose::perf_log("Example 5: Adaptive Mesh Refinement");

int main (int argc, char** argv)
{
  MooseInit init (argc, argv);

  ExampleApp app(argc, argv);
  app.run();

  return 0;
}
