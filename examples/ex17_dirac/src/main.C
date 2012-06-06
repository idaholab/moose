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
 * Example 17: Dirac Kernel - Creating a Dirac Kernel
 */

//Moose Includes
#include "MooseInit.h"
#include "MooseApp.h"
#include "Factory.h"

// Example 17 Includes
#include "Convection.h"
#include "ExampleDirac.h"         // <- New include for our custom DiracKernel

// libMesh includes
#include "perf_log.h"

// Create a performance log
PerfLog Moose::perf_log("Example 17: DiracKernel");

// Begin the main program.
int main (int argc, char** argv)
{
  MooseInit init (argc, argv);
  MooseApp app(argc, argv);
  app.init();

  // Register any custom objects you have built on the MOOSE Framework
  registerKernel(Convection);
  registerDiracKernel(ExampleDirac);  // <- registration

  app.run();

  return 0;
}
