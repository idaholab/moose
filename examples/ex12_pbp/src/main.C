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
 * Example 12: Physics Based Preconditioning
 * This example shows how to enable the use of more advanced preconditioning
 * with the optional Kernel::computeQpOffDiagJacobian method and input PBP block
 */

//Moose Includes
#include "MooseInit.h"
#include "Executioner.h"

// Parser
#include "Parser.h"
#include "MooseSyntax.h"

// libMesh includes
#include "perf_log.h"

PerfLog Moose::perf_log("Example12: Physics Based Preconditioning");

int main (int argc, char** argv)
{
  MooseInit init (argc, argv);

  Moose::registerObjects();

  Parser p;

  std::string input_filename = "";
  if ( Moose::command_line->search("-i") )
    input_filename = Moose::command_line->next(input_filename);
  else
    p.printUsage();
  
  // Associate Parser Syntax with specific MOOSE Actions
  Moose::associateSyntax(p);
  
  p.parse(input_filename);
  p.execute();
  
  Executioner *e = p.getExecutioner();
  e->execute();

  return 0;
}
