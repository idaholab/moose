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
 * Example 20: UserObjects - Specialized computations
 */

// Example Includes
#include "BlockAverageDiffusionMaterial.h"
#include "BlockAverageValue.h"
#include "ExampleDiffusion.h"

// Moose Includes
#include "MooseInit.h"
#include "Moose.h"
#include "MooseApp.h"
#include "Factory.h"

// libMesh includes
#include "perf_log.h"

// Create a performance log
PerfLog Moose::perf_log("Example 20 UserObjects");

 // Begin the main program.
int main (int argc, char** argv)
{
  MooseInit init (argc, argv);
  MooseApp app(argc, argv);
  app.init();

  registerMaterial(BlockAverageDiffusionMaterial);
  registerKernel(ExampleDiffusion);

  // This is how to register a UserObject
  registerUserObject(BlockAverageValue);

  app.run();

  return 0;
}
