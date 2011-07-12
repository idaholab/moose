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
#include "MooseInit.h"
#include "Executioner.h"
#include "Factory.h"

// Parser
#include "Parser.h"
#include "MooseSyntax.h"

// Example 13 Includes
#include "TransientHalf.h"
#include "ExampleDiffusion.h"
#include "Convection.h"
#include "ExampleImplicitEuler.h"
#include "ExampleMaterial.h"

// libMesh includes
#include "perf_log.h"

PerfLog Moose::perf_log("Example 13: Custom Executioner");

int main (int argc, char** argv)
{
  MooseInit init (argc, argv);
  Parser p;
  
  Moose::registerObjects();

  // Register our new executioner
  registerExecutioner(TransientHalf);
  registerKernel(ExampleDiffusion);
  registerKernel(Convection);
  registerKernel(ExampleImplicitEuler);
  registerMaterial(ExampleMaterial);

  // Associate Parser Syntax with specific MOOSE Actions
  Moose::associateSyntax(p);

  // Parse commandline and return inputfile filename if appropriate
  std::string input_filename = p.parseCommandLine();
  
  p.parse(input_filename);
  p.execute();

  Executioner *e = p.getExecutioner();
  e->execute();
}
