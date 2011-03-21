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
 * Example 9: Stateful Material Properties
 * Demonstrates using material properties computed at previous timesteps
 *
 * Note: AMR cannot be used with stateful material properties and
 * the simulation will run slower
 */

//Moose Includes
#include "MooseInit.h"
#include "Parser.h"
#include "Executioner.h"
#include "Factory.h"

// Example 9 Includes
#include "ExampleDiffusion.h"
#include "Convection.h"
#include "ExampleMaterial.h"

// libMesh includes
#include "perf_log.h"

PerfLog Moose::perf_log("Example 9: Stateful Material Properties");

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
}
