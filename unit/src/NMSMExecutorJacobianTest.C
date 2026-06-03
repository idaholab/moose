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
#include "MooseApp.h"
#include "FEProblemBase.h"
#include "NewtonSNESExecutor.h"
#include "SNESNPCExecutor.h"
#include "NonlinearSystem.h"

#include "libmesh/petsc_vector.h"

#include <petscsnes.h>
#include <petscvec.h>
#include <petscmat.h>

using namespace libMesh;

class NMSMExecutorJacobianTest : public testing::TestWithParam<std::string>
{
};

TEST_P(NMSMExecutorJacobianTest, JacobianMatchesFD)
{
  const std::string sweep_type = GetParam();
  const PetscReal eps = 1e-6;
  const PetscReal tol = 1e-5;

  auto app = AppFactory::create("MooseUnitApp",
                                {"--executor",
                                 "-i",
                                 "files/NMSMExecutorJacobianTest/off_diag_coupling_npc_jacobian.i",
                                 "Executors/shell_pc/sweep_type=" + sweep_type});
  app->setupOptions();
  app->runInputFile();

  auto & fe_problem = app->feProblem();
  fe_problem.initialSetup();
  fe_problem.insertPetscOptionsIfNeeded();

  auto & outer = app->getExecutor<NewtonSNESExecutor>("outer");
  auto & shell_pc = app->getExecutor<SNESNPCExecutor>("shell_pc");

  // Trigger SNES setup for both executors and attach the NPC
  SNES outer_snes = outer.getSNES();
  SNES npc_snes = shell_pc.getSNES();
  PetscCallAbort(PETSC_COMM_WORLD, SNESSetNPC(outer_snes, npc_snes));

  // Build a VecNest aliasing the libmesh solution vecs and set a deterministic base state
  const PetscInt n_sys = 2;
  std::vector<Vec> lm_sol(n_sys);
  for (PetscInt i = 0; i < n_sys; ++i)
  {
    auto & nl_sys = fe_problem.getNonlinearSystem(i);
    lm_sol[i] = cast_ptr<PetscVector<Number> *>(nl_sys.system().solution.get())->vec();
    PetscCallAbort(PETSC_COMM_WORLD, VecSet(lm_sol[i], 0.5));
    nl_sys.system().update();
  }

  Vec x;
  PetscCallAbort(PETSC_COMM_WORLD,
                 VecCreateNest(PETSC_COMM_WORLD, n_sys, nullptr, lm_sol.data(), &x));

  // Assemble the Jacobian at the base state
  Mat J, P_mat;
  PetscCallAbort(PETSC_COMM_WORLD, SNESGetJacobian(outer_snes, &J, &P_mat, nullptr, nullptr));
  PetscCallAbort(PETSC_COMM_WORLD, SNESComputeJacobian(outer_snes, x, J, P_mat));

  // Working vecs for the FD test
  Vec d, x_plus, x_minus, r_plus, r_minus, fd, jac_action;
  PetscCallAbort(PETSC_COMM_WORLD, VecDuplicate(x, &d));
  PetscCallAbort(PETSC_COMM_WORLD, VecDuplicate(x, &x_plus));
  PetscCallAbort(PETSC_COMM_WORLD, VecDuplicate(x, &x_minus));
  PetscCallAbort(PETSC_COMM_WORLD, VecDuplicate(x, &r_plus));
  PetscCallAbort(PETSC_COMM_WORLD, VecDuplicate(x, &r_minus));
  PetscCallAbort(PETSC_COMM_WORLD, VecDuplicate(x, &fd));
  PetscCallAbort(PETSC_COMM_WORLD, VecDuplicate(x, &jac_action));

  PetscInt n_nest;
  PetscCallAbort(PETSC_COMM_WORLD, VecNestGetSize(x, &n_nest));

  for (PetscInt block = 0; block < n_nest; ++block)
  {
    Vec sub_d;
    PetscCallAbort(PETSC_COMM_WORLD, VecNestGetSubVec(d, block, &sub_d));

    Vec sub_x;
    PetscCallAbort(PETSC_COMM_WORLD, VecNestGetSubVec(x, block, &sub_x));

    PetscInt n_block;
    PetscCallAbort(PETSC_COMM_WORLD, VecGetSize(sub_x, &n_block));

    for (PetscInt dof = 0; dof < n_block; ++dof)
    {
      // Build unit basis direction in block `block`, entry `dof`
      PetscCallAbort(PETSC_COMM_WORLD, VecSet(d, 0.0));
      PetscCallAbort(PETSC_COMM_WORLD, VecSetValue(sub_d, dof, 1.0, INSERT_VALUES));
      PetscCallAbort(PETSC_COMM_WORLD, VecAssemblyBegin(sub_d));
      PetscCallAbort(PETSC_COMM_WORLD, VecAssemblyEnd(sub_d));

      // Shell Jacobian action: jac_action = J * d
      PetscCallAbort(PETSC_COMM_WORLD, MatMult(J, d, jac_action));

      // Build both perturbed points before any SNESApplyNPC call.
      // SNESApplyNPC runs sub-solves that write M(x) back into the libmesh solution
      // vecs, which are aliased by the test's x.  If x_minus were built after the
      // first SNESApplyNPC, it would be based on M(x_plus) rather than the original x.
      PetscCallAbort(PETSC_COMM_WORLD, VecCopy(x, x_plus));
      PetscCallAbort(PETSC_COMM_WORLD, VecAXPY(x_plus, eps, d));
      PetscCallAbort(PETSC_COMM_WORLD, VecCopy(x, x_minus));
      PetscCallAbort(PETSC_COMM_WORLD, VecAXPY(x_minus, -eps, d));

      // SNESApplyNPC returns P(x) = x - M(x), so the FD below approximates
      // (I - dM/dx)*d, which matches what applyBA computes.
      PetscCallAbort(PETSC_COMM_WORLD, SNESApplyNPC(outer_snes, x_plus, nullptr, r_plus));
      PetscCallAbort(PETSC_COMM_WORLD, SNESApplyNPC(outer_snes, x_minus, nullptr, r_minus));

      PetscCallAbort(PETSC_COMM_WORLD, VecCopy(r_plus, fd));
      PetscCallAbort(PETSC_COMM_WORLD, VecAXPY(fd, -1.0, r_minus));
      PetscCallAbort(PETSC_COMM_WORLD, VecScale(fd, 1.0 / (2.0 * eps)));

      // Compare: error = jac_action - fd
      PetscCallAbort(PETSC_COMM_WORLD, VecAXPY(jac_action, -1.0, fd));
      PetscReal error_norm, fd_norm;
      PetscCallAbort(PETSC_COMM_WORLD, VecNorm(jac_action, NORM_2, &error_norm));
      PetscCallAbort(PETSC_COMM_WORLD, VecNorm(fd, NORM_2, &fd_norm));
      EXPECT_LT(error_norm / (fd_norm + 1e-15), tol)
          << "block=" << block << " dof=" << dof << " sweep_type=" << sweep_type;
    }
  }

  PetscCallAbort(PETSC_COMM_WORLD, VecDestroy(&d));
  PetscCallAbort(PETSC_COMM_WORLD, VecDestroy(&x_plus));
  PetscCallAbort(PETSC_COMM_WORLD, VecDestroy(&x_minus));
  PetscCallAbort(PETSC_COMM_WORLD, VecDestroy(&r_plus));
  PetscCallAbort(PETSC_COMM_WORLD, VecDestroy(&r_minus));
  PetscCallAbort(PETSC_COMM_WORLD, VecDestroy(&fd));
  PetscCallAbort(PETSC_COMM_WORLD, VecDestroy(&jac_action));
  PetscCallAbort(PETSC_COMM_WORLD, VecDestroy(&x));
}

INSTANTIATE_TEST_SUITE_P(SweepTypes,
                         NMSMExecutorJacobianTest,
                         testing::Values("multiplicative", "symmetric_multiplicative"));
