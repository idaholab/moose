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
 * Example 14: Functions - Using Postprocessors
 *
 * This example will show how to execute some code verification
 * via Method of Manufactured Solutions (MMS).
 *
 * This is achieved by specifying Functions for a forcing function
 * and boundary condition as well as exact solution.  Then comparing
 * the computed solution to the exact solution using Postprocessors.
 */

// Example Includes
#include "ExampleFunction.h"

// Moose Includes
#include "MooseInit.h"
#include "Executioner.h"
#include "Factory.h"

// Parser
#include "Parser.h"
#include "MooseSyntax.h"

// libMesh includes
#include "perf_log.h"

// Create a performance log
PerfLog Moose::perf_log("Example 14 Functions");

 // Begin the main program.
int main (int argc, char** argv)
{
  MooseInit init (argc, argv);

  Moose::registerObjects();

  registerFunction(ExampleFunction);

  // Associate Parser Syntax with specific MOOSE Actions
  Moose::associateSyntax();
  Parser p(Moose::syntax);

  // Parse commandline and return inputfile filename if appropriate
  std::string input_filename = p.parseCommandLine();

  p.parse(input_filename);
  p.execute();

  Moose::executioner->execute();

  return 0;
}
