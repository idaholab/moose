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

#include <array>
#include <cmath>

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

// Assert that a/b are within one decade of each other, i.e. the same order of magnitude.
void
expectSameOrder(Real a, Real b, const std::string & label)
{
  ASSERT_NE(a, 0.0) << label << ": expected a nonzero entry";
  ASSERT_NE(b, 0.0) << label << ": expected a nonzero entry";
  const Real ratio = std::abs(a / b);
  EXPECT_GT(ratio, 1e-1) << label << ": ratio = " << ratio;
  EXPECT_LT(ratio, 1e1) << label << ": ratio = " << ratio;
}

struct Blocks
{
  bool found;
  Real Jdd;
  Real Jdl;
  Real Jll;
  Real Jld;
};

// A mortar LM's coupling into the displacement (elasticity) equations is a residual term of the
// form (scalar) * (component of a mesh-dependent unit normal/tangent vector), so no single global
// displacement component is guaranteed to capture that coupling's full magnitude -- the unit
// vector's orientation is set by mesh geometry, not by the global axes. Reading all three
// displacement columns/rows for a fixed LM dof and taking their Euclidean norm recovers
// (scalar) * |unit vector| = (scalar) regardless of that orientation.
Real
dispCouplingNorm(FEProblemBase & fe_problem,
                 Mat mat,
                 const std::array<dof_id_type, 3> & disp_dofs,
                 dof_id_type other_dof,
                 bool disp_is_row)
{
  Real sum_sq = 0.0;
  for (const auto & disp_dof : disp_dofs)
  {
    const Real value = disp_is_row ? rawEntry(fe_problem, mat, disp_dof, other_dof)
                                   : rawEntry(fe_problem, mat, other_dof, disp_dof);
    sum_sq += value * value;
  }
  return std::sqrt(sum_sq);
}

// The elasticity (displacement/displacement) diagonal block isn't tied to any mortar-specific
// direction, but norming it the same way as dispCouplingNorm keeps the comparison self-consistent
// without hardcoding a single displacement component here either.
Real
dispDiagonalNorm(FEProblemBase & fe_problem, Mat mat, const std::array<dof_id_type, 3> & disp_dofs)
{
  Real sum_sq = 0.0;
  for (const auto & disp_dof : disp_dofs)
  {
    const Real value = rawEntry(fe_problem, mat, disp_dof, disp_dof);
    sum_sq += value * value;
  }
  return std::sqrt(sum_sq);
}

// Each secondary-side contact node's NCP branch (open vs. closed) is selected independently by
// ComputeWeightedGapLMMechanicalContact::enforceConstraintOnDof, so a fixture nominally in one
// state can still have a handful of nodes (e.g. face-edge nodes with partial mortar coverage) in
// the other. Scan the contact face for a node actually in the requested state, then read the
// displacement/LM Jacobian blocks at that node's disp_x/y/z and mortar_normal_lm dofs -- including
// Jdd and Jld, whose columns are a displacement dof, so the elasticity block itself is read the
// same way as the mortar-coupling blocks it is compared against.
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
  const unsigned int disp_x_var_num = problem.getVariable(0, "disp_x").number();
  const unsigned int disp_y_var_num = problem.getVariable(0, "disp_y").number();
  const unsigned int disp_z_var_num = problem.getVariable(0, "disp_z").number();

  Blocks blocks;
  blocks.found = false;
  for (const auto id : node_ids)
  {
    const Node & node = mesh.nodeRef(id);
    const dof_id_type lm_dof = node.dof_number(sys_num, lm_var_num, 0);
    const std::array<dof_id_type, 3> disp_dofs{{node.dof_number(sys_num, disp_x_var_num, 0),
                                                node.dof_number(sys_num, disp_y_var_num, 0),
                                                node.dof_number(sys_num, disp_z_var_num, 0)}};
    const bool is_closed = rawEntry(problem, mat->mat(), lm_dof, disp_dofs[2]) != 0.0;
    if (is_closed != want_closed)
      continue;

    blocks.found = true;
    blocks.Jdd = dispDiagonalNorm(problem, mat->mat(), disp_dofs);
    blocks.Jdl = dispCouplingNorm(problem, mat->mat(), disp_dofs, lm_dof, /*disp_is_row=*/true);
    blocks.Jll = rawEntry(problem, mat->mat(), lm_dof, lm_dof);
    blocks.Jld = dispCouplingNorm(problem, mat->mat(), disp_dofs, lm_dof, /*disp_is_row=*/false);
    break;
  }
  return blocks;
}

} // namespace

// Run an app from the given input file. The caller must keep the returned MooseApp alive for as
// long as it uses the FEProblemBase obtained from it.
std::unique_ptr<MooseApp>
runApp(const std::string & input_file)
{
  std::vector<std::string> str_args = {"contact-unit", "-i", inputPath(__FILE__, input_file)};
  std::vector<char *> argv_vec;
  for (auto & s : str_args)
    argv_vec.push_back(s.data());
  const int argc = static_cast<int>(argv_vec.size());

  auto app = Moose::createMooseApp("ContactApp", argc, argv_vec.data());
  app->run();
  return app;
}

// Verify that the LM equation's own Jacobian diagonal (Jll in the open state, Jld in the closed
// state) and the reverse displacement-vs-LM coupling (Jdl, from NormalMortarMechanicalContact)
// both land at the same order of magnitude as the displacement (elasticity) block (Jdd). The
// physical LM value is baked into the contact residuals via the derived stiffness scale, so this
// checks the assembled Jacobian directly rather than a solve-time column scale. Run against both
// a fixture with no scaling anywhere and one with scaling applied only to the displacement
// variables, to verify the compensation mechanism is robust to the displacement-scaling choice
// rather than correct for one hardcoded value.
void
expectFrictionlessBlocksSameOrder(const std::string & input_file, bool want_closed)
{
  auto app = runApp(input_file);
  auto & problem = app->getExecutioner()->feProblem();

  if (want_closed)
    EXPECT_GT(problem.getPostprocessorValueByName("contact"), 0)
        << "expected active contact DOFs in the all-closed state";
  else
    EXPECT_DOUBLE_EQ(problem.getPostprocessorValueByName("contact"), 0)
        << "expected no active contact DOFs in the all-open state";

  const auto blocks = computeBlocks(problem, want_closed);

  ASSERT_TRUE(blocks.found) << "no boundary node found in the " << (want_closed ? "closed" : "open")
                            << " NCP branch";

  const Real off_diag = want_closed ? blocks.Jld : blocks.Jll;
  const std::string off_diag_label = want_closed ? "Jld" : "Jll";

  if (want_closed)
  {
    EXPECT_EQ(blocks.Jll, 0.0) << "Jll should be structurally zero in the closed state";
    EXPECT_NE(blocks.Jld, 0.0) << "Jld should be populated in the closed state";
  }
  else
  {
    EXPECT_EQ(blocks.Jld, 0.0) << "Jld should be structurally zero in the open state";
    EXPECT_NE(blocks.Jll, 0.0) << "Jll should be populated in the open state";
  }

  expectSameOrder(blocks.Jdd, blocks.Jdl, "Jdd vs Jdl");
  expectSameOrder(blocks.Jdd, off_diag, "Jdd vs " + off_diag_label);
  expectSameOrder(blocks.Jdl, off_diag, "Jdl vs " + off_diag_label);
}

TEST(PhysicalMortarScalingBlocks, AllOpenBlocksSameOrder)
{
  expectFrictionlessBlocksSameOrder("frictionless_physical_open.i", /*want_closed=*/false);
}

TEST(PhysicalMortarScalingBlocks, AllOpenBlocksSameOrderDisplacementScaled)
{
  expectFrictionlessBlocksSameOrder("frictionless_physical_open_scaled.i", /*want_closed=*/false);
}

TEST(PhysicalMortarScalingBlocks, AllClosedBlocksSameOrder)
{
  expectFrictionlessBlocksSameOrder("frictionless_physical_closed.i", /*want_closed=*/true);
}

TEST(PhysicalMortarScalingBlocks, AllClosedBlocksSameOrderDisplacementScaled)
{
  expectFrictionlessBlocksSameOrder("frictionless_physical_closed_scaled.i", /*want_closed=*/true);
}

namespace
{

struct FrictionBlocks
{
  bool found;
  Real Jtt;
  Real Jtd;
  Real Jdt;
  Real Jdd;
};

// The three mutually-exclusive branches of the epsilon-gated frictional contact residual (see
// Moose::Mortar::Contact::frictionalContactResidual): Open, where the raw normal pressure is below
// epsilon and the residual falls back to the trivial identity tangential_pressure; and, among
// NCP-closed nodes, Stick (Jtt == 0: the tangential force is below the friction limit and held by
// direct enforcement of zero relative tangential displacement) or Slip (Jtt != 0: the tangential
// force is pinned to the friction cone, c_t * normal_lm).
enum class FrictionBranch
{
  Open,
  Stick,
  Slip
};

// Scan for a node in the requested friction branch and read the friction-row/displacement-column
// Jacobian blocks at that node's mortar_tangential_lm and disp_x/y/z dofs, mirroring
// computeBlocks's normal-LM scan. Jtd and Jdt are norms across all three displacement components
// for the same reason computeBlocks norms Jdl/Jld: the friction residual couples to displacement
// through the mesh's tangent direction, which need not align with any single global axis.
FrictionBlocks
computeFrictionBlocks(FEProblemBase & problem, FrictionBranch branch)
{
  auto & nl = problem.getNonlinearSystem(0);

  problem.computeJacobian(*nl.currentSolution(), *nl.sys().matrix, nl.number());

  auto * mat = dynamic_cast<libMesh::PetscMatrix<Number> *>(nl.sys().matrix);
  mooseAssert(mat, "Expected a PetscMatrix for the system Jacobian");

  const auto & mesh = problem.mesh();
  const auto boundary_id = mesh.getBoundaryID("top_bottom");
  const auto & node_ids = mesh.getNodeList(boundary_id);

  const unsigned int sys_num = nl.number();
  const unsigned int normal_lm_var_num = problem.getVariable(0, "mortar_normal_lm").number();
  const unsigned int t_var_num = problem.getVariable(0, "mortar_tangential_lm").number();
  const unsigned int disp_x_var_num = problem.getVariable(0, "disp_x").number();
  const unsigned int disp_y_var_num = problem.getVariable(0, "disp_y").number();
  const unsigned int disp_z_var_num = problem.getVariable(0, "disp_z").number();

  FrictionBlocks blocks;
  blocks.found = false;
  for (const auto id : node_ids)
  {
    const Node & node = mesh.nodeRef(id);
    const dof_id_type normal_lm_dof = node.dof_number(sys_num, normal_lm_var_num, 0);
    const dof_id_type t_dof = node.dof_number(sys_num, t_var_num, 0);
    const std::array<dof_id_type, 3> disp_dofs{{node.dof_number(sys_num, disp_x_var_num, 0),
                                                node.dof_number(sys_num, disp_y_var_num, 0),
                                                node.dof_number(sys_num, disp_z_var_num, 0)}};

    const bool is_closed = rawEntry(problem, mat->mat(), normal_lm_dof, disp_dofs[2]) != 0.0;
    if (branch == FrictionBranch::Open)
    {
      if (is_closed)
        continue;
    }
    else
    {
      if (!is_closed)
        continue;
      const bool is_slip = rawEntry(problem, mat->mat(), t_dof, t_dof) != 0.0;
      if (is_slip != (branch == FrictionBranch::Slip))
        continue;
    }

    if (blocks.found)
      continue;

    blocks.found = true;
    blocks.Jtt = rawEntry(problem, mat->mat(), t_dof, t_dof);
    blocks.Jtd = dispCouplingNorm(problem, mat->mat(), disp_dofs, t_dof, /*disp_is_row=*/false);
    blocks.Jdt = dispCouplingNorm(problem, mat->mat(), disp_dofs, t_dof, /*disp_is_row=*/true);
    blocks.Jdd = dispDiagonalNorm(problem, mat->mat(), disp_dofs);
  }
  return blocks;
}

} // namespace

// Verify that for both stick and slip contact nodes, the friction-row/displacement-column coupling
// (Jtd, Jdt) lands at the same order of magnitude as the displacement (elasticity) block (Jdd), and
// that for a slipping node the friction LM's own Jacobian diagonal (Jtt) does too. A sticking
// contact node's Jtt is structurally zero, since the friction residual's tangential_pressure terms
// cancel exactly in the stick branch, but Jtd and Jdt remain populated there too -- Jtd through the
// residual's dependence on tangential velocity, Jdt through the reaction-force term in
// TangentialMortarMechanicalContact's residual, which reads the friction LM unconditionally in both
// branches -- and the row/column compensation is applied unconditionally on the tangential
// strategy rather than gated on stick vs. slip, so the same same-order expectation applies to them
// in the stick branch. For an open (non-contact) node, the epsilon gate falls back to the trivial
// identity residual, tangential_pressure, so Jtt is not zero there either (raw Jtt = 1) -- the same
// row/column compensation applies to that identity term as to the stick/slip terms, mirroring how
// the normal LM's own diagonal (Jll) lands at Jdd's order in the frictionless open-state test -- so
// Jtt is asserted same-order there too. This targets the degree-one (Alart-Curnier) friction
// residual only -- see FrictionProjectionTest.C::Homogeneity for why: Hueber-Stadler-Wohlmuth (the
// default) is homogeneous of degree two jointly in pressure and the friction-cone radius, so its
// diagonal block scales with the solution-dependent contact pressure and cannot be brought to Jdd's
// order by a fixed compensation constant the way the degree-one residual's diagonal can. Jtl (the
// friction-row/normal-LM-column cross-coupling) is intentionally not asserted to be same-order
// here, unlike Jtt/Jtd/Jdt.
void
expectFrictionalBlocksSameOrder(const std::string & input_file)
{
  auto app = runApp(input_file);
  auto & problem = app->getExecutioner()->feProblem();

  const auto open = computeFrictionBlocks(problem, FrictionBranch::Open);
  ASSERT_TRUE(open.found) << "no boundary node found outside contact (open normal LM branch)";
  EXPECT_NE(open.Jtt, 0.0) << "Jtt should be populated in the open branch";
  expectSameOrder(open.Jdd, open.Jtt, "Jdd vs Jtt (open)");

  const auto stick = computeFrictionBlocks(problem, FrictionBranch::Stick);
  ASSERT_TRUE(stick.found) << "no boundary node found in the stick friction branch";
  EXPECT_EQ(stick.Jtt, 0.0) << "Jtt should be structurally zero in the stick branch";
  expectSameOrder(stick.Jdd, stick.Jtd, "Jdd vs Jtd (stick)");
  expectSameOrder(stick.Jdd, stick.Jdt, "Jdd vs Jdt (stick)");

  const auto slip = computeFrictionBlocks(problem, FrictionBranch::Slip);
  ASSERT_TRUE(slip.found) << "no boundary node found in the slip friction branch";
  EXPECT_NE(slip.Jtt, 0.0) << "Jtt should be populated in the slip branch";

  expectSameOrder(slip.Jdd, slip.Jtd, "Jdd vs Jtd");
  expectSameOrder(slip.Jdd, slip.Jtt, "Jdd vs Jtt");
  expectSameOrder(slip.Jdd, slip.Jdt, "Jdd vs Jdt");
}

TEST(PhysicalMortarScalingBlocks, FrictionalBlocksSameOrder)
{
  expectFrictionalBlocksSameOrder("frictional_physical_ac.i");
}

TEST(PhysicalMortarScalingBlocks, FrictionalBlocksSameOrderUnscaled)
{
  expectFrictionalBlocksSameOrder("frictional_physical_ac_unscaled.i");
}

#endif // MOOSE_AD_MAX_DOFS_PER_ELEM >= 250
