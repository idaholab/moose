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
 * Example 10: Auxiliary Calcuations
 * This examples shows how to compute auxiliary values outside of the
 * non-linear system
 */

// Moose Includes
#include "MooseInit.h"
#include "MooseApp.h"
#include "Factory.h"

// Example 10 Includes
#include "ExampleAux.h"

// libMesh includes
#include "perf_log.h"

PerfLog Moose::perf_log("Example 10: Auxiliary Calculations");

int main (int argc, char** argv)
{
  MooseInit init (argc, argv);
  MooseApp app(argc, argv);
  app.init();

  // Register our Example AuxKernel with the AuxFactory
  registerAux(ExampleAux);

  app.run();

  return 0;
}
