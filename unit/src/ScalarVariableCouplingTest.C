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
#include "MooseVariableScalar.h"
#include "NonlinearSystem.h"

#include <string>

using MooseAppTestUtils::Args;
using MooseAppTestUtils::getJacobianNNZ;

// A kernel here (ScalarLagrangeMultiplier) couples a field variable "u" to a scalar variable
// "lambda". The default coupling matrix is diagonal-only, so the u/lambda block is not part of
// what the user asked the preconditioner to assemble; requesting full coupling instead asks for
// every block. The assembled Jacobian's preallocated sparsity should reflect exactly which of the
// two was actually requested, rather than always reflecting the union of what any kernel is
// capable of coupling.
TEST(ScalarVariableCoupling, DefaultDiagonalCouplingOmitsScalarOffDiagonal)
{
  const std::string input = "files/ScalarVariableCouplingTest/coupling.i";

  PetscInt nnz_diagonal;
  {
    Args args({"-i", input});
    const auto app = Moose::createMooseApp("MooseUnitApp", args.argc(), args.argv());
    app->run();
    auto & fe_problem = app->getExecutioner()->feProblem();
    nnz_diagonal = getJacobianNNZ(fe_problem);

    // Default coupling is diagonal-only: u/lambda is not a requested block.
    auto & sys = fe_problem.getNonlinearSystemBase(0);
    EXPECT_FALSE(fe_problem.areCoupled(
        sys.getVariable(0, "u").number(), sys.getScalarVariable(0, "lambda").number(), 0));
  }

  PetscInt nnz_full;
  {
    Args args({"-i", input, "Preconditioning/smp/full=true"});
    const auto app = Moose::createMooseApp("MooseUnitApp", args.argc(), args.argv());
    app->run();
    auto & fe_problem = app->getExecutioner()->feProblem();
    nnz_full = getJacobianNNZ(fe_problem);

    // full=true requests every block, u/lambda included.
    auto & sys = fe_problem.getNonlinearSystemBase(0);
    EXPECT_TRUE(fe_problem.areCoupled(
        sys.getVariable(0, "u").number(), sys.getScalarVariable(0, "lambda").number(), 0));
  }

  // The assembled matrix itself reflects the coupling that was actually requested: full coupling
  // preallocates strictly more nonzeros than the diagonal default.
  EXPECT_LT(nnz_diagonal, nnz_full);
}
