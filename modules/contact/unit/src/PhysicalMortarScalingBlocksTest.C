//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "AppFactory.h"
#include "Executioner.h"
#include "FEProblemBase.h"
#include "MooseConfig.h"
#include "MooseMain.h"
#include "MooseMesh.h"
#include "MooseVariableFieldBase.h"
#include "MortarTestUtils.h"
#include "NonlinearSystem.h"

#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/petsc_matrix.h"

// 3-D mortar contact requires enough AD derivative slots for the coupled
// displacement and LM DOFs on each element.
#if MOOSE_AD_MAX_DOFS_PER_ELEM >= 250

namespace
{

// Raw assembled Jacobian entry J[row][col], or 0.0 if that column is structurally absent from
// the row (a valid outcome, not an error, for the mutually-exclusive open/closed NCP branches).
Real
rawEntry(FEProblemBase & fe_problem, Mat mat, dof_id_type row, dof_id_type col)
{
  PetscInt ncols;
  const PetscInt * cols;
  const PetscScalar * vals;
  LibmeshPetscCallA(fe_problem.comm().get(), MatGetRow(mat, row, &ncols, &cols, &vals));
  Real value = 0.0;
  for (PetscInt i = 0; i < ncols; ++i)
    if (static_cast<dof_id_type>(cols[i]) == col)
    {
      value = vals[i];
      break;
    }
  LibmeshPetscCallA(fe_problem.comm().get(), MatRestoreRow(mat, row, &ncols, &cols, &vals));
  return value;
}

// The raw entry scaled by the column dof's PETSc right-diagonal-scale value: the magnitude that
// column actually contributes to the linear system KSPSolve() sees, since
// ComputeWeightedGapLMMechanicalContact::enforceConstraintOnDof sets that per-LM-dof scale to the
// derived physical stiffness via setKSPRightDiagonalScale, and KSPSetRightDiagonalScale applies it
// as a solve-time-only column change of variables rather than mutating the stored matrix.
Real
effectiveEntry(FEProblemBase & fe_problem,
               NonlinearSystemBase & nl,
               Mat mat,
               dof_id_type row,
               dof_id_type col)
{
  return rawEntry(fe_problem, mat, row, col) * nl.getVector("ksp_right_diagonal_scale")(col);
}

// Assert that a/b are within two decades of each other, i.e. the same order of magnitude.
void
expectSameOrder(Real a, Real b, const std::string & label)
{
  ASSERT_NE(a, 0.0) << label << ": expected a nonzero entry";
  ASSERT_NE(b, 0.0) << label << ": expected a nonzero entry";
  const Real ratio = std::abs(a / b);
  EXPECT_GT(ratio, 1e-2) << label << ": ratio = " << ratio;
  EXPECT_LT(ratio, 1e2) << label << ": ratio = " << ratio;
}

struct Blocks
{
  bool found;
  Real Jdd;
  Real Jdl;
  Real Jll;
  Real Jld;
  Real disp_scale;
};

// Each secondary-side contact node's NCP branch (open vs. closed) is selected independently by
// ComputeWeightedGapLMMechanicalContact::enforceConstraintOnDof, so a fixture nominally in one
// state can still have a handful of nodes (e.g. face-edge nodes with partial mortar coverage) in
// the other. Scan the contact face for a node actually in the requested state, then read the four
// displacement/LM Jacobian blocks at that node's disp_z and mortar_normal_lm dofs, all through
// effectiveEntry -- including Jdd and Jld, whose columns are the displacement dof, so that the
// unity right-diagonal-scale assumption for displacement dofs is verified rather than assumed.
Blocks
computeBlocks(FEProblemBase & problem, bool want_closed)
{
  auto & nl = problem.getNonlinearSystem(0);

  problem.computeJacobian(*nl.currentSolution(), *nl.sys().matrix, nl.number());

  auto * mat = dynamic_cast<libMesh::PetscMatrix<Number> *>(nl.sys().matrix);
  mooseAssert(mat, "Expected a PetscMatrix for the system Jacobian");

  const auto & mesh = problem.mesh();
  const auto boundary_id = mesh.getBoundaryID("top_bottom");
  const auto & node_ids = mesh.getNodeList(boundary_id);

  const unsigned int sys_num = nl.number();
  const unsigned int lm_var_num = problem.getVariable(0, "mortar_normal_lm").number();
  const unsigned int disp_var_num = problem.getVariable(0, "disp_z").number();

  Blocks blocks;
  blocks.found = false;
  for (const auto id : node_ids)
  {
    const Node & node = mesh.nodeRef(id);
    const dof_id_type lm_dof = node.dof_number(sys_num, lm_var_num, 0);
    const dof_id_type disp_dof = node.dof_number(sys_num, disp_var_num, 0);
    const bool is_closed = rawEntry(problem, mat->mat(), lm_dof, disp_dof) != 0.0;
    if (is_closed != want_closed)
      continue;

    blocks.found = true;
    blocks.Jdd = effectiveEntry(problem, nl, mat->mat(), disp_dof, disp_dof);
    blocks.Jdl = effectiveEntry(problem, nl, mat->mat(), disp_dof, lm_dof);
    blocks.Jll = effectiveEntry(problem, nl, mat->mat(), lm_dof, lm_dof);
    blocks.Jld = effectiveEntry(problem, nl, mat->mat(), lm_dof, disp_dof);
    blocks.disp_scale = nl.getVector("ksp_right_diagonal_scale")(disp_dof);
    break;
  }
  return blocks;
}

} // namespace

// Verify that in an all-open contact state, the LM equation's own Jacobian diagonal (Jll) and the
// reverse displacement-vs-LM coupling (Jdl, from NormalMortarMechanicalContact) both land at the
// same order of magnitude as the displacement (elasticity) block (Jdd) once the PETSc
// right-diagonal scale is applied -- and that displacement dofs carry unity right-diagonal scale,
// so effectiveEntry leaves Jdd itself unaffected.
TEST(PhysicalMortarScalingBlocks, AllOpenBlocksSameOrder)
{
  std::vector<std::string> str_args = {
      "contact-unit", "-i", inputPath(__FILE__, "frictionless_physical_open.i")};
  std::vector<char *> argv_vec;
  for (auto & s : str_args)
    argv_vec.push_back(s.data());
  const int argc = static_cast<int>(argv_vec.size());

  auto app = Moose::createMooseApp("ContactApp", argc, argv_vec.data());
  app->run();
  auto & problem = app->getExecutioner()->feProblem();

  EXPECT_DOUBLE_EQ(problem.getPostprocessorValueByName("contact"), 0)
      << "expected no active contact DOFs in the all-open state";

  const auto blocks = computeBlocks(problem, /*want_closed=*/false);

  ASSERT_TRUE(blocks.found) << "no boundary node found in the open NCP branch";
  EXPECT_DOUBLE_EQ(blocks.disp_scale, 1.0)
      << "displacement dofs should carry unity right-diagonal scale";
  EXPECT_EQ(blocks.Jld, 0.0) << "Jld should be structurally zero in the open state";
  EXPECT_NE(blocks.Jll, 0.0) << "Jll should be populated in the open state";

  expectSameOrder(blocks.Jdd, blocks.Jdl, "Jdd vs Jdl");
  expectSameOrder(blocks.Jdd, blocks.Jll, "Jdd vs Jll");
  expectSameOrder(blocks.Jdl, blocks.Jll, "Jdl vs Jll");
}

// Verify that in an all-closed contact state, the LM equation's own Jacobian off-diagonal (Jld)
// and the reverse displacement-vs-LM coupling (Jdl) both land at the same order of magnitude as
// the displacement block (Jdd), and that displacement dofs carry unity right-diagonal scale.
TEST(PhysicalMortarScalingBlocks, AllClosedBlocksSameOrder)
{
  std::vector<std::string> str_args = {
      "contact-unit", "-i", inputPath(__FILE__, "frictionless_physical_closed.i")};
  std::vector<char *> argv_vec;
  for (auto & s : str_args)
    argv_vec.push_back(s.data());
  const int argc = static_cast<int>(argv_vec.size());

  auto app = Moose::createMooseApp("ContactApp", argc, argv_vec.data());
  app->run();
  auto & problem = app->getExecutioner()->feProblem();

  EXPECT_GT(problem.getPostprocessorValueByName("contact"), 0)
      << "expected active contact DOFs in the all-closed state";

  const auto blocks = computeBlocks(problem, /*want_closed=*/true);

  ASSERT_TRUE(blocks.found) << "no boundary node found in the closed NCP branch";
  EXPECT_DOUBLE_EQ(blocks.disp_scale, 1.0)
      << "displacement dofs should carry unity right-diagonal scale";
  EXPECT_EQ(blocks.Jll, 0.0) << "Jll should be structurally zero in the closed state";
  EXPECT_NE(blocks.Jld, 0.0) << "Jld should be populated in the closed state";

  expectSameOrder(blocks.Jdd, blocks.Jdl, "Jdd vs Jdl");
  expectSameOrder(blocks.Jdd, blocks.Jld, "Jdd vs Jld");
  expectSameOrder(blocks.Jdl, blocks.Jld, "Jdl vs Jld");
}

#endif // MOOSE_AD_MAX_DOFS_PER_ELEM >= 250
