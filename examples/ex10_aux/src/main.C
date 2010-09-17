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
 * Example 10: Auxiliary Calcuations
 * This examples shows how to compute auxiliary values outside of the
 * non-linear system
 */

// Moose Includes
#include "Moose.h"
#include "MooseSystem.h"
#include "Parser.h"
#include "MooseFactory.h"
#include "Executioner.h"

// Example 10 Includes
#include "ExampleAux.h"

// C++ include files that we need
#include <iostream>

// libMesh includes
#include "perf_log.h"

PerfLog Moose::perf_log("Example 10: Auxiliary Calculations");

int main (int argc, char** argv)
{
  MooseInit init (argc, argv);

  MooseSystem moose_system;
  
  Moose::registerObjects();

  // Register our Example AuxKernel with the AuxFactory
  registerAux(ExampleAux);
  
  Parser p(moose_system);

  std::string input_filename = "";
  if ( Moose::command_line->search("-i") )
    input_filename = Moose::command_line->next(input_filename);
  else
    mooseError("Must specify an input file using -i");

  p.parse(input_filename);
  p.execute();

  Executioner &e = moose_system.getExecutioner();
  e.setup();
  e.execute();
}
