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
 * Example 2: Kernel - Creating your first custom Kernel
 * This example augments the first example (Input File) by adding a custom kernel
 * to apply a convection operator to the domain.
 */

//Moose Includes
#include "MooseInit.h"
#include "Executioner.h"
#include "Factory.h"              // <- New include for registration

// Parser
#include "Parser.h"
#include "MooseSyntax.h"
 
// Example 2 Includes
#include "Convection.h"           // <- New include for our custom kernel

// libMesh includes
#include "perf_log.h"

// Create a performance log
PerfLog Moose::perf_log("Example 2: Kernel");

// Begin the main program.
int main (int argc, char** argv)
{
  MooseInit init (argc, argv);
  // Create a parser object
  Parser p;

  Moose::registerObjects();

  // Register any custom objects you have built on the MOOSE Framework
  registerKernel(Convection);  // <- registration

  // Associate Parser Syntax with specific MOOSE Actions
  Moose::associateSyntax(p);
  
  // Parse commandline and return inputfile filename if appropriate
  std::string input_filename = p.parseCommandLine();
  
  p.parse(input_filename);
  p.execute();

  Executioner *e = p.getExecutioner();
  e->execute();

  return 0;
}
