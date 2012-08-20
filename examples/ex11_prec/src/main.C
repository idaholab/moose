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
 * Example 12: Physics Based Preconditioning
 * This example shows how to enable the use of more advanced preconditioning
 * with the optional Kernel::computeQpOffDiagJacobian method and input PBP block
 */

//Moose Includes
#include "MooseInit.h"
#include "Moose.h"
#include "MooseApp.h"

// libMesh includes
#include "perf_log.h"

PerfLog Moose::perf_log("Example12: Physics Based Preconditioning");

int main (int argc, char** argv)
{
  MooseInit init (argc, argv);
  MooseApp app(argc, argv);
  app.init();

  app.run();

  return 0;
}
