#ifdef MFEM_ENABLED

#include "MFEMObjectUnitTest.h"
#include "MFEMScalarDirichletBC.h"
#include "MFEMDiffusionKernel.h"
#include "MFEMHypreBoomerAMG.h"
#include "equation_system_problem_operator.h"
#include "MFEMParaViewDataCollection.h"
#include "MFEMEstimator.h"

static double SolveEquationAndCheckResidual(
  std::unique_ptr<platypus::EquationSystemProblemOperator> &,
  std::shared_ptr<platypus::EquationSystem> &,
  mfem::BlockVector &
);

class MFEMMeshRefinementTest : public MFEMObjectUnitTest
{
public:
  MFEMMeshRefinementTest() : MFEMObjectUnitTest("PlatypusApp")
  {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    mfem::ParMesh & pmesh = _mfem_mesh_ptr->getMFEMParMesh();
    int dim               = pmesh.Dimension();
    int sdim              = pmesh.SpaceDimension();
    
    int order = 1;
    
    // construct problem operator
    auto & problem_data      = _mfem_problem->getProblemData();
    problem_data.eqn_system = std::make_shared<platypus::EquationSystem>();
    auto eqn_system          = problem_data.eqn_system;
    // FE space
    InputParameters fespace_params             = _factory.getValidParams("MFEMScalarFESpace");
    fespace_params.set<MooseEnum>("fec_type")  = "H1";
    fespace_params.set<MooseEnum>("fec_order") = "FIRST";
    _mfem_problem->addFESpace("MFEMScalarFESpace", "H1FESpace", fespace_params);
    
    // bottom BC
    InputParameters bc_bottom_params              = _factory.getValidParams("MFEMScalarDirichletBC");
    bc_bottom_params.set<std::string>("variable") = "diffused";
    bc_bottom_params.set<Real>("value")           = 1.;
    bc_bottom_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
    _mfem_problem->addBoundaryCondition("MFEMScalarDirichletBC", "bottom", bc_bottom_params);
    
    // low terminal BC
    InputParameters bc_low_terminal_params              = _factory.getValidParams("MFEMScalarDirichletBC");
    bc_low_terminal_params.set<std::string>("variable") = "diffused";
    bc_low_terminal_params.set<Real>("value")           = 0.;
    bc_low_terminal_params.set<std::vector<BoundaryName>>("boundary") = {"2"};
    _mfem_problem->addBoundaryCondition("MFEMScalarDirichletBC", "low_terminal", bc_low_terminal_params);
    
    
    // diffusion coefficient material
    InputParameters coef_params = _factory.getValidParams("MFEMGenericConstantMaterial");
    coef_params.set<std::vector<std::string>>("prop_names") = {"diffusivity"};
    coef_params.set<std::vector<double>>("prop_values")     = {1.0};
    _mfem_problem->addMaterial("MFEMGenericConstantMaterial", "material1", coef_params);
    
    // diffusion kernel
    InputParameters kernel_params                   = _factory.getValidParams("MFEMDiffusionKernel");
    kernel_params.set<std::string>("variable")      = "diffused";
    kernel_params.set<std::string>("coefficient")   = "diffusivity";
    _mfem_problem->addKernel("MFEMDiffusionKernel", "kernel1", kernel_params);
    
    // construct precon
    InputParameters precon_params      = _factory.getValidParams("MFEMHypreBoomerAMG");
    precon_params.set<double>("l_tol") = 1e-7; // HypreBoomerAMG cannot set absolute tolerance
    _mfem_problem->addMFEMPreconditioner("MFEMHypreBoomerAMG", "precon1", precon_params);
    
    // construct solver
    InputParameters solver_params          = _factory.getValidParams("MFEMHyprePCG");
    solver_params.set<double>("l_tol")     = 1e-7;
    solver_params.set<double>("l_abs_tol") = 1e-5;
    _mfem_problem->addMFEMSolver("MFEMHyprePCG", "solver1", solver_params);
    
    // next, the variables
    InputParameters var_params                = _factory.getValidParams("MFEMVariable");
    var_params.set<UserObjectName>("fespace") = "H1FESpace";
    _mfem_problem->addVariable("MFEMVariable", "diffused", var_params);
    
    _mfem_problem->addMFEMNonlinearSolver();
    
  }
};

/// Test based on MFEM example 6
TEST_F(MFEMMeshRefinementTest, DiffusionRefinement)
{
  // fetch the references which we init during the constructor
  auto & problem_data = _mfem_problem->getProblemData();
  auto eqn_system          = problem_data.eqn_system;

  // Finished initialisation...
  mfem::BlockVector    X;

  eqn_system->Init(
    problem_data.gridfunctions,
    problem_data.fespaces,
    mfem::AssemblyLevel::LEGACY
  );

  // problem operator
  auto problem_operator = std::make_unique<platypus::EquationSystemProblemOperator>(problem_data);
  problem_operator->SetGridFunctions();
  problem_operator->Init( X );
  
  auto residual = SolveEquationAndCheckResidual( problem_operator, eqn_system, X );
  ASSERT_LE(residual, 1E-5);

  // Refine the mesh
  problem_operator->UniformRefinement();

  residual = SolveEquationAndCheckResidual( problem_operator, eqn_system, X );
  ASSERT_LE(residual, 1E-5);
}


/*++++++++++++++++++++++++++++ HELPER FUNCTIONS ++++++++++++++++++++++++++++*/

static double SolveEquationAndCheckResidual(
  std::unique_ptr<platypus::EquationSystemProblemOperator> & problem_operator,
  std::shared_ptr<platypus::EquationSystem> &                eqn_system,
  mfem::BlockVector & X
) {
  // solve!
  problem_operator->Solve( X );

  // Check L2 norm of residual
  mfem::Vector Y( problem_operator->_true_x.Size() );
  eqn_system->Mult( problem_operator->_true_x, Y );
  Y -= problem_operator->_true_rhs;

  return Y.Norml2();
}


#endif
