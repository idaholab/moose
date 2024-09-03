#include "MFEMObjectUnitTest.h"
#include "MFEMHypreGMRES.h"
#include "MFEMHypreFGMRES.h"
#include "MFEMHyprePCG.h"
#include "MFEMHypreBoomerAMG.h"
#include "MFEMHypreAMS.h"
#include "MFEMSuperLU.h"

class MFEMSolverTest : public MFEMObjectUnitTest
{
public:
  MFEMSolverTest() : MFEMObjectUnitTest("PlatypusApp") {}

  static double uexact(const mfem::Vector & x)
  {
    double u;
    switch (x.Size())
    {
      case 1:
        u = 3.0 + 2.0 * x(0) - 0.5 * x(0) * x(0);
        break;
      case 2:
        u = 1.0 + 0.2 * x(0) - 0.9 * x(0) * x(1) + x(1) * x(1) * x(0);
        break;
      default:
        u = x(2) * x(2) * x(2) - 5.0 * x(0) * x(0) * x(1) * x(2);
        break;
    }
    return u;
  }

  static void gradexact(const mfem::Vector & x, mfem::Vector & grad)
  {
    grad.SetSize(x.Size());
    switch (x.Size())
    {
      case 1:
        grad[0] = 2.0 - x(0);
        break;
      case 2:
        grad[0] = 0.2 - 0.9 * x(1) + x(1) * x(1);
        grad[1] = -0.9 * x(0) + 2.0 * x(0) * x(1);
        break;
      default:
        grad[0] = -10.0 * x(0) * x(1) * x(2);
        grad[1] = -5.0 * x(0) * x(0) * x(2);
        grad[2] = 3.0 * x(2) * x(2) - 5.0 * x(0) * x(0) * x(1);
        break;
    }
  }

  static double d2uexact(const mfem::Vector & x) // returns \Delta u
  {
    double d2u;
    switch (x.Size())
    {
      case 1:
        d2u = -1.0;
        break;
      case 2:
        d2u = 2.0 * x(0);
        break;
      default:
        d2u = -10.0 * x(1) * x(2) + 6.0 * x(2);
        break;
    }
    return d2u;
  }

  static double fexact(const mfem::Vector & x) // returns -\Delta u
  {
    double d2u = d2uexact(x);
    return -d2u;
  }

  /**
   * Test a solver can solve a dummy diffusion problem to the expected tolerance.
   * Based on mfem/tests/unit/linalg/test_direct_solvers.cpp.
   */
  void testDiffusionSolve(mfem::Solver & solver, mfem::real_t tol)
  {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    const int ne = 4;
    int dim = 3;
    mfem::Mesh mesh;
    mesh = mfem::Mesh::MakeCartesian3D(ne, ne, ne, mfem::Element::HEXAHEDRON, 1.0, 1.0, 1.0);
    mfem::ParMesh pmesh(MPI_COMM_WORLD, mesh);
    mesh.Clear();
    int order = 3;
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

    solver.SetOperator(*A);
    solver.Mult(B, X);

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
  solver_params.set<double>("l_tol") = 0.0;
  solver_params.set<double>("l_abs_tol") = 1e-5;

  // Construct kernel
  MFEMHypreGMRES & solver = addObject<MFEMHypreGMRES>("MFEMHypreGMRES", "solver1", solver_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto solver_downcast = std::dynamic_pointer_cast<mfem::HypreGMRES>(solver.getSolver());
  ASSERT_NE(solver_downcast.get(), nullptr);
  testDiffusionSolve(*solver_downcast.get(), 1e-5);
}

/**
 * Test MFEMHypreFGMRES creates an mfem::HyperFGMRES solver successfully.
 */
TEST_F(MFEMSolverTest, MFEMHypreFGMRES)
{
  // Build required kernel inputs
  InputParameters solver_params = _factory.getValidParams("MFEMHypreFGMRES");
  solver_params.set<double>("l_tol") = 1e-7; // HypreFGMRES cannot set absolute tolerance
  // Construct kernel
  MFEMHypreFGMRES & solver =
      addObject<MFEMHypreFGMRES>("MFEMHypreFGMRES", "solver1", solver_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto solver_downcast = std::dynamic_pointer_cast<mfem::HypreFGMRES>(solver.getSolver());
  ASSERT_NE(solver_downcast.get(), nullptr);
  testDiffusionSolve(*solver_downcast.get(), 1e-5);
}

/**
 * Test MFEMHyprePCG creates an mfem::HyperPCG solver successfully.
 */
TEST_F(MFEMSolverTest, MFEMHyprePCG)
{
  // Build required kernel inputs
  InputParameters solver_params = _factory.getValidParams("MFEMHyprePCG");
  solver_params.set<double>("l_tol") = 0.0;
  solver_params.set<double>("l_abs_tol") = 1e-5;

  // Construct kernel
  MFEMHyprePCG & solver = addObject<MFEMHyprePCG>("MFEMHyprePCG", "solver1", solver_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto solver_downcast = std::dynamic_pointer_cast<mfem::HyprePCG>(solver.getSolver());
  ASSERT_NE(solver_downcast.get(), nullptr);
  testDiffusionSolve(*solver_downcast.get(), 1e-5);
}

/**
 * Test MFEMHypreBoomerAMG creates an mfem::HypreBoomerAMG solver successfully.
 */
TEST_F(MFEMSolverTest, MFEMHypreBoomerAMG)
{
  // Build required solver inputs
  InputParameters solver_params = _factory.getValidParams("MFEMHypreBoomerAMG");
  solver_params.set<double>("l_tol") = 1e-7; // HypreBoomerAMG cannot set absolute tolerance

  // Construct solver
  MFEMHypreBoomerAMG & solver =
      addObject<MFEMHypreBoomerAMG>("MFEMHypreBoomerAMG", "solver1", solver_params);

  // Test MFEMSolver returns an solver of the expected type
  auto solver_downcast = std::dynamic_pointer_cast<mfem::HypreBoomerAMG>(solver.getSolver());
  // HypreBoomerAMG warnings are tripped by zero rows in matrices; turn this off for this test
  solver_downcast->SetErrorMode(mfem::HypreSolver::ErrorMode::IGNORE_HYPRE_ERRORS);
  ASSERT_NE(solver_downcast.get(), nullptr);
  testDiffusionSolve(*solver_downcast.get(), 1e-5);
}

/**
 * Test MFEMHypreAMS creates an mfem::HypreAMS solver successfully.
 */
TEST_F(MFEMSolverTest, MFEMHypreAMS)
{
  // Build required FESpace
  InputParameters fespace_params = _factory.getValidParams("MFEMFESpace");

  fespace_params.set<MooseEnum>("fec_order") = "FIRST";
  fespace_params.set<MooseEnum>("fec_type") = "ND";

  // Construct fespace
  MFEMFESpace & fespace = addObject<MFEMFESpace>("MFEMFESpace", "HCurlFESpace", fespace_params);

  // Build required solver inputs
  InputParameters solver_params = _factory.getValidParams("MFEMHypreAMS");
  solver_params.set<UserObjectName>("fespace") = "HCurlFESpace";

  // Construct solver
  MFEMHypreAMS & solver = addObject<MFEMHypreAMS>("MFEMHypreAMS", "solver1", solver_params);

  // Test MFEMSolver returns an solver of the expected type
  auto solver_downcast = std::dynamic_pointer_cast<mfem::HypreAMS>(solver.getSolver());
  ASSERT_NE(solver_downcast.get(), nullptr);
}

/**
 * Test MFEMSuperLU creates an platypus::SuperLUSolver successfully.
 */
TEST_F(MFEMSolverTest, MFEMSuperLU)
{
  // Build required kernel inputs
  InputParameters solver_params = _factory.getValidParams("MFEMSuperLU");

  // Construct kernel
  MFEMSuperLU & solver = addObject<MFEMSuperLU>("MFEMSuperLU", "solver1", solver_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto solver_downcast = std::dynamic_pointer_cast<platypus::SuperLUSolver>(solver.getSolver());
  ASSERT_NE(solver_downcast.get(), nullptr);
  testDiffusionSolve(*solver_downcast.get(), 1e-12);
}
