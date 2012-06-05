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
 * Example 15: Custom Actions
 * This example runs our favorite Convection Diffusion problems but uses
 * a custom parser block to add the kernels all together with one new
 * block in our input file instead of listing everything explicitly
 * It also shows how to inherit from MooseApp and use it.
 */

//Moose Includes
#include "MooseInit.h"
#include "Ex15App.h"

// libMesh includes
#include "perf_log.h"

PerfLog Moose::perf_log("Example 15: Custom Actions");

int main (int argc, char** argv)
{
  MooseInit init (argc, argv);
  Ex15App app(argc, argv);

  app.run();

  return 0;
}
