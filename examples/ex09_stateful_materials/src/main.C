/**
 * Example 9: Stateful Material Properties
 * Demonstrates using material properties computed at previous timesteps
 *
 * Note: AMR cannot be used with stateful material properties and
 * the simulation will run slower
 */

//Moose Includes
#include "Parser.h"
#include "Executioner.h"
#include "MooseSystem.h"
#include "KernelFactory.h"
#include "MaterialFactory.h"

// Example 9 Includes
#include "ExampleDiffusion.h"
#include "Convection.h"
#include "ExampleMaterial.h"

// C++ include files
#include <iostream>

// libMesh includes
#include "perf_log.h"

PerfLog Moose::perf_log("Example 9: Stateful Material Properties");

int main (int argc, char** argv)
{
  MooseInit init (argc, argv);

  MooseSystem moose_system;

  Moose::registerObjects();

  KernelFactory::instance()->registerKernel<Convection>("Convection");
  KernelFactory::instance()->registerKernel<ExampleDiffusion>("ExampleDiffusion");
  MaterialFactory::instance()->registerMaterial<ExampleMaterial>("ExampleMaterial");

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
