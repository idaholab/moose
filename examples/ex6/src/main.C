//Local Includes
#include "Convection.h"
#include "ExampleDiffusion.h"
#include "ExampleMaterial.h"

//Moose Includes
#include "Moose.h"
#include "MooseSystem.h"
#include "KernelFactory.h"
#include "BCFactory.h"
#include "MaterialFactory.h"
#include "AuxFactory.h"
#include "ComputeResidual.h"
#include "ComputeJacobian.h"

// C++ include files that we need
#include <iostream>
#include <fstream>

// libMesh includes
#include "libmesh.h"
#include "perf_log.h"
#include "mesh.h"
#include "boundary_info.h"
#include "exodusII_io.h"
#include "equation_systems.h"
#include "nonlinear_solver.h"
#include "nonlinear_implicit_system.h"
#include "linear_implicit_system.h"
#include "transient_system.h"
#include "getpot.h"
#include "mesh_refinement.h"
#include "error_vector.h"
#include "kelly_error_estimator.h"
#include "gmv_io.h"

// Initialize default Performance Logging
PerfLog Moose::perf_log("Example6");

// Begin the main program.
int main (int argc, char** argv)
{
  // Initialize Moose
  MooseInit init(argc, argv);

  // Create a GetPot object to parse the command line
  GetPot command_line (argc, argv);

  // The diffusivity we're going to pass to our Material
  // We're giving it a default value of 1.0
  Real diffusivity = 0.1;

  // See if a diffusivity was provided on the command-line
  if(command_line.search("--diffusivity"))
    diffusivity = command_line.next(diffusivity);

  // This registers a bunch of common objects that exist in Moose with the factories.
  // that includes several Kernels, BoundaryConditions, AuxKernels and Materials
  Moose::registerObjects();

  // Register a new kernel with the factory so we can use it in the computation.
  KernelFactory::instance()->registerKernel<Convection>("Convection");
  KernelFactory::instance()->registerKernel<ExampleDiffusion>("ExampleDiffusion");

  // Register our new material class so we can use it.
  MaterialFactory::instance()->registerMaterial<ExampleMaterial>("ExampleMaterial");

  // Create the mesh object
  Mesh mesh(2);
  // Read the mesh from an Exodus file and prepare it for use
  ExodusII_IO exreader(mesh);
  exreader.read("square.e");

  /**
   * Do some uniform refinement of the mesh so we can capture the solution better.
   */
  MeshRefinement mesh_refinement(mesh);
  mesh_refinement.uniformly_refine(1);

  /**
   * Setup the percentage of elements to refine and coarsen and the
   * maximum refinement level.
   */
  mesh_refinement.refine_fraction()  = .3;
  mesh_refinement.coarsen_fraction() = 0;
  mesh_refinement.max_h_level()      = 10;

  // The number of refinement steps we want to do
  unsigned int max_r_steps = 7;

  // Print some useful information about the mesh
  mesh.print_info();

  // Create a MooseSystem
  MooseSystem moose_system(mesh);

  /**
   * Add a variable named "u" to solve for.
   * We are going to approximate it using First order Lagrange FEs
   */
  moose_system.addVariable("u", FIRST, LAGRANGE);

  /**
   * Add a variable named "v" to solve for.
   * We are going to approximate it using First order Lagrange FEs
   *
   * We are going to couple this variable into the convection kernel for variable "u"
   */
  moose_system.addVariable("v", FIRST, LAGRANGE);

  // Initialize the Systems and print some info out
  moose_system.init();

  /**
   * Next we are going to define our coupling vectors.
   *
   * coupled_to = the actual name of the coupled variable
   *
   * coupled_as = the name the Kernel _expects_...
   *   This is essentially the name you pass to the Coupled*()
   *   functions when you try to get a coupled variable in a Kernel.
   *   In this case we call coupledGrad("some_var") in Convection.h
   *   therefore the variable "v" is coupled_as "some_var.
   *
   * Note that coupled_to and coupled_as are _paired_.
   */
  std::vector<std::string> conv_coupled_to;
  std::vector<std::string> conv_coupled_as;

  conv_coupled_to.push_back("v");
  conv_coupled_as.push_back("some_var");


  //////////////
  // "u" Kernels
  //////////////

  // Add a Diffusion Kernel from MOOSE into the calculation.
  InputParameters diff_params;
  diff_params.addParam<std::string>("variable", "u", "variable for which to apply this kernel");
  moose_system.addKernel("ExampleDiffusion", "diff", diff_params);

  // Add the Convection Kernel from this application into the calculation
  // Note that we are passing in our coupling vectors
  InputParameters conv_params;
  conv_params.addParam<std::string>("variable", "u", "variable for which to apply this kernel");
  conv_params.addParam<std::vector<std::string> >("coupled_to", conv_coupled_to, "coupled to variable");
  conv_params.addParam<std::vector<std::string> >("coupled_as", conv_coupled_as, "coupled as variable");
  moose_system.addKernel("Convection", "conv", conv_params);

  // Add the two boundary conditions using the DirichletBC object from MOOSE
  InputParameters left_bc_params;
  std::vector<unsigned int> boundary_ids(1);
  boundary_ids[0] = 1;
  left_bc_params.addParam("value", 0.0, "value on the left boundary") ;
  left_bc_params.addParam("boundary", boundary_ids, "exodus boundary number for which to apply this BC");
  left_bc_params.addParam<std::string>("variable", "u", "variable for which to apply this BC");
  moose_system.addBC("DirichletBC", "left", left_bc_params);

  InputParameters right_bc_params;
  boundary_ids[0] = 2;
  right_bc_params.addParam("value", 1.0, "value on the right boundary");
  right_bc_params.addParam("boundary", boundary_ids, "exodus boundary number for which to apply this BC");
  right_bc_params.addParam<std::string>("variable", "u", "variable for which to apply this BC");
  moose_system.addBC("DirichletBC", "right", right_bc_params);


  //////////////
  // "v" Kernels
  //////////////

  // Add a Diffusion Kernel from MOOSE into the calculation.
  diff_params.set<std::string>("variable") = "v";
  moose_system.addKernel("Diffusion", "diff", diff_params);

  // Add the two boundary conditions using the DirichletBC object from MOOSE
  left_bc_params.set<std::string>("variable") = "v";
  moose_system.addBC("DirichletBC", "left",  left_bc_params);

  right_bc_params.set<std::string>("variable") = "v";
  moose_system.addBC("DirichletBC", "right", right_bc_params);

  // Get the default values for the ExampleMaterial's parameters
  InputParameters mat_params = MaterialFactory::instance()->getValidParams("ExampleMaterial");

  // Override the default diffusivity
  mat_params.set<Real>("diffusivity") = diffusivity;
  mat_params.set<std::vector<unsigned int> >("block") = std::vector<unsigned int>(1, 1);

  // Add the Example material into the calculation using the new diffusivity.
  moose_system.addMaterial("ExampleMaterial", "example", mat_params);

  {
    // Holds a scalar per element representing the error in that element
    ErrorVector error;

    // Gradient Jump based error estimator... others exist in libMesh
    KellyErrorEstimator error_estimator;

    // Note the <= to make sure we solve at least once
    for(unsigned int i=0; i<= max_r_steps; ++i)
    {
      // Solve the Nonlinear System
      moose_system.solve();

      if(i != max_r_steps)
      {
        error_estimator.estimate_error(*moose_system.getNonlinearSystem(), error);

        // Flag elements to be refined and coarsened
        mesh_refinement.flag_elements_by_error_fraction (error);

        // Perform refinement and coarsening
        mesh_refinement.refine_and_coarsen_elements();

        // Tell MOOSE that the mesh changed so it can reinitialize internal data structures.
        moose_system.meshChanged();

        // Print out mesh info so we can see how many elements / nodes we have now
        mesh.print_info();
      }
    }
  }

  // Write the solution out.
  moose_system.output_system(0, 1);

  return 0;
}
