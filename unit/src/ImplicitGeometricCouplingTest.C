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
#include "MooseAppTestUtils.h"
#include "MooseMain.h"

#include <string>

using MooseAppTestUtils::Args;
using MooseAppTestUtils::getJacobianNNZ;

// With standard (CSR) matrix assembly and a NodeFaceConstraint, MOOSE pre-populates the Jacobian
// sparsity with zeros for all potential secondary-primary element DOF pairs via
// addImplicitGeometricCouplingEntries. With hash-table matrix assembly this step is skipped
// because the format handles new entries dynamically. The result should be measurably fewer
// stored nonzeros in the hash-table case.
TEST(ImplicitGeometricCoupling, HashTableAssemblyDoesNotInflateSparsity)
{
  const std::string input = "files/ImplicitGeometricCouplingTest/sparsity.i";

  PetscInt nnz_standard;
  {
    Args args({"-i", input});
    const auto app = Moose::createMooseApp("MooseUnitApp", args.argc(), args.argv());
    app->run();
    nnz_standard = getJacobianNNZ(app->getExecutioner()->feProblem());
  }

  PetscInt nnz_hash_table;
  {
    Args args({"-i", input, "Problem/use_hash_table_matrix_assembly=true"});
    const auto app = Moose::createMooseApp("MooseUnitApp", args.argc(), args.argv());
    app->run();
    nnz_hash_table = getJacobianNNZ(app->getExecutioner()->feProblem());
  }

  EXPECT_LT(nnz_hash_table, nnz_standard);
}
