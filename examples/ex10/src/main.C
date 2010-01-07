// Local Includes
#include "ExampleIC.h"

//Moose Includes
#include "Moose.h"
#include "Parser.h"
#include "InitialConditionFactory.h"

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

// Initialize default Performance Logging
PerfLog Moose::perf_log("Example10");

// Begin the main program.
int main (int argc, char** argv)
{
  // Initialize Moose and any dependent libaries
  MooseInit init (argc, argv);

  // This registers a bunch of common objects that exist in Moose with the factories.
   // that includes several Kernels, BoundaryConditions, AuxKernels and Materials
  Moose::registerObjects();

  InitialConditionFactory::instance()->registerInitialCondition<ExampleIC>("ExampleIC");

  // Automatically look for a dump option on the commandline
  Parser p;

  std::string input_filename = "";
  if ( Moose::command_line->search("-i") )
    input_filename = Moose::command_line->next(input_filename);
  else
    mooseError("Must specify an input file using -i");

  // Use the parser
  p.parse(input_filename);
  p.execute();

  // Output the initial condition in whatever ways are specified
  Moose::output_system(0, 0.0);

  // Solve the system inside of Moose
  {
    TransientNonlinearImplicitSystem & system =
      Moose::equation_system->get_system<TransientNonlinearImplicitSystem>("NonlinearSystem");

    system.solve();
  }

  // Output the solution in whatever ways are specified
  Moose::output_system(1, 1.0);
}
