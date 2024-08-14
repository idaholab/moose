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
};

/**
 * Test MFEMHypreGMRES creates an mfem::HyperGMRES successfully.
 */
TEST_F(MFEMSolverTest, MFEMHypreGMRES)
{
  // Build required kernel inputs
  InputParameters solver_params = _factory.getValidParams("MFEMHypreGMRES");

  // Construct kernel
  MFEMHypreGMRES & solver = addObject<MFEMHypreGMRES>("MFEMHypreGMRES", "solver1", solver_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto solver_downcast = std::dynamic_pointer_cast<mfem::HypreGMRES>(solver.getSolver());
  ASSERT_NE(solver_downcast.get(), nullptr);
}

/**
 * Test MFEMHypreFGMRES creates an mfem::HyperFGMRES successfully.
 */
TEST_F(MFEMSolverTest, MFEMHypreFGMRES)
{
  // Build required kernel inputs
  InputParameters solver_params = _factory.getValidParams("MFEMHypreFGMRES");

  // Construct kernel
  MFEMHypreFGMRES & solver =
      addObject<MFEMHypreFGMRES>("MFEMHypreFGMRES", "solver1", solver_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto solver_downcast = std::dynamic_pointer_cast<mfem::HypreFGMRES>(solver.getSolver());
  ASSERT_NE(solver_downcast.get(), nullptr);
}

/**
 * Test MFEMHyprePCG creates an mfem::HyperPCG successfully.
 */
TEST_F(MFEMSolverTest, MFEMHyprePCG)
{
  // Build required kernel inputs
  InputParameters solver_params = _factory.getValidParams("MFEMHyprePCG");

  // Construct kernel
  MFEMHyprePCG & solver = addObject<MFEMHyprePCG>("MFEMHyprePCG", "solver1", solver_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto solver_downcast = std::dynamic_pointer_cast<mfem::HyprePCG>(solver.getSolver());
  ASSERT_NE(solver_downcast.get(), nullptr);
}

/**
 * Test MFEMHypreBoomerAMG creates an mfem::HypreBoomerAMG solver successfully.
 */
TEST_F(MFEMSolverTest, MFEMHypreBoomerAMG)
{
  // Build required solver inputs
  InputParameters solver_params = _factory.getValidParams("MFEMHypreBoomerAMG");

  // Construct solver
  MFEMHypreBoomerAMG & solver =
      addObject<MFEMHypreBoomerAMG>("MFEMHypreBoomerAMG", "solver1", solver_params);

  // Test MFEMSolver returns an solver of the expected type
  auto solver_downcast = std::dynamic_pointer_cast<mfem::HypreBoomerAMG>(solver.getSolver());
  ASSERT_NE(solver_downcast.get(), nullptr);
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
}
