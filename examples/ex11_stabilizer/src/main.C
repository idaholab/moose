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
 * Example 11 - Stabilization
 * Demonstrates using one of the built in stabalizers inside of MOOSE.  That
 * means that there is no additional registration necessary
 */

//Moose Includes
#include "MooseInit.h"
#include "Parser.h"
#include "Executioner.h"
#include "Factory.h"

// Example 11 Includes
#include "ExampleDiffusion.h"
#include "Convection.h"
#include "ExampleMaterial.h"

// libMesh includes
#include "perf_log.h"

PerfLog Moose::perf_log("Example 11: Stabilization");

int main (int argc, char** argv)
{
  MooseInit init (argc, argv);

  Moose::registerObjects();

  registerKernel(Convection);
  registerKernel(ExampleDiffusion);
  registerMaterial(ExampleMaterial);

  Parser p;
  
  std::string input_filename = "";
  if ( Moose::command_line->search("-i") )
    input_filename = Moose::command_line->next(input_filename);
  else
    p.printUsage();

  p.parse(input_filename);
  p.execute();

  Executioner *e = p.getExecutioner();
  e->execute();

  return 0;
}
