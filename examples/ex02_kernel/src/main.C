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
 * Example 2: Kernel - Creating your first custom Kernel
 * This example augments the first example (Input File) by adding a custom kernel
 * to apply a convection operator to the domain.
 */

//Moose Includes
#include "MooseInit.h"
#include "MooseApp.h"
#include "Factory.h"
// Example 2 Includes
#include "Convection.h"           // <- New include for our custom kernel

// libMesh includes
#include "perf_log.h"

// Create a performance log
PerfLog Moose::perf_log("Example 2: Kernel");

// Begin the main program.
int main (int argc, char** argv)
{
  MooseInit init (argc, argv);
  MooseApp app(argc, argv);

  // Register any custom objects you have built on the MOOSE Framework
  registerKernel(Convection);  // <- registration

  app.run();

  return 0;
}
