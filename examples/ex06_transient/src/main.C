/**
 * Example 6: Transient Analysis
 * This example demonstrates how to use the builtin transient executioner.
 * Shows the creation of a time kernel "ExampleImplicitEuler"
 */

// Moose Includes
#include "Parser.h"
#include "Executioner.h"
#include "MooseSystem.h"
#include "KernelFactory.h"
#include "MaterialFactory.h"

// Example 6 Includes
#include "ExampleDiffusion.h"
#include "Convection.h"
#include "ExampleImplicitEuler.h"

#include "ExampleMaterial.h"

// C++ include files
#include <iostream>

// libMesh includes
#include "perf_log.h"

// Create a performance log
PerfLog Moose::perf_log("Example 6");

int main (int argc, char** argv)
{
  MooseInit init (argc, argv);
  
  MooseSystem moose_system;

  Moose::registerObjects();

  KernelFactory::instance()->registerKernel<Convection>("Convection");
  KernelFactory::instance()->registerKernel<ExampleDiffusion>("ExampleDiffusion");
  KernelFactory::instance()->registerKernel<ExampleImplicitEuler>("ExampleImplicitEuler");
  
  // Register our new material class so we can use it.
  MaterialFactory::instance()->registerMaterial<ExampleMaterial>("ExampleMaterial");

  Parser p(moose_system);
  
  std::string input_filename = "";
  if ( Moose::command_line->search("-i") )
    input_filename = Moose::command_line->next(input_filename);
  else
    mooseError("Must specify an input file using -i");      

  p.parse(input_filename);
  p.execute();

  if(!Moose::executioner)
    mooseError("Executioner not supplied!");
  
  Moose::executioner->setup();
  Moose::executioner->execute();
}
