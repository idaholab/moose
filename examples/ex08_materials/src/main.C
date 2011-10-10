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
 * Example 8: Material Properties
 * Demonstrates making a Material Class and using a Material Property in a Residual
 * Calculation.
 */

//Moose Includes
#include "MooseInit.h"
#include "Executioner.h"
#include "Factory.h"

// Parser
#include "Parser.h"
#include "MooseSyntax.h"

// Example 8 Includes
#include "ExampleDiffusion.h"
#include "Convection.h"
#include "ExampleMaterial.h"

// libMesh includes
#include "perf_log.h"

// Create a performance log
PerfLog Moose::perf_log("Example 8: Material Properties");

 // Begin the main program.
int main (int argc, char** argv)
{
  MooseInit init (argc, argv);
  Parser p;

  Moose::registerObjects();
  registerKernel(Convection);
  // Our new Diffusion Kernel that accepts a material property
  registerKernel(ExampleDiffusion);
  // Register our new material class so we can use it.
  registerMaterial(ExampleMaterial);

  // Associate Parser Syntax with specific MOOSE Actions
  Moose::associateSyntax(p);

  // Parse commandline and return inputfile filename if appropriate
  std::string input_filename = p.parseCommandLine();

  p.parse(input_filename);
  p.execute();

  Moose::executioner->execute();

  return 0;
}
