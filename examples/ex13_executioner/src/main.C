/**
 * Example 13: Custom Executioner
 * This example uses the modular execution system in MOOSE
 * to plugin a new executioner.  This example extendes the transient
 * executioner to change the timestep in a fixed way over the
 * course of the simulation
 */

//Moose Includes
#include "Moose.h"
#include "MooseSystem.h"
#include "Parser.h"
#include "Executioner.h"
#include "ExecutionerFactory.h"
#include "KernelFactory.h"
#include "MaterialFactory.h"

// Example 13 Includes
#include "TransientHalf.h"
#include "ExampleDiffusion.h"
#include "Convection.h"
#include "ExampleImplicitEuler.h"
#include "ExampleMaterial.h"

// C++ include files that we need
#include <iostream>

// libMesh includes
#include "perf_log.h"

PerfLog Moose::perf_log("Example 13: Custom Executioner");

int main (int argc, char** argv)
{
  MooseInit init (argc, argv);

  MooseSystem moose_system;

  Moose::registerObjects();

  // Register our new executioner
  ExecutionerFactory::instance()->registerExecutioner<TransientHalf>("TransientHalf");
  KernelFactory::instance()->registerKernel<ExampleDiffusion>("ExampleDiffusion");
  KernelFactory::instance()->registerKernel<Convection>("Convection");
  KernelFactory::instance()->registerKernel<ExampleImplicitEuler>("ExampleImplicitEuler");
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
