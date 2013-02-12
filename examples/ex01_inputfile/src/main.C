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
 * Example 1: Input File - The smallest MOOSE based application possible.  It solves
 * a simple 2D diffusion problem with Dirichlet boundary conditions using built-in
 * objects from MOOSE.
 */

//Moose Includes
#include "MooseInit.h"
#include "Moose.h"
#include "ExampleApp.h"

// Create a performance log
PerfLog Moose::perf_log("Example 1: Input File");

// Begin the main program.
int main (int argc, char** argv)
{
  // Create a MooseInit Object
  MooseInit init (argc, argv);
  // Create MOOSE app that will take care of registering object, syntax, etc.
  ExampleApp app(argc, argv);
  app.run();

  return 0;
}
