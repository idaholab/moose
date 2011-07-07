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
#include "Executioner.h"
#include "Factory.h"

// Parser
#include "Parser.h"
#include "MooseSyntax.h"

// Example 4 Includes
#include "Convection.h"
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

  Moose::registerObjects();

  registerKernel(Convection);
  registerBoundaryCondition(CoupledDirichletBC);    // Register our Boundary Conditions
  registerBoundaryCondition(CoupledNeumannBC);

  Parser p;
  
  std::string input_filename = "";
  if ( Moose::command_line->search("-i") )
    input_filename = Moose::command_line->next(input_filename);
  else
    p.printUsage();

  // Associate Parser Syntax with specific MOOSE Actions
  Moose::associateSyntax(p);

  p.parse(input_filename);
  p.execute();

  Executioner *e = p.getExecutioner();
  e->execute();
}
