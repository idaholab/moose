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
 * Example 5: Adaptive Mesh Refinement:
 * This example shows how to enable AMR in the input file.
 *
 * Note: This file contains no additional changes from the previous example
 */

//Moose Includes
#include "MooseInit.h"
#include "Executioner.h"
#include "Factory.h"

// Parser
#include "Parser.h"
#include "MooseSyntax.h"

// Example 5 Registration
#include "Convection.h"

// libMesh includes
#include "perf_log.h"

PerfLog Moose::perf_log("Example 5: Adaptive Mesh Refinement");

int main (int argc, char** argv)
{
  MooseInit init (argc, argv);
  Parser p;

  Moose::registerObjects();

  registerKernel(Convection);
  // Associate Parser Syntax with specific MOOSE Actions
  Moose::associateSyntax(p);

  // Parse commandline and return inputfile filename if appropriate
  std::string input_filename = p.parseCommandLine();

  p.parse(input_filename);
  p.execute();

  Executioner *e = p.getExecutioner();
  e->execute();

  return 0;
}
