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
 * Example 6: Transient Analysis
 * This example demonstrates how to use the builtin transient executioner.
 * Shows the creation of a time kernel "ExampleImplicitEuler"
 *
 * Additional registrations are added but the rest of main stays the same
 */

// Moose Includes
#include "MooseInit.h"
#include "MooseApp.h"
#include "Factory.h"

// Example 6 Includes
#include "ExampleDiffusion.h"
#include "Convection.h"
#include "ExampleTimeDerivative.h"

// libMesh includes
#include "perf_log.h"

// Create a performance log
PerfLog Moose::perf_log("Example 6: Transient Analysis");

int main (int argc, char** argv)
{
  MooseInit init (argc, argv);
  MooseApp app(argc, argv);
  app.init();

  registerKernel(Convection);
  registerKernel(ExampleDiffusion);
  registerKernel(ExampleTimeDerivative);

  app.run();

  return 0;
}
