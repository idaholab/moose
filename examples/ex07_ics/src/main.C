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
 * Example 7: Initial Conditions
 * This example demonstrates how to create and apply custom initial condition object
 * for one of our simulation variables
 */

//Moose Includes
#include "MooseInit.h"
#include "Moose.h"
#include "ExampleApp.h"

// libMesh includes
#include "libmesh/perf_log.h"

// Initialize default Performance Logging
PerfLog Moose::perf_log("Example 7: Initial Condition");

int main (int argc, char** argv)
{
  MooseInit init (argc, argv);

  ExampleApp app(argc, argv);
  app.run();

  return 0;
}
