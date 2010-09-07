/**
 * Example 8: Material Properties
 * Demonstrates making a Material Class and using a Material Property in a Residual
 * Calculation.
 */

//Moose Includes
#include "Parser.h"
#include "Executioner.h"
#include "MooseSystem.h"
#include "MooseFactory.h"

// Example 8 Includes
#include "ExampleDiffusion.h"
#include "Convection.h"
#include "ExampleMaterial.h"

// C++ include files
#include <iostream>

// libMesh includes
#include "perf_log.h"

// Create a performance log
PerfLog Moose::perf_log("Example 8: Material Properties");

 // Begin the main program.
int main (int argc, char** argv)
{
  MooseInit init (argc, argv);
  
  MooseSystem moose_system;
  
  Moose::registerObjects();

  registerKernel(Convection);

  // Our new Diffusion Kernel that accepts a material property
  registerKernel(ExampleDiffusion);
  
  // Register our new material class so we can use it.
  registerMaterial(ExampleMaterial);

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
