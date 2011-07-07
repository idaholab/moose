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
 * Example 3: Coupling - Demonstrating Variable Coupling
 * This example augments the second example (Convection w/velocity) by coupling
 * in a variable 'v' into the residual calculation of the Convection operator
 *
 * Note: This main subroutine contains no additional code to enable coupling
 */

// Moose Includes
#include "MooseInit.h"
#include "Executioner.h"

// Parser
#include "Parser.h"
#include "MooseSyntax.h"

// Example 3 Includes
#include "Factory.h"
#include "Convection.h"

// libMesh includes
#include "perf_log.h"

// Create a performance log
PerfLog Moose::perf_log("Example 3: Coupling");

// Begin the main program.
int main (int argc, char** argv)
{
  MooseInit init (argc, argv);
  
  Moose::registerObjects();

  registerKernel(Convection);

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
