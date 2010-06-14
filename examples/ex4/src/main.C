/**
 * Example 4 - Demonstrating Boundary Condition Coupling and active syntax in input file
 * This example augments the third example (Convection-Diffusion) by adding custom boundary
 * conditions selectable by changing the input file
 */

//Moose Includes
#include "Parser.h"
#include "Executioner.h"
#include "MooseSystem.h"
// Moose Registration
#include "KernelFactory.h"
#include "Convection.h"
#include "BCFactory.h"
#include "CoupledDirichletBC.h"
#include "CoupledNeumannBC.h"

// C++ include files
#include <iostream>

// libMesh includes
#include "perf_log.h"

// Create a performance log
PerfLog Moose::perf_log("Example 4");

 // Begin the main program.
int main (int argc, char** argv)
{
  // Create a MooseInit Object
  MooseInit init (argc, argv);

  // Create a single MooseSystem which can hold
  // a single nonlinear system and single auxillary system
  MooseSystem moose_system;

  // Register a bunch of common objects that exist inside of Moose.
  // You will generally create a registerObjects method of your own
  // to register modules that you create in your own application where
  // you will generally call this method.
  Moose::registerObjects();

  // Register any custom objects you have built on the MOOSE Framework
  KernelFactory::instance()->registerKernel<Convection>("Convection");
  
  // Register a new boundary condition with the factory so we can use it in the computation.
  BCFactory::instance()->registerBC<CoupledDirichletBC>("CoupledDirichletBC");
  BCFactory::instance()->registerBC<CoupledNeumannBC>("CoupledNeumannBC");

  // Create the input file parser which takes a reference to the main
  // MooseSystem
  Parser p(moose_system);
  
  // Do some bare minimum command line parsing to look for a filename
  // to parse
  std::string input_filename = "";
  if ( Moose::command_line->search("-i") )
    input_filename = Moose::command_line->next(input_filename);
  else
    mooseError("Must specify an input file using -i");      

  // Tell the parser to parse the given file to setup the simulation and execute
  p.parse(input_filename);
  p.execute();

  if(!Moose::executioner)
    mooseError("Executioner not supplied!");

  // Run the executioner once the problem has been setup by the parser
  Moose::executioner->setup();
  Moose::executioner->execute();
}
