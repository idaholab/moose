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
#include "MooseMain.h"
#include "NonlinearSystem.h"

#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/petsc_matrix.h"

#include <string>
#include <vector>

namespace
{

/// Wraps a vector of string arguments as argc/argv with a fake executable name prepended.
struct Args
{
  Args(const std::vector<std::string> & args) : _args(args)
  {
    _args.insert(_args.begin(), "unused");
    for (auto & arg : _args)
      _argv.push_back(arg.data());
    _argv.push_back(nullptr);
  }
  int argc() const { return static_cast<int>(_argv.size()) - 1; }
  char ** argv() { return _argv.data(); }
  std::vector<std::string> _args;
  std::vector<char *> _argv;
};

// Returns the number of stored nonzero entries in the assembled Jacobian.
// Accesses the matrix via the libMesh NonlinearImplicitSystem directly, since
// MOOSE's tag-based getMatrix() is not valid after run() returns.
PetscInt
getJacobianNNZ(FEProblemBase & fe_problem)
{
  auto * mat =
      dynamic_cast<libMesh::PetscMatrix<Number> *>(fe_problem.getNonlinearSystem(0).sys().matrix);
  mooseAssert(mat, "Expected a PetscMatrix for the system Jacobian");
  MatInfo info;
  LibmeshPetscCallA(fe_problem.comm().get(), MatGetInfo(mat->mat(), MAT_LOCAL, &info));
  return static_cast<PetscInt>(info.nz_used);
}

} // namespace

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
