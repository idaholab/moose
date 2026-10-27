//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseAppTestUtils.h"

#include "FEProblemBase.h"
#include "NonlinearSystem.h"

#include "libmesh/nonlinear_implicit_system.h"

namespace MooseAppTestUtils
{

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

} // namespace MooseAppTestUtils
