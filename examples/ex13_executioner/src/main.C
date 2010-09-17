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
#include "MooseFactory.h"

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
  registerExecutioner(TransientHalf);
  registerKernel(ExampleDiffusion);
  registerKernel(Convection);
  registerKernel(ExampleImplicitEuler);
  registerMaterial(ExampleMaterial);

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
