#ifdef MFEM_ENABLED

#include "MFEMObjectUnitTest.h"
#include "MFEMHypreGMRES.h"
#include "MFEMHypreFGMRES.h"
#include "MFEMHyprePCG.h"
#include "MFEMHypreBoomerAMG.h"
#include "MFEMHypreADS.h"
#include "MFEMHypreAMS.h"
#include "MFEMSuperLU.h"
#include "MFEMGMRESSolver.h"
#include "MFEMCGSolver.h"
#include "MFEMOperatorJacobiSmoother.h"
#include "MFEMVectorFESpace.h"

class MFEMSolverTest : public MFEMObjectUnitTest
{
public:
  MFEMSolverTest() : MFEMObjectUnitTest("MooseUnitApp") {}

  static double uexact(const mfem::Vector & x)
  {
    double u;
    u = x(2) * x(2) * x(2) - 5.0 * x(0) * x(0) * x(1) * x(2);
    return u;
  }

  static void gradexact(const mfem::Vector & x, mfem::Vector & grad)
  {
    grad.SetSize(x.Size());
    grad[0] = -10.0 * x(0) * x(1) * x(2);
    grad[1] = -5.0 * x(0) * x(0) * x(2);
    grad[2] = 3.0 * x(2) * x(2) - 5.0 * x(0) * x(0) * x(1);
  }

  static double d2uexact(const mfem::Vector & x) // returns \Delta u
  {
    double d2u;
    d2u = -10.0 * x(1) * x(2) + 6.0 * x(2);
    return d2u;
  }

  static double fexact(const mfem::Vector & x) // returns -\Delta u
  {
    double d2u = d2uexact(x);
    return -d2u;
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
  void testDiffusionSolve(MFEMSolverBase & solver, mfem::real_t tol)
  {
    mfem::ParMesh pmesh = makeMesh();
    int order = 3;
    int dim = 3;
    mfem::H1_FECollection fec(order, dim);
    mfem::ParFiniteElementSpace fespace(&pmesh, &fec);
    mfem::Array<int> ess_tdof_list, ess_bdr;
    if (pmesh.bdr_attributes.Size())
    {
      ess_bdr.SetSize(pmesh.bdr_attributes.Max());
      ess_bdr = 1;
      fespace.GetEssentialTrueDofs(ess_bdr, ess_tdof_list);
    }
    mfem::FunctionCoefficient f(fexact);
    mfem::ParLinearForm b(&fespace);
    b.AddDomainIntegrator(new mfem::DomainLFIntegrator(f));
    b.Assemble();

    mfem::ParBilinearForm a(&fespace);
    mfem::ConstantCoefficient one(1.0);
    a.AddDomainIntegrator(new mfem::DiffusionIntegrator(one));
    a.Assemble();

    mfem::ParGridFunction x(&fespace);
    mfem::FunctionCoefficient uex(uexact);
    x = 0.0;
    x.ProjectBdrCoefficient(uex, ess_bdr);

    mfem::OperatorPtr A;
    mfem::Vector B, X;
    a.FormLinearSystem(ess_tdof_list, x, b, A, X, B);

    solver.updateSolver(a, ess_tdof_list);
    auto solver_ptr = dynamic_cast<SolverType *>(&solver.getSolver());
    // Test MFEMKernel returns an integrator of the expected type
    ASSERT_TRUE(solver_ptr != nullptr);
    solver_ptr->SetOperator(*A);
    solver_ptr->Mult(B, X);

    mfem::Vector Y(X.Size());
    A->Mult(X, Y);
    Y -= B;
    ASSERT_LE(Y.Norml2(), tol);
  }
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
  auto solver_downcast = dynamic_cast<mfem::HypreBoomerAMG *>(&solver.getSolver());
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
  solver_params.set<UserObjectName>("fespace") = "HDivFESpace";

  // Construct solver
  MFEMHypreADS & solver = addObject<MFEMHypreADS>("MFEMHypreADS", "solver1", solver_params);

  // Test MFEMSolver returns a solver of the expected type
  auto solver_downcast = dynamic_cast<mfem::HypreADS *>(&solver.getSolver());
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
  solver_params.set<UserObjectName>("fespace") = "HCurlFESpace";

  // Construct solver
  MFEMHypreAMS & solver = addObject<MFEMHypreAMS>("MFEMHypreAMS", "solver1", solver_params);

  // Test MFEMSolver returns an solver of the expected type
  auto solver_downcast = dynamic_cast<mfem::HypreAMS *>(&solver.getSolver());
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
  // Build required kernel inputs
  InputParameters solver_params = _factory.getValidParams("MFEMHypreBoomerAMG");
  solver_params.set<bool>("low_order_refined") = true;

  // Construct kernel
  MFEMHypreBoomerAMG & solver =
      addObject<MFEMHypreBoomerAMG>("MFEMHypreBoomerAMG", "solver1", solver_params);

  mfem::ParMesh pmesh = makeMesh();
  mfem::ParFiniteElementSpace fespace(&pmesh, new mfem::H1_FECollection(3, 3));
  mfem::Array<int> ess_tdof_list;
  mfem::ParBilinearForm a(&fespace);
  mfem::ParGridFunction x(&fespace);
  mfem::ParLinearForm b(&fespace);
  a.Assemble();
  b.Assemble();

  mfem::OperatorPtr A;
  mfem::Vector B, X;
  a.FormLinearSystem(ess_tdof_list, x, b, A, X, B);

  solver.updateSolver(a, ess_tdof_list);

  auto solver_ptr = dynamic_cast<mfem::LORSolver<mfem::HypreBoomerAMG> *>(&solver.getSolver());
  // Test MFEMKernel returns an integrator of the expected type
  ASSERT_TRUE(solver_ptr != nullptr);
}

#endif
