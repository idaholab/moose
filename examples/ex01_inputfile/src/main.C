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
 * Example 1: Input File - The smallest MOOSE based application possible.  It solves
 * a simple 2D diffusion problem with Dirichlet boundary conditions using built-in
 * objects from MOOSE.
 */

//Moose Includes
#include "MooseInit.h"
#include "Executioner.h"

// Parser
#include "Parser.h"
#include "MooseSyntax.h"

// libMesh includes
#include "perf_log.h"

// Create a performance log
PerfLog Moose::perf_log("Example 1: Input File");

// Begin the main program.
int main (int argc, char** argv)
{
  // Create a MooseInit Object
  MooseInit init (argc, argv);

  // Create a parser object
  Parser p;

  // Register a bunch of common objects that exist inside of Moose.  You will 
  // generally create a registerObjects method of your own to register modules
  // that you create in your own application.
  Moose::registerObjects();

  // Associate Parser Syntax with specific MOOSE Actions
  Moose::associateSyntax(p);

  // Parse commandline and return inputfile filename if appropriate
  std::string input_filename = p.parseCommandLine();
  
  // Tell the parser to parse the given file to setup the simulation and execute
  p.parse(input_filename);
  p.execute();

  Executioner *e = p.getExecutioner();
  e->execute();

  return 0;
}
