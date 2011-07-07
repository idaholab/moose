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
 * Example 18: Kernel - Creating a custom Dirac Kernel
 */

//Moose Includes
#include "MooseInit.h"
#include "Executioner.h"
#include "Factory.h"

// Parser
#include "Parser.h"
#include "MooseSyntax.h"

// Example 18 Includes
#include "Convection.h"
#include "ExampleDirac.h"         // <- New include for our custom DiracKernel

// libMesh includes
#include "perf_log.h"

// Create a performance log
PerfLog Moose::perf_log("Example 18: DiracKernel");

// Begin the main program.
int main (int argc, char** argv)
{
  MooseInit init (argc, argv);

  Moose::registerObjects();

  // Register any custom objects you have built on the MOOSE Framework
  registerKernel(Convection);  
  registerDiracKernel(ExampleDirac);  // <- registration

  // Create a parser object
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

  return 0;
}
