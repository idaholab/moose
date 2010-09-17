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
 * Example 3: Coupling - Demonstrating Variable Coupling
 * This example augments the second example (Convection w/velocity) by coupling
 * in a variable 'v' into the residual calculation of the Convection operator
 *
 * Note: This main subroutine contains no additional code to enable coupling
 */

// Moose Includes
#include "Parser.h"
#include "Executioner.h"
#include "MooseSystem.h"

// Example 3 Includes
#include "MooseFactory.h"
#include "Convection.h"

// C++ include files
#include <iostream>

// libMesh includes
#include "perf_log.h"

// Create a performance log
PerfLog Moose::perf_log("Example 3: Coupling");

// Begin the main program.
int main (int argc, char** argv)
{
  MooseInit init (argc, argv);
  
  MooseSystem moose_system;

  Moose::registerObjects();

  registerKernel(Convection);

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
