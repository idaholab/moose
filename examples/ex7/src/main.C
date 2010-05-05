//Local Includes
#include "Convection.h"
#include "ExampleDiffusion.h"
#include "ExampleMaterial.h"
#include "ExampleImplicitEuler.h"

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
#include "numeric_vector.h"
#include "mesh_refinement.h"
#include "getpot.h"

// Initialize default Performance Logging
PerfLog Moose::perf_log("Example7");

// Begin the main program.
int main (int argc, char** argv)
{
  // Initialize Moose
  MooseInit init(argc, argv);

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

  // Create a parser for the input file
  GetPot input_file(input_filename);

  // The diffusivity we're going to pass to our Material
  // We're giving it a default value of 1.0
  Real diffusivity = input_file("MatProps/diffusivity",1.0);

  // The time_coefficient we're going to pass to our Material
  // We're giving it a default value of 1.0
  Real time_coefficient = input_file("MatProps/time_coefficient",1.0);

  // The timestep size to take... defaulting to 1.0
  Real input_dt = input_file("Transient/dt", 1.0);

  // This registers a bunch of common objects that exist in Moose with the factories.
  // that includes several Kernels, BoundaryConditions, AuxKernels and Materials
  Moose::registerObjects();

  // Register a new kernel with the factory so we can use it in the computation.
  KernelFactory::instance()->registerKernel<Convection>("Convection");
  KernelFactory::instance()->registerKernel<ExampleDiffusion>("ExampleDiffusion");
  KernelFactory::instance()->registerKernel<ExampleImplicitEuler>("ExampleImplicitEuler");

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
  mesh_refinement.uniformly_refine(4);

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

  // Initialize the systems
  moose_system.init();

  /**
   * Set up Transient parameters.  For a Transient solve time, dt and t_step
   * MUST exist and must be set before calling MooseSystem::init()!
   */

  // The starting time
  Real & time = moose_system.parameters().set<Real> ("time") = 0;

  // The step size
  Real & dt = moose_system.parameters().set<Real> ("dt") = input_dt;

  // The starting step
  int & t_step = moose_system.parameters().set<int> ("t_step") = 0;

  // Number of timesteps to take
  unsigned int num_steps = 20;

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

  // Add an ImplicitEuler Kernel for the time operator
  moose_system.addKernel("ImplicitEuler", "u_ie", diff_params);

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

  // Add an ImplicitEuler Kernel for the time operator
  moose_system.addKernel("ExampleImplicitEuler", "v_ie", diff_params);

  // Add the two boundary conditions using the DirichletBC object from MOOSE
  left_bc_params.set<std::string>("variable") = "v";
  moose_system.addBC("DirichletBC", "left", left_bc_params);

  right_bc_params.set<std::string>("variable") = "v";
  moose_system.addBC("DirichletBC", "right", right_bc_params);


  // Get the default values for the ExampleMaterial's parameters
  InputParameters mat_params = MaterialFactory::instance()->getValidParams("ExampleMaterial");

  // Override the default diffusivity
  mat_params.set<Real>("diffusivity") = diffusivity;

  // Override the default time_coefficient
  mat_params.set<Real>("time_coefficient") = time_coefficient;
  mat_params.set<std::vector<unsigned int> >("block") = std::vector<unsigned int>(1, 1);

  // Add the Example material into the calculation using the new diffusivity.
  moose_system.addMaterial("ExampleMaterial", "example", mat_params);

  // Create an output object that we can write to each timestep
  // Note that this ONLY works with Exodus!
  ExodusII_IO ex_out(mesh);

  // Write out the initial condition
  // The +1 is because Exodus starts counting from 1
  ex_out.write_timestep("out.e", *moose_system.getEquationSystems(), t_step+1, time);

  for(t_step = 1; t_step <= num_steps; ++t_step)
  {
    // Solve for the next timestep
    time += dt;

    // Print out what timestep we're at and the current time
    std::cout<<std::endl<<"Solving Timestep "<<t_step<<" at time "<<time<<std::endl;

    // After time, dt or t_step change reinitDT MUST be called
    moose_system.reinitDT();

    // Copy the old solutions backwards
    moose_system.copy_old_solutions();

    // Solve the Nonlinear System for this timestep
    moose_system.solve();

    // Write the solution out.
    // The +1 is because Exodus numbering starts at 1
    ex_out.write_timestep("out.e", *moose_system.getEquationSystems(), t_step+1, time);
  }

  return 0;
}
