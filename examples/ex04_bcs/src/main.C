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
 * Example 4: Boundary Conditions
 * This example demonstrates Boundary Condition Coupling and active syntax in input file
 * This example augments the third example (Convection-Diffusion) by adding custom boundary
 * conditions selectable by changing the input file
 */

// Moose Includes
#include "MooseInit.h"
#include "MooseApp.h"
#include "Factory.h"

// Example 4 Includes
#include "Convection.h"
#include "GaussContForcing.h"
#include "CoupledDirichletBC.h"
#include "CoupledNeumannBC.h"

// libMesh includes
#include "perf_log.h"

// Create a performance log
PerfLog Moose::perf_log("Example 4: Boundary Conditions");

// Begin the main program.
int main (int argc, char** argv)
{
  MooseInit init (argc, argv);
  MooseApp app(argc, argv);
  app.init();

  registerKernel(Convection);
  registerKernel(GaussContForcing);                 // Extra forcing term
  registerBoundaryCondition(CoupledDirichletBC);    // Register our Boundary Conditions
  registerBoundaryCondition(CoupledNeumannBC);

  app.run();

  return 0;
}
