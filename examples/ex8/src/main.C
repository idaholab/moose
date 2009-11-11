//Moose Includes
#include "Moose.h"
#include "Parser.h"

// C++ include files that we need
#include <iostream>
#include <fstream>

// libMesh includes
#include "libmesh.h"
#include "perf_log.h"
#include "mesh.h"
//#include "boundary_info.h"
#include "exodusII_io.h"
#include "equation_systems.h"
//#include "nonlinear_solver.h"
//#include "nonlinear_implicit_system.h"
//#include "linear_implicit_system.h"
#include "transient_system.h"
//#include "numeric_vector.h"
//#include "mesh_refinement.h"
#include "getpot.h"

// Initialize default Performance Logging
PerfLog Moose::perf_log("Example8");

// Begin the main program.
int main (int argc, char** argv)
{
// Initialize libMesh and any dependent libaries
  LibMeshInit init (argc, argv);

   // This registers a bunch of common objects that exist in Moose with the factories.
   // that includes several Kernels, BoundaryConditions, AuxKernels and Materials
  Moose::registerObjects();
  
   // Create a GetPot object to parse the command line
  GetPot command_line (argc, argv);
    
   // Will hold the name of the input file... default to blank
  std::string input_filename = "";
    
   // See if an input file was provided on the command-line
  if ( command_line.search("-i") )
    input_filename = command_line.next(input_filename);
  else
  {
     // Print a message and throw an error if the input file wasn't provided
     std::cout<<"Must specify an input file using -i"<<std::endl;
     libmesh_error();
  }

  Parser p = Parser(input_filename);
  p.execute();

  // Solve the system inside of Moose
  {
    TransientNonlinearImplicitSystem & system =
      Moose::equation_system->get_system<TransientNonlinearImplicitSystem>("NonlinearSystem");

    system.solve();
  }

  // Only bother with exodus outputs
  if (Moose::exodus_output)
    ExodusII_IO(*Moose::mesh).write_equation_systems(Moose::file_base + ".e", *Moose::equation_system);
  else
    std::cout << "Exodus Output not selected and no other output method implemented\n";
}
