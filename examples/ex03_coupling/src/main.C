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
  Parser p;

  Moose::registerObjects();

  registerKernel(Convection);
  Moose::associateSyntax(p);

  // Parse commandline and return inputfile filename if appropriate
  std::string input_filename = p.parseCommandLine();

  p.parse(input_filename);
  p.execute();

  Executioner *e = p.getExecutioner();
  e->execute();

  return 0;
}
