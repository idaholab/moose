/**
 * Example 12 - Demonstrates creating a custom Executioner
 * This example uses the modular execution system in MOOSE
 * to plugin a new executioner.  This example extendes the trasient
 * executioner to change the timestep in a fixed way over the
 * course of the simulation
 */

//Moose Includes
#include "Moose.h"
#include "MooseSystem.h"
#include "Parser.h"
#include "Executioner.h"
#include "ExecutionerFactory.h"
#include "TransientHalf.h"
#include "KernelFactory.h"
#include "ExampleDiffusion.h"
#include "Convection.h"
#include "ExampleImplicitEuler.h"
#include "ExampleMaterial.h"
#include "MaterialFactory.h"

// C++ include files that we need
#include <iostream>
#include <fstream>

// libMesh includes
#include "perf_log.h"
#include "mesh.h"

#include "exodusII_io.h"
#include "equation_systems.h"
#include "transient_system.h"
#include "getpot.h"

// Initialize default Performance Logging
PerfLog Moose::perf_log("Example11");

// Begin the main program.
int main (int argc, char** argv)
{
  // Initialize Moose and any dependent libaries
  MooseInit init (argc, argv);

  // Create a MooseSystem
  MooseSystem moose_system;

  // This registers a bunch of common objects that exist in Moose with the factories.
   // that includes several Kernels, BoundaryConditions, AuxKernels and Materials
  Moose::registerObjects();

  ExecutionerFactory::instance()->registerExecutioner<TransientHalf>("TransientHalf");
  KernelFactory::instance()->registerKernel<ExampleDiffusion>("ExampleDiffusion");
  KernelFactory::instance()->registerKernel<Convection>("Convection");
  KernelFactory::instance()->registerKernel<ExampleImplicitEuler>("ExampleImplicitEuler");
  MaterialFactory::instance()->registerMaterial<ExampleMaterial>("ExampleMaterial");

  // Automatically look for a dump option on the commandline
  Parser p(moose_system);

  std::string input_filename = "";
  if ( Moose::command_line->search("-i") )
    input_filename = Moose::command_line->next(input_filename);
  else
    mooseError("Must specify an input file using -i");

  // Use the parser
  p.parse(input_filename);
  p.execute();

  // Output the initial condition in whatever ways are specified
  moose_system.output_system(0, 0.0);

  if(!Moose::executioner)
      mooseError("Executioner not supplied!");

  Moose::executioner->setup();
  Moose::executioner->execute();
}
