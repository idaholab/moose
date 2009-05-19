//Moose Includes
#include "Moose.h"
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

PerfLog Moose::perf_log("Example1");

// Begin the main program.
int main (int argc, char** argv)
{
  {
    // Initialize libMesh and any dependent libaries
    LibMeshInit init (argc, argv);

    // This registers a bunch of common objects that exist in Moose with the factories.
    // that includes several Kernels, BoundaryConditions, AuxKernels and Materials
    Moose::registerObjects();

    // Create the mesh object
    Mesh *mesh = new Mesh(2);

    // MUST set the global mesh!
    Moose::mesh = mesh;
    
    ExodusII_IO exreader(*mesh);

    exreader.read("square.e");

    mesh->prepare_for_use(false);

    //This is necessary for Dirichlet BCs
    mesh->boundary_info->build_node_list_from_side_list();

    mesh->print_info();
    
    EquationSystems equation_systems (*mesh);

    // MUST set the global equation_systems!
    Moose::equation_system = &equation_systems;
    
    TransientNonlinearImplicitSystem& system =
      equation_systems.add_system<TransientNonlinearImplicitSystem> ("NonlinearSystem");
    
    system.add_variable("u", FIRST, LAGRANGE);

    system.nonlinear_solver->residual = Moose::compute_residual;
    system.nonlinear_solver->jacobian = Moose::compute_jacobian;

    TransientExplicitSystem& aux_system =
      equation_systems.add_system<TransientExplicitSystem> ("AuxiliarySystem");

    equation_systems.init();
    equation_systems.print_info();

    //Initialize common data structures for Kernels
    Kernel::init(&equation_systems);
    BoundaryCondition::init();
    AuxKernel::init();

    Parameters params;

    Parameters left_bc_params;
    left_bc_params.set<Real>("value") = 0.0;

    Parameters right_bc_params;
    right_bc_params.set<Real>("value") = 1.0;
    
    KernelFactory::instance()->add("Diffusion", "diff", params, "u");
    BCFactory::instance()->add("DirichletBC", "left",  left_bc_params,  "u", 1);
    BCFactory::instance()->add("DirichletBC", "right", right_bc_params, "u", 2);
    MaterialFactory::instance()->add("EmptyMaterial", "empty", params, 1);

    system.solve();

    ExodusII_IO(*mesh).write_equation_systems("out.e", equation_systems);
  }

  return 0;
}
