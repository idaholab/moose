/**
 * Example 6: Transient Analysis
 * This example demonstrates how to use the builtin transient executioner.
 * Shows the creation of a time kernel "ExampleImplicitEuler"
 *
 * Additional registrations are added but the rest of main stays the same
 */

// Moose Includes
#include "Parser.h"
#include "Executioner.h"
#include "MooseSystem.h"
#include "MooseFactory.h"

// Example 6 Includes
#include "ExampleDiffusion.h"
#include "Convection.h"
#include "ExampleImplicitEuler.h"

// C++ include files
#include <iostream>

// libMesh includes
#include "perf_log.h"

// Create a performance log
PerfLog Moose::perf_log("Example 6: Transient Analysis");

int main (int argc, char** argv)
{
  MooseInit init (argc, argv);
  
  MooseSystem moose_system;

  Moose::registerObjects();

  registerKernel(Convection);
  registerKernel(ExampleDiffusion);
  registerKernel(ExampleImplicitEuler);
  
  Parser p(moose_system);
  
  std::string input_filename = "";
  if ( Moose::command_line->search("-i") )
    input_filename = Moose::command_line->next(input_filename);
  else
    mooseError("Must specify an input file using -i");      

  p.parse(input_filename);
  p.execute();

  Executioner &e = p.getExecutioner();
  e.setup();
  e.execute();
}
