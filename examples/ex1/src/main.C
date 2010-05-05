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

// Initialize default Performance Logging
PerfLog Moose::perf_log("Example1");

// Begin the main program.
int main (int argc, char** argv)
{
  {
    // Initialize Moose
    MooseInit init(argc, argv);

    // This registers a bunch of common objects that exist in Moose with the factories.
    // that includes several Kernels, BoundaryConditions, AuxKernels and Materials
    Moose::registerObjects();

    // Create the mesh object
    Mesh mesh(2);
    // Read the mesh from an Exodus file and prepare it for use
    ExodusII_IO exreader(mesh);
    exreader.read("square.e");

    // Create a MooseSystem
    MooseSystem moose_system(mesh);

    // Print some useful information about the mesh
    mesh.print_info();

    /**
     * Add a variable named "u" to solve for.
     * We are going to approximate it using First order Lagrange FEs
     */
    moose_system.addVariable("u", FIRST, LAGRANGE);

    // Initialize the moose_system
    moose_system.init();

    // Add a Diffusion Kernel from MOOSE into the calculation.
    InputParameters diff_params;
    diff_params.addParam<std::string>("variable", "u", "variable for which to apply this kernel");

    moose_system.addKernel("Diffusion", "diff", diff_params);

    // Parameters for DirichletBC's
    InputParameters left_bc_params;
    std::vector<unsigned int> boundary_ids(1);
    boundary_ids[0] = 1;
    left_bc_params.addParam("value", 0.0, "value on the left boundary") ;
    left_bc_params.addParam("boundary", boundary_ids, "exodus boundary number for which to apply this BC");
    left_bc_params.addParam<std::string>("variable", "u", "variable for which to apply this BC");

    InputParameters right_bc_params;
    boundary_ids[0] = 2;
    right_bc_params.addParam("value", 1.0, "value on the right boundary");
    right_bc_params.addParam("boundary", boundary_ids, "exodus boundary number for which to apply this BC");
    right_bc_params.addParam<std::string>("variable", "u", "variable for which to apply this BC");

    // Add the two boundary conditions using the DirichletBC object from MOOSE
    moose_system.addBC("DirichletBC", "left", left_bc_params);
    moose_system.addBC("DirichletBC", "right", right_bc_params);

    // Every calculation MUST add at least one Material
    // Here we use the EmptyMaterial from MOOSE because we don't need material properties.
    InputParameters material_params;
    material_params.addParam("block", std::vector<unsigned int>(1, 1), "block that this material is associated with");
    moose_system.addMaterial("EmptyMaterial", "empty", material_params);

    // Solve the Nonlinear System
    moose_system.solve();

    // Write the solution out.
    moose_system.output_system(0, 1.);
  }

  return 0;
}
