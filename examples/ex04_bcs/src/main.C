/**
 * Example 4: Boundary Conditions
 * This example demonstrates Boundary Condition Coupling and active syntax in input file
 * This example augments the third example (Convection-Diffusion) by adding custom boundary
 * conditions selectable by changing the input file
 */

// Moose Includes
#include "Parser.h"
#include "Executioner.h"
#include "MooseSystem.h"
#include "KernelFactory.h"
#include "BCFactory.h"       // <- Include BCFactory to register bcs

// Example 4 Includes
#include "Convection.h"
#include "CoupledDirichletBC.h"
#include "CoupledNeumannBC.h"

// C++ include files
#include <iostream>

// libMesh includes
#include "perf_log.h"

// Create a performance log
PerfLog Moose::perf_log("Example 4: Boundary Conditions");

 // Begin the main program.
int main (int argc, char** argv)
{
  MooseInit init (argc, argv);

  MooseSystem moose_system;

  Moose::registerObjects();

  KernelFactory::instance()->registerKernel<Convection>("Convection");
  
  /**
   * In this example we need to register our Boundary Conditions with BCFactory.
   * Each Main MOOSE Module has it's own factory that you will register your objects with
   */
  BCFactory::instance()->registerBC<CoupledDirichletBC>("CoupledDirichletBC");
  BCFactory::instance()->registerBC<CoupledNeumannBC>("CoupledNeumannBC");

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
