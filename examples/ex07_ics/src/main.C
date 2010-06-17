/**
 * Example 7: Initial Conditions
 * This example demonstrates how to create and apply custom initial condition object
 * for one of our simulation variables
 */

//Moose Includes
#include "Parser.h"
#include "Executioner.h"
#include "MooseSystem.h"
#include "InitialConditionFactory.h"

// Example 7 Includes
#include "ExampleIC.h"

// C++ include files that we need
#include <iostream>

// libMesh includes
#include "perf_log.h"

// Initialize default Performance Logging
PerfLog Moose::perf_log("Example 7: Initial Condition");

int main (int argc, char** argv)
{
  MooseInit init (argc, argv);

  MooseSystem moose_system;

  Moose::registerObjects();

  // Register our custom Initial Condition with the InitialConditionFactory
  InitialConditionFactory::instance()->registerInitialCondition<ExampleIC>("ExampleIC");

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
