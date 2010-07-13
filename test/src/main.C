/**
 * Moose Test Application
 */

#include "MooseTest.h"

//Moose Includes
#include "Parser.h"
#include "Executioner.h"
#include "MooseSystem.h"

// C++ include files
#include <iostream>

// libMesh includes
#include "perf_log.h"

// Create a performance log
PerfLog Moose::perf_log("Moose Test");

 // Begin the main program.
int main (int argc, char** argv)
{
  MooseInit init (argc, argv);
  MooseSystem moose_system;


  Moose::registerObjects();
  
  MooseTest::registerObjects();
  
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
