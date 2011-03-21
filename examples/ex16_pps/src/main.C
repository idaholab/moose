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
 * Example 16: Functions - Using Postprocessors
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
#include "Parser.h"
#include "Executioner.h"
#include "Factory.h"

// libMesh includes
#include "perf_log.h"

// Create a performance log
PerfLog Moose::perf_log("Example 16 Functions");

 // Begin the main program.
int main (int argc, char** argv)
{
  MooseInit init (argc, argv);

  Moose::registerObjects();

  registerFunction(ExampleFunction);

  Parser p;
  
  std::string input_filename = "";
  if ( Moose::command_line->search("-i") )
    input_filename = Moose::command_line->next(input_filename);
  else
    p.printUsage();
  
  p.parse(input_filename);
  p.execute();

  Executioner *e = p.getExecutioner();
  e->execute();
}
