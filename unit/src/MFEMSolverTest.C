//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMObjectUnitTest.h"
#include "MFEMScalarDirichletBC.h"
#include "MFEMDiffusionKernel.h"
#include "MFEMDomainLFKernel.h"
#include "MFEMHypreGMRES.h"
#include "MFEMHypreFGMRES.h"
#include "MFEMHyprePCG.h"
#include "MFEMHypreBoomerAMG.h"
#include "MFEMHypreADS.h"
#include "MFEMHypreAMS.h"
#include "MFEMSuperLU.h"
#include "MFEMMUMPS.h"
#include "MFEMGMRESSolver.h"
#include "MFEMCGSolver.h"
#include "MFEMOperatorJacobiSmoother.h"
#include "MFEMVectorFESpace.h"

class MFEMSolverTest : public MFEMObjectUnitTest
{
public:
  MFEMSolverTest() : MFEMObjectUnitTest("MooseUnitApp")
  {
    _pmesh = std::make_unique<mfem::ParMesh>(makeMesh());
    _fec = std::make_unique<mfem::H1_FECollection>(3, 3);
    _fespace = std::make_unique<mfem::ParFiniteElementSpace>(_pmesh.get(), _fec.get());
    _x = std::make_shared<mfem::ParGridFunction>(_fespace.get());
    *_x = 0.0;

    const VariableName var_name("test_variable_name");
    _mfem_problem->getProblemData().gridfunctions.Register(var_name, _x);
    _equation_system = std::make_shared<Moose::MFEM::EquationSystem>();
    _mfem_problem->getProblemData().eqn_system = _equation_system;
  }

  static mfem::real_t uexact(const mfem::Vector & x)
  {
    return x(2) * x(2) * x(2) - 5.0 * x(0) * x(0) * x(1) * x(2);
  }

  static void gradexact(const mfem::Vector & x, mfem::Vector & grad)
  {
    grad.SetSize(x.Size());
    grad[0] = -10.0 * x(0) * x(1) * x(2);
    grad[1] = -5.0 * x(0) * x(0) * x(2);
    grad[2] = 3.0 * x(2) * x(2) - 5.0 * x(0) * x(0) * x(1);
  }

  static mfem::real_t d2uexact(const mfem::Vector & x) // returns \Delta u
  {
    return -10.0 * x(1) * x(2) + 6.0 * x(2);
  }

  static mfem::real_t fexact(const mfem::Vector & x) // returns -\Delta u
  {
    return -d2uexact(x);
  }

  // Create a simple 3D mesh for testing
  mfem::ParMesh makeMesh()
  {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    const int ne = 4;
    mfem::Mesh mesh;
    mesh = mfem::Mesh::MakeCartesian3D(ne, ne, ne, mfem::Element::HEXAHEDRON, 1.0, 1.0, 1.0);
    mfem::ParMesh pmesh(MPI_COMM_WORLD, mesh);
    mesh.Clear();
    return pmesh;
  }

  /**
   * Test a solver can solve a dummy diffusion problem to the expected tolerance.
   * Based on mfem/tests/unit/linalg/test_direct_solvers.cpp.
   */
  template <typename SolverType>
  void testDiffusionSolve(Moose::MFEM::LinearSolverBase & solver,
                          mfem::real_t tol,
                          bool solve_system = true)
  {
    _mfem_problem->getCoefficients().declareScalar<mfem::FunctionCoefficient>("f_exact", fexact);

    const VariableName var_name("test_variable_name");
    InputParameters bc_params = _factory.getValidParams("MFEMScalarDirichletBC");
    bc_params.set<VariableName>("variable") = var_name;
    bc_params.set<MFEMScalarCoefficientName>("coefficient") = "0.";
    auto essential_bcs =
        _mfem_problem->addObject<MFEMScalarDirichletBC>("MFEMScalarDirichletBC", "bc1", bc_params);

    InputParameters kernel1_params = _factory.getValidParams("MFEMDiffusionKernel");
    kernel1_params.set<VariableName>("variable") = var_name;
    kernel1_params.set<MFEMScalarCoefficientName>("coefficient") = "1.";
    auto diffusion_kernels = _mfem_problem->addObject<MFEMDiffusionKernel>(
        "MFEMDiffusionKernel", "kernel1", kernel1_params);

    InputParameters kernel2_params = _factory.getValidParams("MFEMDomainLFKernel");
    kernel2_params.set<VariableName>("variable") = var_name;
    kernel2_params.set<MFEMScalarCoefficientName>("coefficient") = "f_exact";
    auto source_kernels = _mfem_problem->addObject<MFEMDomainLFKernel>(
        "MFEMDomainLFKernel", "kernel2", kernel2_params);

    auto & equation_system = *_equation_system;
    equation_system.AddEssentialBC(std::move(essential_bcs[0]));
    equation_system.AddKernel(std::move(diffusion_kernels[0]));
    equation_system.AddKernel(std::move(source_kernels[0]));
    equation_system.Init(_mfem_problem->getProblemData().gridfunctions,
                         _mfem_problem->getProblemData().cmplx_gridfunctions,
                         mfem::AssemblyLevel::LEGACY);

    mfem::Array<int> block_offsets(2);
    block_offsets[0] = 0;
    block_offsets[1] = _fespace->TrueVSize();
    mfem::BlockVector X(block_offsets), B(block_offsets);
    equation_system.FormSystem(X, B);

    solver.SetOperator(equation_system.GetGradient(*_x));

    auto solver_ptr = dynamic_cast<SolverType *>(&solver.GetSolver());
    // Test MFEMKernel returns an integrator of the expected type
    ASSERT_TRUE(solver_ptr != nullptr);
    if (!solve_system)
      return;
    solver_ptr->Mult(B, X);

    mfem::Vector Y(X.Size());
    mfem::OperatorHandle A = equation_system.GetLinearOperator();
    A->Mult(X, Y);
    Y -= B;
    ASSERT_LE(Y.Norml2(), tol);
    ASSERT_GE(B.Norml2(), tol);
  }

private:
  std::unique_ptr<mfem::ParMesh> _pmesh;
  std::unique_ptr<mfem::H1_FECollection> _fec;
  std::unique_ptr<mfem::ParFiniteElementSpace> _fespace;
  std::shared_ptr<mfem::ParGridFunction> _x;
  std::shared_ptr<Moose::MFEM::EquationSystem> _equation_system;
};

/**
 * Test MFEMHypreGMRES creates an mfem::HyperGMRES solver successfully.
 */
TEST_F(MFEMSolverTest, MFEMHypreGMRES)
{
  // Build required kernel inputs
  InputParameters solver_params = _factory.getValidParams("MFEMHypreGMRES");
  solver_params.set<mfem::real_t>("l_tol") = 0.0;
  solver_params.set<mfem::real_t>("l_abs_tol") = 1e-5;

  // Construct kernel
  MFEMHypreGMRES & solver = addObject<MFEMHypreGMRES>("MFEMHypreGMRES", "solver1", solver_params);

  testDiffusionSolve<mfem::HypreGMRES>(solver, 1e-5);
}

/**
 * Test MFEMHypreFGMRES creates an mfem::HyperFGMRES solver successfully.
 */
TEST_F(MFEMSolverTest, MFEMHypreFGMRES)
{
  // Build required kernel inputs
  InputParameters solver_params = _factory.getValidParams("MFEMHypreFGMRES");
  solver_params.set<mfem::real_t>("l_tol") = 1e-7; // HypreFGMRES cannot set absolute tolerance
  // Construct kernel
  MFEMHypreFGMRES & solver =
      addObject<MFEMHypreFGMRES>("MFEMHypreFGMRES", "solver1", solver_params);

  testDiffusionSolve<mfem::HypreFGMRES>(solver, 1e-5);
}

/**
 * Test MFEMHyprePCG creates an mfem::HyprePCG solver successfully.
 */
TEST_F(MFEMSolverTest, MFEMHyprePCG)
{
  // Build required kernel inputs
  InputParameters solver_params = _factory.getValidParams("MFEMHyprePCG");
  solver_params.set<mfem::real_t>("l_tol") = 0.0;
  solver_params.set<mfem::real_t>("l_abs_tol") = 1e-5;

  // Construct kernel
  MFEMHyprePCG & solver = addObject<MFEMHyprePCG>("MFEMHyprePCG", "solver1", solver_params);

  testDiffusionSolve<mfem::HyprePCG>(solver, 1e-5);
}

/**
 * Test MFEMGMRES creates an mfem::GMRESSolver solver successfully.
 */

TEST_F(MFEMSolverTest, MFEMGMRESSolver)
{
  // Build required kernel inputs
  InputParameters solver_params = _factory.getValidParams("MFEMGMRESSolver");
  solver_params.set<mfem::real_t>("l_tol") = 0.0;
  solver_params.set<mfem::real_t>("l_abs_tol") = 1e-5;

  // Construct kernel
  MFEMGMRESSolver & solver =
      addObject<MFEMGMRESSolver>("MFEMGMRESSolver", "solver1", solver_params);

  testDiffusionSolve<mfem::GMRESSolver>(solver, 1e-5);
}

TEST_F(MFEMSolverTest, MFEMCGSolver)
{
  // Build required kernel inputs
  InputParameters solver_params = _factory.getValidParams("MFEMCGSolver");
  solver_params.set<mfem::real_t>("l_tol") = 0.0;
  solver_params.set<mfem::real_t>("l_abs_tol") = 1e-5;

  // Construct kernel
  MFEMCGSolver & solver = addObject<MFEMCGSolver>("MFEMCGSolver", "solver1", solver_params);

  testDiffusionSolve<mfem::CGSolver>(solver, 1e-5);
}

/**
 * Test MFEMHypreBoomerAMG creates an mfem::HypreBoomerAMG solver successfully.
 */
TEST_F(MFEMSolverTest, MFEMHypreBoomerAMG)
{
  // Build required solver inputs
  InputParameters solver_params = _factory.getValidParams("MFEMHypreBoomerAMG");
  solver_params.set<mfem::real_t>("l_tol") = 1e-7; // HypreBoomerAMG cannot set absolute tolerance

  // Construct solver
  MFEMHypreBoomerAMG & solver =
      addObject<MFEMHypreBoomerAMG>("MFEMHypreBoomerAMG", "solver1", solver_params);

  // Test MFEMSolver returns an solver of the expected type
  auto solver_downcast = dynamic_cast<mfem::HypreBoomerAMG *>(&solver.GetSolver());
  // HypreBoomerAMG warnings are tripped by zero rows in matrices; turn this off for this test
  solver_downcast->SetErrorMode(mfem::HypreSolver::ErrorMode::IGNORE_HYPRE_ERRORS);
  ASSERT_NE(solver_downcast, nullptr);
  testDiffusionSolve<mfem::HypreBoomerAMG>(solver, 1e-5);
}

/**
 * Test MFEMHypreADS creates an mfem::HypreADS solver successfully.
 */
TEST_F(MFEMSolverTest, MFEMHypreADS)
{
  // Build required FESpace
  InputParameters fespace_params = _factory.getValidParams("MFEMVectorFESpace");

  fespace_params.set<MooseEnum>("fec_order") = "CONSTANT";
  fespace_params.set<MooseEnum>("fec_type") = "RT";

  // Construct fespace
  addObject<MFEMVectorFESpace>("MFEMVectorFESpace", "HDivFESpace", fespace_params);

  // Build required solver inputs
  InputParameters solver_params = _factory.getValidParams("MFEMHypreADS");
  solver_params.set<MFEMFESpaceName>("fespace") = "HDivFESpace";

  // Construct solver
  MFEMHypreADS & solver = addObject<MFEMHypreADS>("MFEMHypreADS", "solver1", solver_params);

  // Test MFEMSolver returns a solver of the expected type
  auto solver_downcast = dynamic_cast<mfem::HypreADS *>(&solver.GetSolver());
  ASSERT_NE(solver_downcast, nullptr);
}

/**
 * Test MFEMHypreAMS creates an mfem::HypreAMS solver successfully.
 */
TEST_F(MFEMSolverTest, MFEMHypreAMS)
{
  // Build required FESpace
  InputParameters fespace_params = _factory.getValidParams("MFEMVectorFESpace");

  fespace_params.set<MooseEnum>("fec_order") = "FIRST";
  fespace_params.set<MooseEnum>("fec_type") = "ND";

  // Construct fespace
  addObject<MFEMVectorFESpace>("MFEMVectorFESpace", "HCurlFESpace", fespace_params);

  // Build required solver inputs
  InputParameters solver_params = _factory.getValidParams("MFEMHypreAMS");
  solver_params.set<MFEMFESpaceName>("fespace") = "HCurlFESpace";

  // Construct solver
  MFEMHypreAMS & solver = addObject<MFEMHypreAMS>("MFEMHypreAMS", "solver1", solver_params);

  // Test MFEMSolver returns an solver of the expected type
  auto solver_downcast = dynamic_cast<mfem::HypreAMS *>(&solver.GetSolver());
  ASSERT_NE(solver_downcast, nullptr);
}

/**
 * Test MFEMSuperLU creates an Moose::MFEM::SuperLUSolver successfully.
 */
TEST_F(MFEMSolverTest, MFEMSuperLU)
{
  // Build required kernel inputs
  InputParameters solver_params = _factory.getValidParams("MFEMSuperLU");

  // Construct kernel
  MFEMSuperLU & solver = addObject<MFEMSuperLU>("MFEMSuperLU", "solver1", solver_params);

  testDiffusionSolve<mfem::SuperLUSolver>(solver, 1e-12);
}

/**
 * Test MFEMMUMPS creates an mfem::MUMPSSolver successfully.
 */
TEST_F(MFEMSolverTest, MFEMMUMPS)
{
  // Build required solver inputs
  InputParameters solver_params = _factory.getValidParams("MFEMMUMPS");

  // Construct solver
  MFEMMUMPS & solver = addObject<MFEMMUMPS>("MFEMMUMPS", "solver1", solver_params);

  testDiffusionSolve<mfem::MUMPSSolver>(solver, 1e-12);
}

/**
 * Test MFEMHypreGMRES creates an LOR mfem::HyperGMRES solver successfully.
 */
TEST_F(MFEMSolverTest, MFEMHypreGMRESLOR)
{
  // Build required kernel inputs
  InputParameters solver_params = _factory.getValidParams("MFEMHypreGMRES");
  solver_params.set<bool>("low_order_refined") = true;
  solver_params.set<mfem::real_t>("l_tol") = 1e-7;

  // Construct kernel
  MFEMHypreGMRES & solver = addObject<MFEMHypreGMRES>("MFEMHypreGMRES", "solver1", solver_params);

  testDiffusionSolve<mfem::LORSolver<mfem::HypreGMRES>>(solver, 1e-5);
}

/**
 * Test MFEMHypreFGMRES creates an LOR mfem::HyperFGMRES solver successfully.
 */
TEST_F(MFEMSolverTest, MFEMHypreFGMRESLOR)
{
  // Build required kernel inputs
  InputParameters solver_params = _factory.getValidParams("MFEMHypreFGMRES");
  solver_params.set<bool>("low_order_refined") = true;
  solver_params.set<mfem::real_t>("l_tol") = 1e-7;

  // Construct kernel
  MFEMHypreFGMRES & solver =
      addObject<MFEMHypreFGMRES>("MFEMHypreFGMRES", "solver1", solver_params);

  testDiffusionSolve<mfem::LORSolver<mfem::HypreFGMRES>>(solver, 1e-5);
}

/**
 * Test MFEMHyprePCG creates an LOR mfem::HyprePCG solver successfully.
 */
TEST_F(MFEMSolverTest, MFEMHyprePCGLOR)
{
  // Build required kernel inputs
  InputParameters solver_params = _factory.getValidParams("MFEMHyprePCG");
  solver_params.set<bool>("low_order_refined") = true;
  solver_params.set<mfem::real_t>("l_tol") = 1e-7;

  // Construct kernel
  MFEMHyprePCG & solver = addObject<MFEMHyprePCG>("MFEMHyprePCG", "solver1", solver_params);

  testDiffusionSolve<mfem::LORSolver<mfem::HyprePCG>>(solver, 1e-5);
}

/**
 * Test MFEMGMRES creates an LOR mfem::GMRESSolver solver successfully.
 */

TEST_F(MFEMSolverTest, MFEMGMRESSolverLOR)
{
  // Build required kernel inputs
  InputParameters solver_params = _factory.getValidParams("MFEMGMRESSolver");
  solver_params.set<bool>("low_order_refined") = true;
  solver_params.set<mfem::real_t>("l_tol") = 1e-7;

  // Construct kernel
  MFEMGMRESSolver & solver =
      addObject<MFEMGMRESSolver>("MFEMGMRESSolver", "solver1", solver_params);

  testDiffusionSolve<mfem::LORSolver<mfem::GMRESSolver>>(solver, 1e-5);
}

TEST_F(MFEMSolverTest, MFEMCGSolverLOR)
{
  // Build required kernel inputs
  InputParameters solver_params = _factory.getValidParams("MFEMCGSolver");
  solver_params.set<bool>("low_order_refined") = true;
  solver_params.set<mfem::real_t>("l_tol") = 1e-7;

  // Construct kernel
  MFEMCGSolver & solver = addObject<MFEMCGSolver>("MFEMCGSolver", "solver1", solver_params);

  testDiffusionSolve<mfem::LORSolver<mfem::CGSolver>>(solver, 1e-5);
}

TEST_F(MFEMSolverTest, MFEMHypreBoomerAMGLOR)
{
  InputParameters solver_params = _factory.getValidParams("MFEMHypreBoomerAMG");
  solver_params.set<bool>("low_order_refined") = true;

  MFEMHypreBoomerAMG & solver =
      addObject<MFEMHypreBoomerAMG>("MFEMHypreBoomerAMG", "solver1", solver_params);

  testDiffusionSolve<mfem::LORSolver<mfem::HypreBoomerAMG>>(solver, 1e-5, false);
}

#endif
