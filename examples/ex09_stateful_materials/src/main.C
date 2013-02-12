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
 * Example 9: Stateful Material Properties
 * Demonstrates using material properties computed at previous timesteps
 *
 * Note: AMR cannot be used with stateful material properties and
 * the simulation will run slower
 */

//Moose Includes
#include "MooseInit.h"
#include "Moose.h"
#include "ExampleApp.h"

PerfLog Moose::perf_log("Example 9: Stateful Material Properties");

int main (int argc, char** argv)
{
  MooseInit init (argc, argv);

  ExampleApp app(argc, argv);
  app.run();

  return 0;
}
