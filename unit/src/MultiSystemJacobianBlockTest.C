//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest_include.h"

#include "Executioner.h"
#include "FEProblemBase.h"
#include "MooseAppTestUtils.h"
#include "MooseMain.h"
#include "NonlinearSystemBase.h"

#include "libmesh/numeric_vector.h"
#include "libmesh/petsc_matrix.h"

using MooseAppTestUtils::Args;

namespace
{
/// Sets the solution of \p sys to a smooth nonconstant state
void
setState(NonlinearSystemBase & sys, const Real offset)
{
  auto & solution = *sys.system().solution;
  for (const auto i : make_range(solution.first_local_index(), solution.last_local_index()))
    solution.set(i, offset + 0.1 * i);
  solution.close();
  sys.system().update();
}

/**
 * Checks the Jacobian block of the residual of system \p i with respect to the solution of
 * system \p j against central finite differences.
 */
void
checkBlock(FEProblemBase & problem, const unsigned int i, const unsigned int j)
{
  auto & sys_i = problem.getNonlinearSystemBase(i);
  auto & sys_j = problem.getNonlinearSystemBase(j);
  auto & lm_i = sys_i.system();
  auto & lm_j = sys_j.system();

  libMesh::PetscMatrix<Number> jac(problem.comm());
  // Hash table assembly lets the block take any sparsity pattern
  jac.init_without_preallocation(
      lm_i.n_dofs(), lm_j.n_dofs(), lm_i.n_local_dofs(), lm_j.n_local_dofs(), 1);
  jac.finish_initialization();
  problem.computeJacobian(*lm_i.current_local_solution, jac, i, j);
  jac.close();

  auto r_plus = lm_i.solution->zero_clone();
  auto r_minus = lm_i.solution->zero_clone();
  auto direction = lm_j.solution->zero_clone();
  auto jac_column = lm_i.solution->zero_clone();
  auto & solution_j = *lm_j.solution;

  // The residual is a low-degree polynomial of the solution, so the central difference truncation
  // error is far below the tolerance
  const Real eps = 1e-6;
  for (const auto dof : make_range(lm_j.n_dofs()))
  {
    direction->zero();
    if (dof >= direction->first_local_index() && dof < direction->last_local_index())
      direction->set(dof, 1);
    direction->close();
    jac.vector_mult(*jac_column, *direction);

    solution_j.add(eps, *direction);
    solution_j.close();
    lm_j.update();
    problem.computeResidual(*lm_i.current_local_solution, *r_plus, i);

    solution_j.add(-2 * eps, *direction);
    solution_j.close();
    lm_j.update();
    problem.computeResidual(*lm_i.current_local_solution, *r_minus, i);

    solution_j.add(eps, *direction);
    solution_j.close();
    lm_j.update();

    *r_plus -= *r_minus;
    r_plus->scale(1 / (2 * eps));
    *jac_column -= *r_plus;
    EXPECT_LT(jac_column->linfty_norm(), 1e-6 * (1 + r_plus->linfty_norm()))
        << "block (" << i << ", " << j << "), column " << dof;
  }
}
}

TEST(MultiSystemJacobianBlockTest, BlocksMatchFiniteDifferences)
{
  Args args({"-i", "files/MultiSystemJacobianBlockTest/two_systems.i"});
  const auto app = Moose::createMooseApp("MooseUnitApp", args.argc(), args.argv());
  app->run();
  auto & problem = app->getExecutioner()->feProblem();

  setState(problem.getNonlinearSystemBase(0), 0.5);
  setState(problem.getNonlinearSystemBase(1), -0.3);

  for (const auto i : make_range(2u))
    for (const auto j : make_range(2u))
      checkBlock(problem, i, j);
}
