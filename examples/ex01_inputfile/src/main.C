/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

/**
 * Example 1: Input File - The smallest MOOSE based application possible.  It solves
 * a simple 2D diffusion problem with Dirichlet boundary conditions using built-in
 * objects from MOOSE.
 */

//Moose Includes
#include "Parser.h"
#include "Executioner.h"
#include "MooseSystem.h"

// C++ include files
#include <iostream>

// libMesh includes
#include "perf_log.h"

// Create a performance log
PerfLog Moose::perf_log("Example 1: Input File");

// Begin the main program.
int main (int argc, char** argv)
{
  // Create a MooseInit Object
  MooseInit init (argc, argv);

  // Create a single MooseSystem which can hold
  // a single nonlinear system and single auxillary system
  MooseSystem moose_system;

  // Register a bunch of common objects that exist inside of Moose.  You will 
  // generally create a registerObjects method of your own to register modules
  // that you create in your own application.
  Moose::registerObjects();

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

  Executioner &e = moose_system.getExecutioner();
  e.setup();
  e.execute();
}
