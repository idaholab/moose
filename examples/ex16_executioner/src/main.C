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
 * Example 16: Custom Executioner
 * This example uses the modular execution system in MOOSE
 * to plugin a new executioner.  This example extendes the transient
 * executioner to change the timestep in a fixed way over the
 * course of the simulation
 */

//Moose Includes
#include "MooseInit.h"
#include "Moose.h"
#include "ExampleApp.h"

// libMesh includes
#include "libmesh/perf_log.h"

PerfLog Moose::perf_log("Example 16: Custom Executioner");

int main (int argc, char** argv)
{
  MooseInit init (argc, argv);

  ExampleApp app(argc, argv);
  app.run();

  return 0;
}
