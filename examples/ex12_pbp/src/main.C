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
 * Example 13: Physics Based Preconditioning
 * This example shows how to enable the use of more advanced preconditioning
 * with the optional Kernel::computeQpOffDiagJacobian method and input PBP block
 */

//Moose Includes
#include "Moose.h"
#include "MooseSystem.h"
#include "Parser.h"
#include "Executioner.h"

// C++ include files that we need
#include <iostream>
#include <fstream>

// libMesh includes
#include "perf_log.h"
#include "mesh.h"

#include "exodusII_io.h"
#include "equation_systems.h"
#include "transient_system.h"
#include "getpot.h"

PerfLog Moose::perf_log("Example13: Physics Based Preconditioning");

int main (int argc, char** argv)
{
  MooseInit init (argc, argv);

  MooseSystem moose_system;

  Moose::registerObjects();

  Parser p(moose_system);

  std::string input_filename = "";
  if ( Moose::command_line->search("-i") )
    input_filename = Moose::command_line->next(input_filename);
  else
    mooseError("Must specify an input file using -i");

  p.parse(input_filename);
  p.execute();

  Executioner &e = p.getExecutioner();
  e.setup();
  e.execute();
}
